#pragma once 

#include "heater/HeaterState.h"

namespace SSafety {
    bool TimeLimitReached() {
        auto& hs = HeaterState::instance();
        return hs.timer >= hs.cycleTimeout;
    };    
};


class SafetyCheck {
    
};