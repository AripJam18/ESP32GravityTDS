/***************************************************
 DFRobot Gravity: Analog TDS Sensor/Meter
 ESP32 Compatible Version
 <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>
 
 Modified for ESP32 - 2024
 Added ADC configuration for ESP32 compatibility
****************************************************/

#include <EEPROM.h>
#include "ESP32GravityTDS.h"

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

ESP32GravityTDS::ESP32GravityTDS()
{
    this->pin = A0;  // Default pin changed to A0 for better compatibility
    this->temperature = 25.0;
    this->aref = 3.3;  // CHANGED: ESP32 uses 3.3V reference
    this->adcRange = 4096.0;  // CHANGED: ESP32 has 12-bit ADC (0-4095)
    this->kValueAddress = 8;
    this->kValue = 1.0;
    
    // NEW: ESP32 default settings
    this->adcResolution = 12;  // 12-bit for ESP32
    this->adcAttenuation = 11; // ADC_11db for 0-3.6V range
    this->debugMode = false;
    this->lastRawValue = 0;
}

ESP32GravityTDS::~ESP32GravityTDS()
{
}

void ESP32GravityTDS::setPin(int pin)
{
    this->pin = pin;
}

void ESP32GravityTDS::setTemperature(float temp)
{
    this->temperature = temp;
}

void ESP32GravityTDS::setAref(float value)
{
    this->aref = value;
}

void ESP32GravityTDS::setAdcRange(float range)
{
    this->adcRange = range;
}

void ESP32GravityTDS::setKvalueAddress(int address)
{
    this->kValueAddress = address;
}

// NEW: Set ADC resolution for ESP32
void ESP32GravityTDS::setResolution(int bits)
{
    this->adcResolution = bits;
    this->adcRange = pow(2, bits);  // Update range based on resolution
    if(IS_ESP32 && this->debugMode) {
        Serial.print("[DEBUG] ADC Resolution set to ");
        Serial.print(bits);
        Serial.print(" bits, Range: 0-");
        Serial.println(this->adcRange);
    }
}

// NEW: Set ADC attenuation for ESP32
void ESP32GravityTDS::setAttenuation(int atten)
{
    this->adcAttenuation = atten;
    if(IS_ESP32 && this->debugMode) {
        Serial.print("[DEBUG] ADC Attenuation set to ");
        Serial.println(atten);
    }
}

// NEW: Enable/disable debug mode
void ESP32GravityTDS::enableDebug(bool enable)
{
    this->debugMode = enable;
    if(enable) {
        Serial.println("[DEBUG] Debug mode enabled for ESP32GravityTDS");
    }
}

// NEW: Get raw analog value for debugging
int ESP32GravityTDS::getRawAnalogValue()
{
    return this->lastRawValue;
}

// NEW: Configure ADC based on platform
void ESP32GravityTDS::configureADC()
{
#if defined(ESP32)
    // Konfigurasi khusus ESP32
    analogReadResolution(this->adcResolution);
    
    // Set attenuation (ADC_0db = 0, ADC_2_5db = 1, ADC_6db = 2, ADC_11db = 3)
    // Nilai 11 dalam kode ini merepresentasikan ADC_11db
    if(this->adcAttenuation == 11) {
        analogSetAttenuation(ADC_11db);  // Range 0-3.6V
    } else if(this->adcAttenuation == 6) {
        analogSetAttenuation(ADC_6db);   // Range 0-2.0V
    } else if(this->adcAttenuation == 2) {
        analogSetAttenuation(ADC_2_5db); // Range 0-1.1V
    } else {
        analogSetAttenuation(ADC_0db);   // Range 0-0.8V
    }
    
    // Optional: Set lebih akurat untuk pin tertentu
    analogSetPinAttenuation(this->pin, ADC_11db);
    
    if(this->debugMode) {
        Serial.println("[DEBUG] ESP32 ADC configured:");
        Serial.print("  - Resolution: ");
        Serial.print(this->adcResolution);
        Serial.println(" bits");
        Serial.println("  - Attenuation: ADC_11db (0-3.6V)");
        Serial.print("  - Pin: ");
        Serial.println(this->pin);
    }
#else
    // Untuk Arduino/ESP8266, tidak perlu konfigurasi khusus
    if(this->debugMode) {
        Serial.println("[DEBUG] Standard platform detected, no special ADC config needed");
    }
#endif
}

// NEW: Platform-specific analog read
int ESP32GravityTDS::readAnalogPin()
{
    int rawValue = analogRead(this->pin);
    this->lastRawValue = rawValue;
    
    if(this->debugMode) {
        static unsigned long lastDebugPrint = 0;
        if(millis() - lastDebugPrint > 2000) {  // Print every 2 seconds
            lastDebugPrint = millis();
            Serial.print("[DEBUG] Raw ADC: ");
            Serial.print(rawValue);
            Serial.print(" | Voltage: ");
            Serial.print(rawValue / this->adcRange * this->aref, 3);
            Serial.println(" V");
        }
    }
    
    return rawValue;
}

void ESP32GravityTDS::begin()
{
    pinMode(this->pin, INPUT);
    
    // Configure ADC for the platform
    configureADC();
    
    // Read K value from EEPROM
    readKValues();
    
    if(this->debugMode) {
        Serial.println("[DEBUG] ESP32GravityTDS initialized");
        Serial.print("  - Pin: ");
        Serial.println(this->pin);
        Serial.print("  - AREF: ");
        Serial.println(this->aref);
        Serial.print("  - ADC Range: 0-");
        Serial.println(this->adcRange);
        Serial.print("  - K Value: ");
        Serial.println(this->kValue);
    }
}

