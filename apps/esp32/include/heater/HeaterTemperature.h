#pragma once
#include <TempSensor.h>
#include <IRTempSensor.h>

enum HeaterTemperatures {
    MAIN,
    K,
    IR
};


class HeaterTemperature {
public:
    HeaterTemperature();
    void init();
    bool update(HeaterTemperatures type = MAIN, bool ignoreInterval = false);

    uint16_t get(HeaterTemperatures type = MAIN);

    TempSensor* getKSensor() { return &kSensor; }
    IRTempSensor* getIRSensor() { return &irSensor; }

    bool limitReached();

private:
    TempSensor kSensor;
    IRTempSensor irSensor;
    float lastTemperature;

    uint16_t validate(uint16_t temp);
    void handleInitializationError();
};
