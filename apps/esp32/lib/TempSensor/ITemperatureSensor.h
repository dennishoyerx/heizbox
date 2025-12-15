#pragma once
#include <optional>

class ITemperatureSensor {
public:
    ITemperatureSensor(uint16_t readIntervalMs = 100);
    virtual ~ITemperatureSensor() = default;

    virtual float read() = 0;
    float getCelsius();

    bool hasError() const;

    void setReadInterval(uint16_t interval);
    uint16_t getReadInterval() const;

protected:
    float lastValidTemp;
    uint8_t errorCount;

    unsigned long lastReadTime;
    uint16_t readInterval;

    bool validateReading(float temp) const;
};