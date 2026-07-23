#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// =============================================================
// 1. KONFIGURASI WIFI & API BACKEND
// =============================================================
#define WIFI_SSID "Baskoro2"
#define WIFI_PASSWORD "12345677"

// Ganti dengan IP lokal komputer (development) atau domain hosting (production)
#define API_BASE "http://192.168.1.100:5000/api"

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

// === BATAS PEMICU (nilai default, akan di-overwrite dari server) ===
float batasSuhu = 30.0;
int batasJarak = 20;
int batasGas = 500;

// === MODE & KENDALI MANUAL (diambil dari server) ===
String mode = "auto";        // "auto" atau "manual"
String kipasManual = "OFF";
int servoManual = 0;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

unsigned long timerUltrasonic = 0;
unsigned long timerDHTDanLCD = 0;
unsigned long timerGantiLayar = 0;
unsigned long timerKirimData = 0;
unsigned long timerAmbilSettings = 0;

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
  lcd.setCursor(0, 1); lcd.print("Sistem Ready!");
  delay(1500);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // -------------------------------------------------------------------
  // JALUR 1: ULTRASONIC & SERVO (Jeda 250ms) — hanya aktif saat mode auto
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

    if (mode == "auto") {
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
  }

  // -------------------------------------------------------------------
  // JALUR 2: BACA DHT22, MQ-135, & LOGIC KIPAS (Jeda 2000ms)
  // -------------------------------------------------------------------
  if (currentMillis - timerDHTDanLCD >= 2000) {
    timerDHTDanLCD = currentMillis;

    t = dht.readTemperature();
    h = dht.readHumidity();
    nilaiMQ = analogRead(MQ135PIN);

    if (mode == "manual") {
      // MODE MANUAL: abaikan sensor, ikutin perintah dari dashboard
      statusKipas = kipasManual;
      if (statusKipas == "ON") {
        pinMode(RELAYPIN, OUTPUT);
        digitalWrite(RELAYPIN, LOW);
      } else {
        pinMode(RELAYPIN, INPUT);
      }
      myservo.write(servoManual);
    } else if (!isnan(t)) {
      // MODE AUTO: logic asli berdasarkan sensor
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
  // JALUR 3: KIRIM DATA KE BACKEND (Jeda 3000ms) -> fitur CREATE
  // -------------------------------------------------------------------
  if (WiFi.status() == WL_CONNECTED && (currentMillis - timerKirimData >= 3000)) {
    timerKirimData = currentMillis;
    kirimDataSensor();
  }

  // -------------------------------------------------------------------
  // JALUR 4: AMBIL SETTINGS DARI SERVER (Jeda 5000ms) -> fitur UPDATE
  // -------------------------------------------------------------------
  if (WiFi.status() == WL_CONNECTED && (currentMillis - timerAmbilSettings >= 5000)) {
    timerAmbilSettings = currentMillis;
    ambilSettings();
  }

  // -------------------------------------------------------------------
  // JALUR 5: PERGANTIAN HALAMAN LCD (Jeda 5000ms)
  // -------------------------------------------------------------------
  if (currentMillis - timerGantiLayar >= 5000) {
    timerGantiLayar = currentMillis;
    halamanLCD++;
    if (halamanLCD > 2) halamanLCD = 0;
    lcd.clear();
  }

  renderLCD();
}

// =============================================================
// KIRIM DATA SENSOR KE /api/log (POST)
// =============================================================
void kirimDataSensor() {
  HTTPClient http;
  http.begin(String(API_BASE) + "/log");
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc;
  doc["suhu"] = isnan(t) ? 0 : t;
  doc["kelembapan"] = isnan(h) ? 0 : h;
  doc["gas"] = nilaiMQ;
  doc["jarak"] = (jarak == 999) ? 0 : jarak;
  doc["status_kipas"] = statusKipas;
  doc["status_servo"] = statusServoDekat ? 90 : 0;

  String payload;
  serializeJson(doc, payload);

  int httpCode = http.POST(payload);
  if (httpCode <= 0) {
    Serial.print("[GAGAL KIRIM] "); Serial.println(http.errorToString(httpCode));
  } else {
    Serial.print("[KIRIM OK] Status: "); Serial.println(httpCode);
  }
  http.end();
}

// =============================================================
// AMBIL SETTINGS TERBARU DARI /api/settings (GET)
// =============================================================
void ambilSettings() {
  HTTPClient http;
  http.begin(String(API_BASE) + "/settings");
  int httpCode = http.GET();

  if (httpCode == 200) {
    String resp = http.getString();
    StaticJsonDocument<384> doc;
    DeserializationError err = deserializeJson(doc, resp);

    if (!err) {
      batasSuhu = doc["batas_suhu"] | batasSuhu;
      batasJarak = doc["batas_jarak"] | batasJarak;
      batasGas = doc["batas_gas"] | batasGas;
      mode = doc["mode"].as<String>();
      kipasManual = doc["kipas_manual"].as<String>();
      servoManual = doc["servo_manual"] | servoManual;
    }
  } else {
    Serial.print("[GAGAL AMBIL SETTINGS] Kode: "); Serial.println(httpCode);
  }
  http.end();
}

// =============================================================
// RENDER TAMPILAN LCD
// =============================================================
void renderLCD() {
  if (halamanLCD == 0) {
    lcd.setCursor(0, 0); lcd.print("-- SUHU & KIPAS --");
    lcd.setCursor(0, 1);
    if (isnan(t)) {
      lcd.print("Suhu: Error     ");
    } else {
      lcd.print("T:"); lcd.print(t, 1); lcd.print((char)223); lcd.print("C K:");
      lcd.print(statusKipas);
    }
  } else if (halamanLCD == 1) {
    lcd.setCursor(0, 0); lcd.print("- JARAK & SERVO -");
    lcd.setCursor(0, 1);
    lcd.print("J:");
    if (jarak == 999) lcd.print("-- "); else lcd.print(jarak);
    lcd.print("cm M:"); lcd.print(mode == "auto" ? "A" : "M");
  } else if (halamanLCD == 2) {
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
