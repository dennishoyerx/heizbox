#pragma once
#include <MAX6675.h>
#include <Arduino.h>

class TempSensor {
public:
    TempSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t so_pin, uint16_t readIntervalMs = 500);
    bool begin();
    float getTemperature();
    bool update(bool ignoreInterval = false);

    void setClickThreshold(float clickTemp);
    bool hasReachedClickTemp();
    bool hasError() const;

    void setReadInterval(uint16_t interval);
    uint16_t getReadInterval() const;

private:
    MAX6675* thermocouple;
    uint8_t sckPin, csPin, soPin;
    float lastValidTemp;
    float clickThreshold;
    bool clickReached;
    uint8_t errorCount;

    unsigned long lastReadTime;
    uint16_t readInterval; // ms

    bool validateReading(float temp) const;
};
