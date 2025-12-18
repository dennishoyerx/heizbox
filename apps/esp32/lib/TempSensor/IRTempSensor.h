#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "ITemperatureSensor.h"

class IRTempSensor: public ITemperatureSensor {
public:
    IRTempSensor(uint8_t sda_pin, uint8_t scl_pin, uint16_t readIntervalMs = 100);

    bool begin(float emissivity = 0.95);
    
    float read() override;

    bool setEmissivity(float emissivity);
    float getEmissivity();

private:
    Adafruit_MLX90614 mlx;

    uint8_t sdaPin;
    uint8_t sclPin;
};
