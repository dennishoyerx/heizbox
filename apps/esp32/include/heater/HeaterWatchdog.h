#pragma once
#include "heater\HeaterController.h"


class TemperatureWatchdog {
public:
    TemperatureWatchdog();
    void init();
    void update();

private:
    TempSensor tempSensor;
    float lastTemperature;
};

class HeaterWatchdog {
public:
    HeaterWatchdog();
    void init();
    void update();

private:
    HeaterController heater;
    TemperatureWatchdog tempWatchdog;

    void checkTemperature();

};