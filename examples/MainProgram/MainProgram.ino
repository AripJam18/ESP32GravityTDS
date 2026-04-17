#include <EEPROM.h>
#include "ESP32GravityTDS.h"

ESP32GravityTDS tdsSensor;

void setup() {
    Serial.begin(115200);
    
    // Inisialisasi EEPROM (ESP32 butuh ukuran)
    EEPROM.begin(32);  // ESP32 requires EEPROM.begin()
    
    // Konfigurasi sensor untuk ESP32
    tdsSensor.setPin(32);           // Gunakan GPIO32 (ADC1, aman dengan WiFi)
    tdsSensor.setAref(3.3);         // ESP32 menggunakan 3.3V
    tdsSensor.setAdcRange(4096);    // 12-bit ADC (0-4095)
    tdsSensor.setTemperature(25);   // Suhu awal (akan diupdate jika ada sensor)
    
    // Optional: Set resolusi ADC (default 12-bit untuk ESP32)
    tdsSensor.setResolution(12);
    
    // Optional: Enable debug mode untuk troubleshooting
    tdsSensor.enableDebug(true);
    
    // Begin sensor
    tdsSensor.begin();
    
    Serial.println("TDS Sensor ESP32 Ready!");
    Serial.println("=========================");
}

void loop() {
    // Update pembacaan sensor
    tdsSensor.update();
    
    // Dapatkan nilai TDS
    float tdsValue = tdsSensor.getTdsValue();
    float ecValue = tdsSensor.getEcValue();
    float kValue = tdsSensor.getKvalue();
    
    // Tampilkan hasil
    Serial.print("TDS: ");
    Serial.print(tdsValue, 0);
    Serial.print(" ppm | EC: ");
    Serial.print(ecValue, 2);
    Serial.print(" uS/cm | K: ");
    Serial.println(kValue, 3);
    
    // Keterangan kualitas air
    if(tdsValue <= 50) Serial.println("Kualitas: Sangat Baik");
    else if(tdsValue <= 150) Serial.println("Kualitas: Baik");
    else if(tdsValue <= 250) Serial.println("Kualitas: Cukup Baik");
    else if(tdsValue <= 500) Serial.println("Kualitas: Sedang");
    else if(tdsValue <= 1000) Serial.println("Kualitas: Kurang Baik");
    else Serial.println("Kualitas: TIDAK LAYAK MINUM!");
    
    Serial.println("-------------------------");
    
    delay(2000);
}
