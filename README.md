# SIGMA IoT — Sistem Monitoring & Kendali Kandang Otomatis

Dashboard web untuk memantau dan mengendalikan alat berbasis ESP32 (suhu, gas, jarak, kipas, servo) secara real-time, dengan penyimpanan riwayat data di MySQL.

## Fitur Utama (8 fitur)

| # | Fitur | Fungsi CRUD | Deskripsi |
|---|-------|-------------|-----------|
| 1 | Perekaman data otomatis | CREATE | ESP32 mengirim data sensor ke server tiap 3 detik, tersimpan sebagai baris baru di `sensor_logs` |
| 2 | Dashboard real-time | READ | Kartu angka digital suhu, gas, jarak, status kipas & servo, auto-refresh tiap 3 detik |
| 3 | Grafik fluktuasi sensor | READ | Grafik garis interaktif (Chart.js) untuk tren suhu & gas dari 50 data terakhir |
| 4 | Pengaturan batas pemicu dinamis | UPDATE | Form untuk mengubah `batas_suhu`, `batas_jarak`, `batas_gas`; ESP32 polling nilai baru tiap 5 detik |
| 5 | Mode kendali Auto/Manual | UPDATE | Switch mode; saat manual, tombol ON/OFF kipas & slider servo mengabaikan pembacaan sensor |
| 6 | Sistem peringatan dini | READ/UI | Banner merah berkedip otomatis saat suhu/gas melewati ambang kritis |
| 7 | Ekspor laporan CSV | READ/EXPORT | Unduh seluruh riwayat data sensor dalam format `.csv` |
| 8 | Manajemen pembersihan data | DELETE | Hapus data log yang lebih lama dari tanggal tertentu |

## Struktur Folder

```
iot-dashboard/
├── firmware/
│   └── percobaan_kipas_relay/
│       └── percobaan_kipas_relay.ino   # Firmware ESP32 (HTTP API, bukan Firebase)
├── backend/
│   ├── app.py                          # Flask REST API
│   ├── requirements.txt
│   └── templates/
│       ├── index.html                  # Landing page
│       └── dashboard.html               # Dashboard monitoring & kendali
├── database/
│   └── schema.sql                       # Struktur tabel MySQL
└── README.md
```

## Cara Menjalankan (Lokal)

```bash
cd backend
python -m venv venv
venv\Scripts\activate          # Windows
pip install -r requirements.txt
```

1. Jalankan MySQL lokal (XAMPP/Laragon), import `database/schema.sql` lewat phpMyAdmin.
2. Sesuaikan kredensial database di `app.py` (`DB_CONFIG`).
3. Jalankan server:
   ```bash
   python app.py
   ```
4. Buka `http://localhost:5000` di browser.

## Firmware ESP32

1. Buka `firmware/percobaan_kipas_relay/percobaan_kipas_relay.ino` di Arduino IDE.
2. Install library: `DHT sensor library`, `LiquidCrystal_I2C`, `ESP32Servo`, `ArduinoJson`.
3. Sesuaikan `WIFI_SSID`, `WIFI_PASSWORD`, dan `API_BASE` (IP lokal komputer atau domain hosting).
4. Upload ke ESP32.

## Deploy Online

Backend + database dapat di-deploy ke **PythonAnywhere** (gratis, MySQL sudah termasuk) atau kombinasi Render/Railway + filess.io.

## Kontributor

- **[Nama 1]** — Backend Flask, database schema
- **[Nama 2]** — Frontend dashboard, styling
- **[Nama 3]** — Firmware ESP32, integrasi hardware

*(Sesuaikan dengan pembagian kerja & commit masing-masing anggota)*
