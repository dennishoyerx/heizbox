#include "IRTempSensor.h"
#include <Arduino.h>

IRTempSensor::IRTempSensor(uint8_t sda_pin, uint8_t scl_pin, uint16_t readIntervalMs)
    : sdaPin(sda_pin),
      sclPin(scl_pin),
      ITemperatureSensor(readIntervalMs) {
    pinMode(sda_pin, INPUT_PULLUP);
    pinMode(scl_pin, INPUT_PULLUP);
}

bool IRTempSensor::begin() {
    Wire.begin(sdaPin, sclPin);

    if (!mlx.begin()) {
        errorCount++;
        return false;
    }

    delay(100);
    mlx.writeEmissivity(0.95);  
    //setEmissivity(0.95);

    lastValidTemp = NAN;
    errorCount = 0;
    return true;
}

float IRTempSensor::read() {
    float reading = mlx.readObjectTempC();
    //float ambient = mlx.readAmbientTempC();
    //reading -= (ambient - 25) * 0.2;
    return reading;
}


bool IRTempSensor::setEmissivity(float emissivity) {
    mlx.writeEmissivity(emissivity);  
    return getEmissivity() == emissivity;

/*
    // Emissivity muss zwischen 0.1 und 1.0 sein
    if (emissivity < 0.1 || emissivity > 1.0) return false;
    
    // Konvertiere zu MLX90614 Format (0x0000 bis 0xFFFF)
    uint16_t emissValue = (uint16_t)(emissivity * 65535);
    
    // Schreibe ins RAM (nicht permanent)
    Wire.beginTransmission(0x5A);
    Wire.write(0x04); // Emissivity Register (RAM)
    Wire.write(emissValue & 0xFF);        // Low Byte
    Wire.write((emissValue >> 8) & 0xFF); // High Byte
    
    uint8_t pec = 0; // Simplified - no PEC for now
    Wire.write(pec);
    
    return Wire.endTransmission() == 0;*/
}

float IRTempSensor::getEmissivity() {
     return mlx.readEmissivity();
    /*
    Wire.beginTransmission(0x5A);
    Wire.write(0x24); // Read Emissivity from EEPROM
    Wire.endTransmission(false);
    
    if (Wire.requestFrom(0x5A, 3) == 3) {
        uint16_t emissValue = Wire.read();
        emissValue |= Wire.read() << 8;
        Wire.read(); // PEC
        
        return emissValue / 65535.0;
    }
    return 0.95; // Default*/
}