#pragma once

#include "ui/base/Screen.h"
#include "ui/base/UI.h"
#include "heater/ZVSDriver.h"

struct HeatState {
    uint8_t currentCycle = 1;
    uint8_t power;
    uint8_t targetTemp;
    uint16_t currentTemp;
    bool isHeating = false;
    uint32_t elapsedSeconds;
    float progress;
};

void HeatUI(UI* _ui, HeatState state, ZVSDriver* zvs);