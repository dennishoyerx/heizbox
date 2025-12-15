#pragma once
#include "ITemperatureSensor.h"
#include <Arduino.h>

bool ITemperatureSensor::update(bool ignoreInterval) {
    unsigned long now = millis();
    if (ignoreInterval || now - lastReadTime >= readInterval) {
        lastReadTime = now;
        float temp = read();

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

bool ITemperatureSensor::validateReading(float temp) const {
    if (isnan(temp)) return false;
    if (temp < -20 || temp > 500) return false;

    if (!isnan(lastValidTemp)) {
        float delta = temp - lastValidTemp;
        if (delta < -8.0f) {  // keine 8°C Sekundeneinbruch während Heizen
            return false;
        }
        if (delta > 50.0f) { // keine 50°C Sprünge nach oben
            return false;
        }
    }

    return true;
}

float ITemperatureSensor::getCelsius() {
    return lastValidTemp;
}

void ITemperatureSensor::setReadInterval(uint16_t interval) {
    readInterval = interval;
}

uint16_t ITemperatureSensor::getReadInterval() const {
    return readInterval;
}

bool ITemperatureSensor::hasError() const {
    return false;
}
