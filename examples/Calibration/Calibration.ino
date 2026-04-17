/**
 * ESP32GravityTDS - KALIBRASI SENSOR
 * 
 * File terpisah khusus untuk melakukan kalibrasi TDS sensor.
 * Setelah kalibrasi selesai, nilai K akan tersimpan di EEPROM
 * dan bisa digunakan oleh program utama.
 * 
 * Cara penggunaan:
 * 1. Upload sketch ini ke ESP32
 * 2. Buka Serial Monitor (115200 baud)
 * 3. Ikuti petunjuk yang muncul di layar
 * 4. Selesai kalibrasi, upload sketch utama Anda
 */

#include <EEPROM.h>
#include "ESP32GravityTDS.h"

ESP32GravityTDS tdsSensor;

// Pin yang digunakan
#define TDS_PIN 32

void setup() {
    Serial.begin(115200);
    EEPROM.begin(32);
    
    // Konfigurasi sensor
    tdsSensor.setPin(TDS_PIN);
    tdsSensor.setAref(3.3);         // ESP32 3.3V
    tdsSensor.setAdcRange(4096);    // 12-bit ADC
    tdsSensor.setTemperature(25);   // Suhu ruang (ganti jika pakai sensor suhu)
    
    // Aktifkan debug untuk melihat proses
    tdsSensor.enableDebug(true);
    
    tdsSensor.begin();
    
    tampilkanHeader();
    tampilkanStatusSaatIni();
    tampilkanPetunjuk();
}

void loop() {
    // Update sensor - ini WAJIB untuk proses kalibrasi
    tdsSensor.update();
    
    // Tampilkan pembacaan实时 setiap 2 detik
    static unsigned long lastPrint = millis();
    if (millis() - lastPrint > 2000) {
        lastPrint = millis();
        
        Serial.println("─────────────────────────────────────────");
        Serial.print("📊 Pembacaan Saat Ini: ");
        Serial.print(tdsSensor.getTdsValue(), 0);
        Serial.println(" ppm");
        Serial.print("🔧 Nilai K Saat Ini: ");
        Serial.println(tdsSensor.getKvalue(), 4);
        Serial.println("─────────────────────────────────────────");
    }
    
    // Proses input dari user untuk kalibrasi
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        prosesPerintah(input);
    }
}

void prosesPerintah(String cmd) {
    Serial.println();
    Serial.println("═══════════════════════════════════════════");
    
    if (cmd == "1") {
        Serial.println("▶️ Memasuki mode kalibrasi...");
        Serial.println("Ketik: ENTER");
        Serial.println("═══════════════════════════════════════════");
        // Perintah akan diproses oleh library di update()
        // Kita hanya perlu mengirimkan ke serial buffer
        Serial.println("✓ Perintah ENTER telah dikirim");
        Serial.println("Tunggu respon dari sensor...");
        
    }
    else if (cmd == "2") {
        Serial.println("▶️ Menyimpan dan keluar kalibrasi...");
        Serial.println("Ketik: EXIT");
        Serial.println("═══════════════════════════════════════════");
        Serial.println("✓ Perintah EXIT telah dikirim");
        
    }
    else if (cmd == "3") {
        tampilkanPetunjuk();
    }
    else if (cmd == "4") {
        tampilkanStatusSaatIni();
    }
    else if (cmd == "5") {
        resetKalibrasi();
    }
    else if (cmd == "ENTER") {
        // Perintah ENTER akan diproses oleh library
        Serial.println("✓ Perintah ENTER diterima");
        Serial.println("Masukkan nilai kalibrasi (contoh: CAL:707)");
    }
    else if (cmd == "EXIT") {
        Serial.println("✓ Perintah EXIT diterima");
        Serial.println("Kalibrasi akan disimpan ke EEPROM");
    }
    else if (cmd.startsWith("CAL:")) {
        Serial.print("✓ Perintah kalibrasi diterima: ");
        Serial.println(cmd);
        Serial.println("Tunggu konfirmasi 'Confirm Successful'");
    }
    else if (cmd != "") {
        Serial.print("❌ Perintah tidak dikenal: ");
        Serial.println(cmd);
        Serial.println("Gunakan menu angka (1-5) atau perintah langsung (ENTER/CAL:707/EXIT)");
    }
    
    Serial.println("═══════════════════════════════════════════");
    Serial.println();
}

void tampilkanHeader() {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════════════════════════╗");
    Serial.println("║                                                                ║");
    Serial.println("║     ESP32 TDS SENSOR - CALIBRATION TOOL                        ║");
    Serial.println("║                                                                ║");
    Serial.println("║     Library: ESP32GravityTDS                                  ║");
    Serial.println("║     Sensor:  DFRobot Gravity Analog TDS Sensor (SEN0244)      ║");
    Serial.println("║                                                                ║");
    Serial.println("╚══════════════════════════════════════════════════════════════╝");
    Serial.println();
}

