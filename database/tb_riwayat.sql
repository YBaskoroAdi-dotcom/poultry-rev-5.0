-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Jul 25, 2026 at 07:21 AM
-- Server version: 10.4.32-MariaDB
-- PHP Version: 8.2.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `tb_riwayat`
--

-- --------------------------------------------------------

--
-- Table structure for table `sensor_logs`
--

CREATE TABLE `sensor_logs` (
  `id` int(11) NOT NULL,
  `suhu` float DEFAULT NULL,
  `kelembapan` float DEFAULT NULL,
  `gas` int(11) DEFAULT NULL,
  `jarak` int(11) DEFAULT NULL,
  `status_kipas` varchar(10) DEFAULT NULL,
  `status_servo` int(11) DEFAULT NULL,
  `waktu` timestamp NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `sensor_logs`
--

INSERT INTO `sensor_logs` (`id`, `suhu`, `kelembapan`, `gas`, `jarak`, `status_kipas`, `status_servo`, `waktu`) VALUES
(553, 28.2, 73.2, 163, 120, 'OFF', 0, '2026-07-25 03:24:34'),
(554, 28.2, 73.3, 156, 121, 'OFF', 0, '2026-07-25 03:24:37'),
(555, 28.2, 73.3, 163, 107, 'OFF', 0, '2026-07-25 03:24:40'),
(556, 28.2, 73.2, 163, 107, 'OFF', 0, '2026-07-25 03:24:43'),
(557, 28.2, 73.2, 163, 107, 'OFF', 0, '2026-07-25 03:24:46'),
(558, 28.1, 73.3, 164, 121, 'OFF', 0, '2026-07-25 03:24:49'),
(559, 28.2, 73.2, 163, 118, 'OFF', 0, '2026-07-25 03:24:52'),
(560, 28.2, 73.2, 163, 109, 'OFF', 0, '2026-07-25 03:24:55'),
(561, 28.2, 73.2, 165, 107, 'OFF', 0, '2026-07-25 03:24:58'),
(562, 28.2, 73.2, 160, 120, 'OFF', 0, '2026-07-25 03:25:01'),
(563, 28.2, 73.3, 161, 121, 'OFF', 0, '2026-07-25 03:25:04'),
(564, 28.1, 73.2, 161, 120, 'OFF', 0, '2026-07-25 03:25:07'),
(565, 28.1, 73.3, 161, 107, 'OFF', 0, '2026-07-25 03:25:10'),
(566, 28.2, 73.3, 162, 109, 'OFF', 0, '2026-07-25 03:25:13'),
(567, 28.2, 73.3, 160, 107, 'OFF', 0, '2026-07-25 03:25:16'),
(568, 28.2, 73.3, 160, 115, 'OFF', 0, '2026-07-25 03:25:19'),
(569, 28.1, 73.2, 161, 119, 'OFF', 0, '2026-07-25 03:25:22'),
(570, 28.2, 73.3, 159, 116, 'OFF', 0, '2026-07-25 03:25:25'),
(571, 28.1, 73.3, 160, 3, 'OFF', 90, '2026-07-25 03:25:28'),
(572, 28.2, 73.3, 158, 120, 'OFF', 0, '2026-07-25 03:25:31');

-- --------------------------------------------------------

--
-- Table structure for table `settings`
--

CREATE TABLE `settings` (
  `id` int(11) NOT NULL,
  `batas_suhu` float DEFAULT 30,
  `batas_jarak` int(11) DEFAULT 20,
  `batas_gas` int(11) DEFAULT 500,
  `mode` varchar(10) DEFAULT 'auto',
  `kipas_manual` varchar(10) DEFAULT 'OFF',
  `servo_manual` int(11) DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `settings`
--

INSERT INTO `settings` (`id`, `batas_suhu`, `batas_jarak`, `batas_gas`, `mode`, `kipas_manual`, `servo_manual`) VALUES
(1, 30, 20, 500, 'auto', 'OFF', 90);

-- --------------------------------------------------------

--
-- Table structure for table `tb_pengaturan`
--

CREATE TABLE `tb_pengaturan` (
  `id` int(11) NOT NULL,
  `batas_suhu` float DEFAULT NULL,
  `mode_alat` varchar(20) DEFAULT NULL,
  `batas_jarak` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `tb_pengaturan`
--

INSERT INTO `tb_pengaturan` (`id`, `batas_suhu`, `mode_alat`, `batas_jarak`) VALUES
(1, 30, 'Otomatis', 20);

-- --------------------------------------------------------

--
-- Table structure for table `tb_riwayat`
--

CREATE TABLE `tb_riwayat` (
  `id` int(11) NOT NULL,
  `waktu` timestamp NOT NULL DEFAULT current_timestamp(),
  `suhu` float DEFAULT NULL,
  `gas` int(11) DEFAULT NULL,
  `jarak` int(11) DEFAULT NULL,
  `status_kipas` varchar(10) DEFAULT NULL,
  `posisi_servo` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `tb_riwayat`
--

INSERT INTO `tb_riwayat` (`id`, `waktu`, `suhu`, `gas`, `jarak`, `status_kipas`, `posisi_servo`) VALUES
(530, '2026-07-23 08:22:32', 30, 68, 148, 'OFF', 0),
(531, '2026-07-23 08:22:35', 30, 70, 148, 'OFF', 0),
(532, '2026-07-23 08:22:38', 30, 70, 147, 'OFF', 0),
(533, '2026-07-23 08:22:41', 30, 68, 146, 'OFF', 0),
(534, '2026-07-23 08:22:44', 30, 69, 146, 'OFF', 0),
(535, '2026-07-23 08:22:47', 30, 70, 148, 'OFF', 0),
(536, '2026-07-23 08:22:50', 30, 69, 147, 'OFF', 0),
(537, '2026-07-23 08:22:53', 30, 71, 148, 'OFF', 0),
(538, '2026-07-23 08:22:56', 30, 71, 146, 'OFF', 0),
(539, '2026-07-23 08:22:59', 30, 70, 146, 'OFF', 0),
(540, '2026-07-23 08:23:02', 30, 71, 147, 'OFF', 0),
(541, '2026-07-23 08:23:05', 30, 76, 147, 'OFF', 0),
(542, '2026-07-23 08:23:08', 30, 69, 146, 'OFF', 0),
(543, '2026-07-23 08:23:11', 30, 74, 146, 'OFF', 0),
(544, '2026-07-23 08:23:14', 30, 71, 147, 'OFF', 0),
(545, '2026-07-23 08:23:17', 30, 65, 148, 'OFF', 0),
(546, '2026-07-23 08:23:20', 30, 71, 146, 'OFF', 0),
(547, '2026-07-23 08:23:23', 30.1, 68, 147, 'ON', 0),
(548, '2026-07-23 08:23:26', 30.1, 69, 149, 'ON', 0),
(549, '2026-07-23 08:23:29', 30, 67, 149, 'OFF', 0);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `sensor_logs`
--
ALTER TABLE `sensor_logs`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `settings`
--
ALTER TABLE `settings`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tb_pengaturan`
--
ALTER TABLE `tb_pengaturan`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tb_riwayat`
--
ALTER TABLE `tb_riwayat`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `sensor_logs`
--
ALTER TABLE `sensor_logs`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=573;

--
-- AUTO_INCREMENT for table `settings`
--
ALTER TABLE `settings`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `tb_riwayat`
--
ALTER TABLE `tb_riwayat`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=550;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
