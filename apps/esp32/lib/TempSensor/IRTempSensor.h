#pragma once
#include <Wire.h>
#include <Adafruit_MLX90614.h>

class IRTempSensor {
public:
    IRTempSensor(uint8_t sda_pin, uint8_t scl_pin, uint16_t readIntervalMs = 100);

    bool begin();
    void update();
    bool hasError() const;

    void setReadInterval(uint16_t interval);
    uint16_t getReadInterval() const;
    
    float getTemperature();

    bool setEmissivity(float emissivity);
    float getEmissivity();

private:
    Adafruit_MLX90614 mlx;

    uint8_t sdaPin;
    uint8_t sclPin;

    float lastValidTemp;
    uint8_t errorCount;

    unsigned long lastReadTime;
    uint16_t readInterval;

    bool validateReading(float temp) const;
};