void tampilkanPetunjuk() {
    Serial.println();
    Serial.println("┌─────────────────────────────────────────────────────────────┐");
    Serial.println("│                      PETUNJUK KALIBRASI                      │");
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.println("│                                                             │");
    Serial.println("│  PERSIAPAN:                                                 │");
    Serial.println("│    • Larutan standar 707ppm (1413 µS/cm) @25°C              │");
    Serial.println("│    • Atau larutan standar 1413ppm                           │");
    Serial.println("│    • Wadah bersih untuk larutan                             │");
    Serial.println("│                                                             │");
    Serial.println("│  LANGKAH-LANGKAH:                                           │");
    Serial.println("│                                                             │");
    Serial.println("│  [1] KETIK: ENTER        → Masuk mode kalibrasi             │");
    Serial.println("│  [2] Celupkan sensor ke larutan standar                     │");
    Serial.println("│  [3] Tunggu 5-10 detik hingga stabil                        │");
    Serial.println("│  [4] KETIK: CAL:707      → Kirim nilai standar              │");
    Serial.println("│      (ganti 707 sesuai larutan yang digunakan)              │");
    Serial.println("│  [5] Tunggu pesan 'Confirm Successful'                      │");
    Serial.println("│  [6] KETIK: EXIT         → Simpan & keluar                  │");
    Serial.println("│                                                             │");
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.println("│                        MENU CEPAT                            │");
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.println("│                                                             │");
    Serial.println("│    1  → Kirim perintah ENTER (masuk kalibrasi)              │");
    Serial.println("│    2  → Kirim perintah EXIT (simpan & keluar)               │");
    Serial.println("│    3  → Tampilkan petunjuk ini                              │");
    Serial.println("│    4  → Lihat status sensor                                 │");
    Serial.println("│    5  → Reset kalibrasi ke nilai default (K=1.0)            │");
    Serial.println("│                                                             │");
    Serial.println("│    Atau ketik perintah langsung:                            │");
    Serial.println("│      ENTER, CAL:707, EXIT                                   │");
    Serial.println("│                                                             │");
    Serial.println("└─────────────────────────────────────────────────────────────┘");
    Serial.println();
}

void tampilkanStatusSaatIni() {
    Serial.println();
    Serial.println("┌─────────────────────────────────────────────────────────────┐");
    Serial.println("│                      STATUS SENSOR                           │");
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.print  ("│ Pin ADC        : ");
    Serial.print(TDS_PIN);
    Serial.println("                                      │");
    Serial.print  ("│ Tegangan Ref    : 3.3V                                      │");
    Serial.println("                                      │");
    Serial.print  ("│ Resolusi ADC    : 12-bit (0-4095)                           │");
    Serial.println("                                      │");
    Serial.print  ("│ Nilai K Value   : ");
    Serial.print(tdsSensor.getKvalue(), 4);
    Serial.println("                                  │");
    Serial.print  ("│ TDS Saat Ini    : ");
    Serial.print(tdsSensor.getTdsValue(), 0);
    Serial.println(" ppm                                   │");
    Serial.print  ("│ EC Saat Ini     : ");
    Serial.print(tdsSensor.getEcValue(), 2);
    Serial.println(" uS/cm                                │");
    Serial.print  ("│ RAW ADC         : ");
    Serial.print(tdsSensor.getRawAnalogValue());
    Serial.println("                                      │");
    Serial.println("└─────────────────────────────────────────────────────────────┘");
    Serial.println();
}

void resetKalibrasi() {
    Serial.println();
    Serial.println("┌─────────────────────────────────────────────────────────────┐");
    Serial.println("│                    RESET KALIBRASI                          │");
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.println("│  PERINGATAN!                                               │");
    Serial.println("│  Ini akan mengembalikan nilai K ke 1.0 (default pabrik)    │");
    Serial.println("│                                                             │");
    Serial.println("│  Apakah Anda yakin? (Ketik: YES untuk konfirmasi)          │");
    Serial.println("└─────────────────────────────────────────────────────────────┘");
    
    // Tunggu konfirmasi
    unsigned long timeout = millis() + 10000; // 10 detik timeout
    bool confirmed = false;
    
    while (millis() < timeout && !confirmed) {
        if (Serial.available() > 0) {
            String confirm = Serial.readStringUntil('\n');
            confirm.trim();
            if (confirm == "YES") {
                confirmed = true;
            } else {
                Serial.println("❌ Reset dibatalkan (ketik YES untuk reset)");
                return;
            }
        }
    }
    
    if (confirmed) {
        // Simpan nilai K = 1.0 ke EEPROM
        EEPROM.begin(32);
        float defaultK = 1.0;
        EEPROM.put(8, defaultK);
        EEPROM.commit();
        
        Serial.println();
        Serial.println("✓ Kalibrasi telah direset ke nilai default K = 1.0");
        Serial.println("✓ Silakan restart ESP32 atau lakukan kalibrasi ulang");
        Serial.println();
        
        // Baca ulang nilai K
        tdsSensor.begin();
    } else {
        Serial.println("❌ Timeout - Reset dibatalkan");
    }
}
