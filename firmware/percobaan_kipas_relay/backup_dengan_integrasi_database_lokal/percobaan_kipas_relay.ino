#include <WiFi.h> 
#include <HTTPClient.h>
#include <ArduinoJson.h> // Library baru untuk baca data web!

#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h> 

// =============================================================
// 1. KONFIGURASI WIFI & SERVER FLASK
// =============================================================
#define WIFI_SSID "Baskoro2"
#define WIFI_PASSWORD "12345677"

// Ganti 192.168.x.x dengan IP IPv4 laptop abang!
#define SERVER_URL_POST "http://10.122.187.235:5000/kirim_data"
#define SERVER_URL_GET  "http://10.122.187.235:5000/ambil_data" // URL baru untuk baca settingan

// =============================================================
// 2. KONFIGURASI PIN ESP32
// =============================================================
#define DHTPIN 15     
#define RELAYPIN 5    
#define SERVOPIN 13   
#define TRIGPIN 25    
#define ECHOPIN 26    
#define MQ135PIN 34   

#define DHTTYPE DHT22 

// === BATAS PEMICU (Akan otomatis di-update dari Web) ===
float batasSuhu = 30.0; 
int batasJarak = 20;    

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo myservo; 

unsigned long timerUltrasonic = 0;
unsigned long timerDHTDanLCD = 0;
unsigned long timerGantiLayar = 0;
unsigned long timerKirimData = 0; 
unsigned long timerAmbilPengaturan = 0; // Timer baru

float t = 0;
float h = 0;
int jarak = 0;
int nilaiMQ = 0;

byte halamanLCD = 0;
bool statusServoDekat = false;
String statusKipas = "OFF";

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAYPIN, INPUT); 
  digitalWrite(RELAYPIN, LOW); 
  
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
  pinMode(MQ135PIN, INPUT);
  
  myservo.attach(SERVOPIN);
  myservo.write(0); 
  
  dht.begin();
  Wire.begin(); 
  lcd.init();
  lcd.backlight(); 
  
  Serial.println("\n\n--- MULAI SISTEM ---");
  lcd.setCursor(0, 0); lcd.print("Menghubungkan...");
  lcd.setCursor(0, 1); lcd.print(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n[BERHASIL] WiFi Terhubung!");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("WiFi Terhubung!");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // -------------------------------------------------------------------
  // [BARU] JALUR 0: AMBIL PENGATURAN DARI WEB (Jeda 5000ms)
  // -------------------------------------------------------------------
  if (WiFi.status() == WL_CONNECTED && (currentMillis - timerAmbilPengaturan >= 5000)) {
    timerAmbilPengaturan = currentMillis;
    
    HTTPClient http;
    http.begin(SERVER_URL_GET);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      
      // Membedah data JSON dari Flask
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Timpa nilai batas pemicu dengan angka dari Database Web
        batasSuhu = doc["pengaturan"]["batas_suhu"];
        batasJarak = doc["pengaturan"]["batas_jarak"];
        
        Serial.print("[INFO] Pemicu Diupdate -> Suhu: ");
        Serial.print(batasSuhu);
        Serial.print(" | Jarak: ");
        Serial.println(batasJarak);
      }
    }
    http.end();
  }

  // -------------------------------------------------------------------
  // JALUR 1: ULTRASONIC & SERVO (Jeda Baca 250ms)
  // -------------------------------------------------------------------
  if (currentMillis - timerUltrasonic >= 250) {
    timerUltrasonic = currentMillis; 

    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);
    
    long durasi = pulseIn(ECHOPIN, HIGH); 
    jarak = (durasi == 0) ? 999 : (durasi * 0.034 / 2);

    // Sekarang sistem memakai variabel batasJarak yang terbaru dari Web!
    if (jarak < batasJarak) {
      if (!statusServoDekat) { 
        myservo.write(90); 
        statusServoDekat = true; 
      }
    } else if (jarak > (batasJarak + 3)) { 
      if (statusServoDekat) { 
        myservo.write(0);  
        statusServoDekat = false; 
      }
    }
  }

  // -------------------------------------------------------------------
  // JALUR 2: BACA DHT22, MQ-135, & KIPAS (Jeda 2000ms)
  // -------------------------------------------------------------------
  if (currentMillis - timerDHTDanLCD >= 2000) {
    timerDHTDanLCD = currentMillis; 

    t = dht.readTemperature();
    h = dht.readHumidity();
    nilaiMQ = analogRead(MQ135PIN);

    if (!isnan(t)) {
      // Sekarang sistem memakai variabel batasSuhu yang terbaru dari Web!
      if (t > batasSuhu) {
        pinMode(RELAYPIN, OUTPUT);
        digitalWrite(RELAYPIN, LOW); 
        statusKipas = "ON";
      } else {
        pinMode(RELAYPIN, INPUT);    
        statusKipas = "OFF";
      }
    }
  }

  // -------------------------------------------------------------------
  // JALUR 3: KIRIM DATA KE FLASK/MYSQL (Jeda 3000ms)
  // -------------------------------------------------------------------
  if (WiFi.status() == WL_CONNECTED && (currentMillis - timerKirimData >= 3000)) {
    timerKirimData = currentMillis;
    
    HTTPClient http;
    http.begin(SERVER_URL_POST);
    http.addHeader("Content-Type", "application/json");

    String json_data = "{\"suhu\":" + String(t) + 
                       ", \"gas\":" + String(nilaiMQ) + 
                       ", \"jarak\":" + String(jarak == 999 ? 0 : jarak) + 
                       ", \"status_kipas\":\"" + statusKipas + "\"" + 
                       ", \"posisi_servo\":" + String(statusServoDekat ? 90 : 0) + "}";

    int httpResponseCode = http.POST(json_data);

    if (httpResponseCode > 0) {
      Serial.println("[SUKSES] Data Sensor Terkirim!");
    } else {
      Serial.println("[GAGAL] Error HTTP POST");
    }
    http.end(); 
  }

  // -------------------------------------------------------------------
  // JALUR 4: PERGANTIAN HALAMAN LCD (Jeda 5000ms)
  // -------------------------------------------------------------------
  if (currentMillis - timerGantiLayar >= 5000) {
    timerGantiLayar = currentMillis;
    halamanLCD++;           
    if (halamanLCD > 2) halamanLCD = 0;
    lcd.clear();            
  }

  // -------------------------------------------------------------------
  // RENDER TAMPILAN LCD
  // -------------------------------------------------------------------
  if (halamanLCD == 0) {
    lcd.setCursor(0, 0); lcd.print("-- SUHU & KIPAS --");
    lcd.setCursor(0, 1);
    if (isnan(t)) {
      lcd.print("Suhu: Error     ");
    } else {
      lcd.print("T: "); lcd.print(t, 1); lcd.print((char)223); lcd.print("C  K:");
      lcd.print(statusKipas);
    }
  } 
  else if (halamanLCD == 1) {
    lcd.setCursor(0, 0); lcd.print("- JARAK & SERVO -");
    lcd.setCursor(0, 1);
    lcd.print("J:"); 
    if (jarak == 999) lcd.print("-- "); else lcd.print(jarak);
    lcd.print("cm   S:");
    lcd.print(statusServoDekat ? "90" : "0 ");
  } 
  else if (halamanLCD == 2) {
    lcd.setCursor(0, 0); lcd.print("  UDARA & GAS  ");
    lcd.setCursor(0, 1);
    if (isnan(h)) {
      lcd.print("H: Err ");
    } else {
      lcd.print("H:"); lcd.print((int)h); lcd.print("% ");
    }
    lcd.print(" G:"); lcd.print(nilaiMQ);
  }
}