#pragma once

#include "heater/HeaterState.h"
#include <functional>


struct Limit {
    int limit;
    Limit(int l) : limit(l) {}
    void set(int l) { limit = l; }
    bool hasExceeded(int current) { return current > limit; }
};

namespace Safety {

    bool cutoffTemperatureReached(HeaterState& hs) {
        if (hs.tempLimit == 420) return false;
        return hs.temp > hs.tempLimit;
    }

    bool noTempClimb(HeaterState& hs) {
        static const int values[] = { };

        hs.temp;
        return false;
    };
    
    bool timeLimit(HeaterState& hs) {
        return hs.timer >= hs.cycleTimeout;
    };


    bool checkFailed() {
        auto& hs = HeaterState::instance();
        if (cutoffTemperatureReached(hs)) return true;
        if (noTempClimb(hs)) return true;
        if (timeLimit(hs)) return true;

        return false;
    };


    class WatchdogCheck {};
 

};

struct Safety_ {
    struct Config {
        int8_t tempLimit;
        int8_t timeLimit;
    };
    
    bool limitsExceeded() {
        return temp.hasExceeded(0) || time.hasExceeded(0);
    }

    bool checkFailed() {
        if (limitsExceeded()) return true;
        return false;
    };

private:
    Limit time{60};
    Limit temp{240};
};
