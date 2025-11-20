#pragma once

#include "ui/base/Screen.h"

struct HeatState {
    uint8_t currentCycle = 1;
    uint8_t power;
    uint8_t targetTemp;
    uint16_t currentTemp;
    bool isHeating = false;
    uint32_t elapsedSeconds;
    float progress;
};

void HeatUI(RenderSurface s, HeatState state);
