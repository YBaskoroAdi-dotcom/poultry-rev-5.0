-- Jalankan di phpMyAdmin (lokal) atau di panel database hosting (PythonAnywhere/filess.io)
CREATE DATABASE IF NOT EXISTS iot_database;
USE iot_database;

CREATE TABLE IF NOT EXISTS sensor_logs (
  id INT AUTO_INCREMENT PRIMARY KEY,
  suhu FLOAT,
  kelembapan FLOAT,
  gas INT,
  jarak INT,
  status_kipas VARCHAR(5),
  status_servo INT,
  waktu DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS settings (
  id INT PRIMARY KEY DEFAULT 1,
  batas_suhu FLOAT DEFAULT 30,
  batas_jarak INT DEFAULT 20,
  batas_gas INT DEFAULT 500,
  mode VARCHAR(10) DEFAULT 'auto',
  kipas_manual VARCHAR(5) DEFAULT 'OFF',
  servo_manual INT DEFAULT 0
);

INSERT INTO settings (id) VALUES (1)
  ON DUPLICATE KEY UPDATE id = id;
