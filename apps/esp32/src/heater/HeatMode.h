#pragma once

#include "heater/Sensors.h"

class TemperatureControl {
public:
    TemperatureControl();
    void setTargetTemperature(uint16_t temp);
    void setCutoffTemperature(uint16_t temp);
    bool checkTemperature(Sensors::Sensor sensorType);
    uint16_t getCurrentTemperature() const;
    
private:
    Sensors sensors;

    uint16_t targetTemperature;
    uint16_t currentTemperature;
    uint16_t cutoffTemperature;
};

class HeatMode {
public:
    enum Modes {
        TEMP_TARGET_CUTOFF,
        TEMP_TARGET
    };

private:
    Modes mode;
};