float ESP32GravityTDS::getKvalue()
{
    return this->kValue;
}

void ESP32GravityTDS::update()
{
    // Use platform-specific analog read
    this->analogValue = readAnalogPin();
    
    // Calculate voltage based on ADC reading
    this->voltage = this->analogValue / this->adcRange * this->aref;
    
    // Calculate EC value with K factor
    this->ecValue = (133.42 * this->voltage * this->voltage * this->voltage - 
                     255.86 * this->voltage * this->voltage + 
                     857.39 * this->voltage) * this->kValue;
    
    // Temperature compensation to 25°C
    this->ecValue25 = this->ecValue / (1.0 + 0.02 * (this->temperature - 25.0));
    
    // Calculate TDS value
    this->tdsValue = ecValue25 * TdsFactor;
    
    // Handle calibration commands
    if(cmdSerialDataAvailable() > 0)
    {
        ecCalibration(cmdParse());  // if received serial cmd from the serial monitor, enter into the calibration mode
    }
    
    // Debug output for TDS value
    if(this->debugMode) {
        static unsigned long lastTdsDebug = 0;
        if(millis() - lastTdsDebug > 2000) {
            lastTdsDebug = millis();
            Serial.print("[DEBUG] EC25: ");
            Serial.print(this->ecValue25, 2);
            Serial.print(" uS/cm | TDS: ");
            Serial.print(this->tdsValue, 0);
            Serial.println(" ppm");
        }
    }
}

float ESP32GravityTDS::getTdsValue()
{
    return tdsValue;
}

float ESP32GravityTDS::getEcValue()
{
    return ecValue25;
}

void ESP32GravityTDS::readKValues()
{
    EEPROM_read(this->kValueAddress, this->kValue);  
    if(EEPROM.read(this->kValueAddress)==0xFF && 
       EEPROM.read(this->kValueAddress+1)==0xFF && 
       EEPROM.read(this->kValueAddress+2)==0xFF && 
       EEPROM.read(this->kValueAddress+3)==0xFF)
    {
        this->kValue = 1.0;   // default value: K = 1.0
        EEPROM_write(this->kValueAddress, this->kValue);
        if(this->debugMode) {
            Serial.println("[DEBUG] No K value in EEPROM, using default K=1.0");
        }
    } else if(this->debugMode) {
        Serial.print("[DEBUG] K value loaded from EEPROM: ");
        Serial.println(this->kValue);
    }
}

boolean ESP32GravityTDS::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available()>0) 
    {   
        if (millis() - cmdReceivedTimeOut > 500U) 
        {
            cmdReceivedBufferIndex = 0;
            memset(cmdReceivedBuffer,0,(ReceivedBufferLength+1));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex==ReceivedBufferLength){
            cmdReceivedBufferIndex = 0;
            strupr(cmdReceivedBuffer);
            return true;
        }else{
            cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
            cmdReceivedBufferIndex++;
        }
    }
    return false;
}

byte ESP32GravityTDS::cmdParse()
{
    byte modeIndex = 0;
    if(strstr(cmdReceivedBuffer, "ENTER") != NULL) 
        modeIndex = 1;
    else if(strstr(cmdReceivedBuffer, "EXIT") != NULL) 
        modeIndex = 3;
    else if(strstr(cmdReceivedBuffer, "CAL:") != NULL)   
        modeIndex = 2;
    return modeIndex;
}

void ESP32GravityTDS::ecCalibration(byte mode)
{
    char *cmdReceivedBufferPtr;
    static boolean ecCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    float KValueTemp,rawECsolution;
    switch(mode)
    {
        case 0:
            if(enterCalibrationFlag)
                Serial.println(F("Command Error"));
            break;
        
        case 1:
            enterCalibrationFlag = 1;
            ecCalibrationFinish = 0;
            Serial.println();
            Serial.println(F(">>>Enter Calibration Mode<<<"));
            Serial.println(F(">>>Please put the probe into the standard buffer solution<<<"));
            Serial.println();
            break;
     
        case 2:
            cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "CAL:");
            cmdReceivedBufferPtr += strlen("CAL:");
            rawECsolution = strtod(cmdReceivedBufferPtr, NULL) / (float)(TdsFactor);
            rawECsolution = rawECsolution * (1.0 + 0.02 * (temperature - 25.0));
            if(enterCalibrationFlag)
            {
                KValueTemp = rawECsolution / (133.42 * voltage * voltage * voltage - 
                                              255.86 * voltage * voltage + 
                                              857.39 * voltage);
                if((rawECsolution>0) && (rawECsolution<2000) && (KValueTemp>0.25) && (KValueTemp<4.0))
                {
                    Serial.println();
                    Serial.print(F(">>>Confrim Successful,K:"));
                    Serial.print(KValueTemp);
                    Serial.println(F(", Send EXIT to Save and Exit<<<"));
                    kValue = KValueTemp;
                    ecCalibrationFinish = 1;
                }
                else
                {
                    Serial.println();
                    Serial.println(F(">>>Confirm Failed,Try Again<<<"));
                    Serial.println();
                    ecCalibrationFinish = 0;
                }        
            }
            break;

        case 3:
            if(enterCalibrationFlag)
            {
                Serial.println();
                if(ecCalibrationFinish)
                {
                    EEPROM_write(kValueAddress, kValue);
                    Serial.print(F(">>>Calibration Successful,K Value Saved"));
                }
                else 
                    Serial.print(F(">>>Calibration Failed"));       
                Serial.println(F(",Exit Calibration Mode<<<"));
                Serial.println();
                ecCalibrationFinish = 0;
                enterCalibrationFlag = 0;
            }
            break;
    }
}