// TempSensor.cpp
#include "hardware/sensor/TempSensor.h"
#include <MAX6675.h>
#include <Arduino.h>

TempSensor::TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin, uint16_t readIntervalMs)
    : sckPin(sck_pin), csPin(cs_pin), soPin(so_pin),
      lastValidTemp(NAN), clickThreshold(190.0f), clickReached(false),
      errorCount(0), lastReadTime(0), readInterval(readIntervalMs)
{
    thermocouple = new MAX6675(sck_pin, cs_pin, so_pin); // Software-SPI Constructor
}

bool TempSensor::begin() {
    float temp = thermocouple->readCelsius();
    if (isnan(temp)) return false;
    lastValidTemp = temp;
    lastReadTime = millis();
    return true;
}

// Nicht-blockierendes Update
void TempSensor::update() {
    unsigned long now = millis();
    if (now - lastReadTime >= readInterval) {
        lastReadTime = now;
        float temp = thermocouple->readCelsius();

        if (validateReading(temp)) {
            lastValidTemp = temp;
            errorCount = 0;
        } else {
            errorCount++;
            if (errorCount >= 5) lastValidTemp = NAN; // persistent error
        }
    }
}

float TempSensor::getTemperature() {
    return lastValidTemp;
}

bool TempSensor::validateReading(float temp) const {
    return !isnan(temp) && temp >= -20.0f && temp <= 400.0f;
}

bool TempSensor::hasError() const {
    return isnan(lastValidTemp);
}

void TempSensor::setClickThreshold(float clickTemp) {
    clickThreshold = clickTemp;
}

bool TempSensor::hasReachedClickTemp() {
    float temp = getTemperature();
    if (!isnan(temp) && temp >= clickThreshold && !clickReached) {
        clickReached = true;
        return true;
    }
    if (temp < clickThreshold) clickReached = false;
    return false;
}
