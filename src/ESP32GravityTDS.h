/***************************************************
 DFRobot Gravity: Analog TDS Sensor/Meter
 ESP32 Compatible Version
 <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>
 
 Modified for ESP32 - 2026
 Added ADC configuration for ESP32 compatibility
****************************************************/

#ifndef GRAVITY_TDS_H
#define GRAVITY_TDS_H

#include "Arduino.h"

#define ReceivedBufferLength 15
#define TdsFactor 0.5  // tds = ec / 2

// Deteksi platform
#if defined(ESP32)
  #define IS_ESP32 true
#else
  #define IS_ESP32 false
#endif

class ESP32GravityTDS
{
public:
    ESP32GravityTDS();
    ~ESP32GravityTDS();

    void begin();  //initialization
    void update(); //read and calculate
    void setPin(int pin); 
    void setTemperature(float temp);  //set the temperature and execute temperature compensation
    void setAref(float value);  //reference voltage on ADC, default 5.0V on Arduino UNO
    void setAdcRange(float range);  //1024 for 10bit ADC;4096 for 12bit ADC
    void setKvalueAddress(int address); //set the EEPROM address to store the k value,default address:0x08
    void setResolution(int bits); // NEW: set ADC resolution for ESP32 (default 12)
    void setAttenuation(int atten); // NEW: set ADC attenuation for ESP32
    float getKvalue(); 
    float getTdsValue();
    float getEcValue();
    
    // NEW: Debug function untuk ESP32
    void enableDebug(bool enable);
    int getRawAnalogValue(); // Dapatkan nilai ADC mentah untuk debugging

private:
    int pin;
    float aref;  // default 5.0V on Arduino UNO
    float adcRange;
    float temperature;
    int kValueAddress;     //the address of the K value stored in the EEPROM
    char cmdReceivedBuffer[ReceivedBufferLength+1];   // store the serial cmd from the serial monitor
    byte cmdReceivedBufferIndex;
 
    float kValue;      // k value of the probe,you can calibrate in buffer solution ,such as 706.5ppm(1413us/cm)@25^C 
    float analogValue;
    float voltage;
    float ecValue; //before temperature compensation
    float ecValue25; //after temperature compensation
    float tdsValue;
    
    // NEW: ESP32 specific settings
    int adcResolution;  // ADC resolution in bits (default 12 for ESP32)
    int adcAttenuation; // ADC attenuation (default 11 for 0-3.6V)
    bool debugMode;     // Debug mode flag
    int lastRawValue;   // Store last raw ADC value for debugging

    void readKValues();
    boolean cmdSerialDataAvailable();
    byte cmdParse();
    void ecCalibration(byte mode);
    void configureADC(); // NEW: Configure ADC for current platform
    int readAnalogPin(); // NEW: Platform-specific analog read
};  

#endif
