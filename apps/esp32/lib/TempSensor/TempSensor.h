#pragma once
#include <MAX6675.h>
#include <Arduino.h>
#include "ITemperatureSensor.h"

class TempSensor: public ITemperatureSensor {
public:
    TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin, uint16_t readIntervalMs = 500);
    bool begin();
    float read() override;

private:
    MAX6675* thermocouple;
    uint8_t sckPin, csPin, soPin;
};
