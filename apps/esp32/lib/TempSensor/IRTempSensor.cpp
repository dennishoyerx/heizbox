#include "IRTempSensor.h"

IRTempSensor::IRTempSensor(uint8_t sda_pin, uint8_t scl_pin, uint16_t readIntervalMs)
    : sdaPin(sda_pin),
      sclPin(scl_pin),
      ITemperatureSensor(readIntervalMs) {
    pinMode(sda_pin, INPUT_PULLUP);
    pinMode(scl_pin, INPUT_PULLUP);
}

bool IRTempSensor::begin(float emissivity) {
    Wire.begin(sdaPin, sclPin);

    if (!mlx.begin()) {
        errorCount++;
        return false;
    }

    delay(100);
    setEmissivity(emissivity);

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
    if (emissivity < 0.1 || emissivity > 1.0) return false;
    
    mlx.writeEmissivity(emissivity);  
    delay(10); // EEPROM Write Zeit
    
    // Wichtig: Sensor stabilisieren nach EEPROM-Write
    for(int i = 0; i < 5; i++) {
        mlx.readObjectTempC();
        delay(20);
    }
    
    return abs(mlx.readEmissivity() - emissivity) < 0.01;
}

float IRTempSensor::getEmissivity() {
    return mlx.readEmissivity();
}