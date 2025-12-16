#pragma once
#include "heater\HeaterController.h"

class HeaterCycle {
public:
    HeaterCycle();
    void init();
    void update();

    static uint8_t current();
    static uint8_t set(uint8_t cycle);
    static bool is(uint8_t cycle);
    static uint8_t next();

    HeaterCycle& instance();
private:
};
/*
class Sensors {
public:
    Sensors();

    static uint8_t current();
    static uint8_t target();
    static uint8_t setTarget(uint8_t cycle);
};*/