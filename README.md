# ESP32GravityTDS

[![Platform](https://img.shields.io/badge/Platform-ESP32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-green)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-LGPLv3-lightgrey)](https://opensource.org/licenses/LGPL-3.0)

Library untuk **DFRobot Gravity: Analog TDS Sensor/Meter (SEN0244)** yang sudah dimodifikasi khusus untuk **ESP32**.

## 📌 Fitur

- ✅ Kompatibel penuh dengan ESP32 (ADC 12-bit)
- ✅ Mendukung kalibrasi melalui Serial Monitor
- ✅ Kompensasi suhu otomatis
- ✅ Mode debugging untuk troubleshooting
- ✅ Dapat menyimpan nilai kalibrasi ke EEPROM
- ✅ Mudah digunakan dengan method yang sederhana

## 🔧 Perubahan dari Library Original

| Fitur | Original (Arduino) | ESP32GravityTDS |
|-------|-------------------|-----------------|
| ADC Resolution | 10-bit (0-1023) | 12-bit (0-4095) |
| Reference Voltage | 5.0V | 3.3V |
| ADC Konfigurasi | Tidak perlu | `analogReadResolution()` + `analogSetAttenuation()` |
| Debug Mode | Tidak ada | ✅ Ada |
| Platform Detection | Tidak ada | ✅ Auto-detect |

## 📥 Instalasi

### Arduino IDE
1. Download zip dari repository ini
2. Buka Arduino IDE → **Sketch** → **Include Library** → **Add .ZIP Library**
3. Pilih file zip yang sudah di download

Atau manual:
git clone https://github.com/AripJam18/ESP32GravityTDS.git

🔌 Koneksi Hardware
Sensor TDS	ESP32
Merah (VCC)	3.3V
Hitam (GND)	GND
Kuning (Data)	GPIO32 (atau ADC1 manapun)
⚠️ PENTING: Gunakan pin ADC1 untuk hasil terbaik (GPIO32, GPIO33, GPIO34, GPIO35, GPIO36, GPIO39). Hindari pin ADC2 jika menggunakan WiFi.

🚀 Contoh Penggunaan
Contoh Dasar :

#include <EEPROM.h>
#include "ESP32GravityTDS.h"

ESP32GravityTDS tdsSensor;

void setup() {
    Serial.begin(115200);
    EEPROM.begin(32);  // ESP32 butuh EEPROM.begin()
    
    tdsSensor.setPin(32);        // GPIO32
    tdsSensor.setAref(3.3);      // ESP32 = 3.3V
    tdsSensor.setAdcRange(4096); // 12-bit ADC
    tdsSensor.setTemperature(25); // Suhu ruang
    
    tdsSensor.begin();
    
    Serial.println("TDS Sensor Ready!");
}

void loop() {
    tdsSensor.update();
    float tdsValue = tdsSensor.getTdsValue();
    
    Serial.print("TDS: ");
    Serial.print(tdsValue, 0);
    Serial.println(" ppm");
    
    delay(2000);
}

Dengan Mode Debug :

#include <EEPROM.h>
#include "ESP32GravityTDS.h"

ESP32GravityTDS tdsSensor;

void setup() {
    Serial.begin(115200);
    EEPROM.begin(32);
    
    tdsSensor.setPin(34);
    tdsSensor.setAref(3.3);
    tdsSensor.setAdcRange(4096);
    tdsSensor.setTemperature(25);
    tdsSensor.enableDebug(true);  // Aktifkan debug
    
    tdsSensor.begin();
}

void loop() {
    tdsSensor.update();
    
    float tds = tdsSensor.getTdsValue();
    int rawADC = tdsSensor.getRawAnalogValue(); // Dapatkan nilai mentah
    
    Serial.print("RAW ADC: ");
    Serial.print(rawADC);
    Serial.print(" | TDS: ");
    Serial.print(tds, 0);
    Serial.println(" ppm");
    
    // Keterangan kualitas air (Standar WHO)
    if(tds <= 50) Serial.println("💧 Kualitas: Sangat Baik");
    else if(tds <= 150) Serial.println("💧 Kualitas: Baik");
    else if(tds <= 250) Serial.println("💧 Kualitas: Cukup Baik");
    else if(tds <= 500) Serial.println("💧 Kualitas: Sedang - Layak Minum");
    else if(tds <= 1000) Serial.println("⚠️ Kualitas: Kurang Baik");
    else Serial.println("❌ Kualitas: TIDAK LAYAK MINUM!");
    
    delay(2000);
}

📊 Kalibrasi Sensor
Mengapa Perlu Kalibrasi?
Sensor memiliki nilai default K = 1.0, tetapi kalibrasi akan meningkatkan akurasi pengukuran.

Persiapan Kalibrasi
Larutan standar 707ppm (1413 µS/cm) @25°C - Direkomendasikan DFRobot

Atau TDS meter referensi lain yang sudah terkalibrasi

Langkah Kalibrasi
Langkah	Perintah	Keterangan
1	ENTER	Masuk mode kalibrasi
2	Celupkan sensor	Ke larutan standar
3	CAL:707	Kirim nilai standar (ganti 707 sesuai larutan)
4	EXIT	Simpan & keluar mode kalibrasi


Contoh Output Kalibrasi :
>>>Enter Calibration Mode<<<
>>>Please put the probe into the standard buffer solution<<<

>>>Confrim Successful,K:1.28, Send EXIT to Save and Exit<<<

>>>Calibration Successful,K Value Saved,Exit Calibration Mode<<<

📚 API Reference
Konfigurasi
Method	Parameter	Deskripsi
setPin(pin)	int pin	Set pin ADC yang digunakan
setTemperature(temp)	float temp	Set suhu air (°C) untuk kompensasi
setAref(value)	float value	Set tegangan referensi (ESP32 = 3.3)
setAdcRange(range)	float range	Set range ADC (ESP32 = 4096)
setResolution(bits)	int bits	Set resolusi ADC (default 12)
setKvalueAddress(addr)	int addr	Set alamat EEPROM untuk K value
enableDebug(enable)	bool enable	Aktifkan/nonaktifkan debug mode

Pembacaan
Method	Return	Deskripsi
begin()	void	Inisialisasi sensor
update()	void	Baca & hitung nilai terbaru
getTdsValue()	float	Dapatkan nilai TDS (ppm)
getEcValue()	float	Dapatkan nilai EC (µS/cm)
getKvalue()	float	Dapatkan nilai K saat ini
getRawAnalogValue()	int	Dapatkan nilai ADC mentah
⚙️ Konfigurasi ADC ESP32
Library ini secara otomatis mengkonfigurasi ADC ESP32 dengan:

Resolution: 12-bit (0-4095)

Attenuation: ADC_11db (range 0-3.6V)

Anda dapat mengubahnya secara manual:
tdsSensor.setResolution(10);     // Ubah ke 10-bit
tdsSensor.setAttenuation(6);     // Ubah attenuation

📋 Standar Kualitas Air (WHO)
TDS (ppm)	Kualitas
0 - 50	Sangat Baik
50 - 150	Baik
150 - 250	Cukup Baik
250 - 500	Sedang (Layak Minum)
500 - 1000	Kurang Baik
> 1000	Tidak Layak Minum
Sumber: World Health Organization (WHO)
>
> 🐛 Troubleshooting
Masalah: Nilai ADC selalu 0
Solusi:

Pastikan sensor terendam air (air murni bisa baca 0)

Coba tambahkan sedikit garam untuk test

Aktifkan debug mode: tdsSensor.enableDebug(true)

Masalah: Nilai tidak stabil
Solusi:

Pastikan probe tidak terkena cahaya langsung

Biarkan probe terendam beberapa saat hingga stabil

Cek koneksi kabel

Masalah: EEPROM tidak menyimpan
Solusi:
EEPROM.begin(32);  // Pastikan ini dipanggil di setup()

📄 Lisensi
Original library by DFRobot (Jason) - LGPLv3 License
ESP32 modification - LGPLv3 License

🙏 Kredit
Original library: DFRobot Gravity: Analog TDS Sensor

Modifikasi untuk ESP32: [AripJam18]

🤝 Kontribusi
Pull request dan issue selalu diterima! Pastikan:

Testing di ESP32 devkit

Update dokumentasi jika perlu

Ikuti format kode yang ada
