#pragma once

#include "forward.h"

#include "heater/HeaterController.h" // CONVERTED: Use forward.h

class HeaterMonitor {
public:
    HeaterMonitor(HeaterController& heater);

    void checkHeatingStatus();
    void heatCycleCompleted(uint32_t duration);

private:
    HeaterController& heater;
    bool lastHeatingStatusSent = false;
};
