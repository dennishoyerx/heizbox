#include "TempSensor.h"
#include <MAX6675.h>
#include <Arduino.h>

TempSensor::TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin, uint16_t readIntervalMs)
    : sckPin(sck_pin), csPin(cs_pin), soPin(so_pin), ITemperatureSensor(readIntervalMs)
{
    thermocouple = new MAX6675(sck_pin, cs_pin, so_pin);
}

bool TempSensor::begin() {
    float temp = thermocouple->readCelsius();
    if (isnan(temp)) return false;
    lastValidTemp = temp;
    lastReadTime = millis();
    return true;
}

// Nicht-blockierendes Update
bool TempSensor::update(bool ignoreInterval) {
    unsigned long now = millis();
    if (ignoreInterval || now - lastReadTime >= readInterval) {
        lastReadTime = now;
        float temp = thermocouple->readCelsius();

        if (validateReading(temp)) {
            lastValidTemp = temp;
            errorCount = 0;
            return true;
        } else {
            errorCount++;
            if (errorCount >= 5) lastValidTemp = NAN; // persistent error
        }
    }
    return false;
}

float TempSensor::getTemperature() {
    return lastValidTemp;
}
