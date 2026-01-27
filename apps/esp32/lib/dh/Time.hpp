#pragma once

#include <cstdint>
#include <Arduino.h>

/*class IStatic {
public:
    void deleteStatic() {};
};

class Statics {
public:
    Statics() {}

    template <typename T>
    T useStatic(IStatic st) {
        return st;
    }
};
void xx() {
    Statics st = Statics();
    Time::Throttle t = st.useStatic<Time::Throttle>(Time::Throttle(200));
    t.shouldTrigger();
}*/


namespace Time {

class Throttle {
private:
    uint32_t interval;
    uint32_t lastTrigger = 0;

public:
    explicit Throttle(uint32_t intervalMs) : interval(intervalMs) {}
    
    bool shouldTrigger() {
        uint32_t now = millis();
        if (now - lastTrigger >= interval) {
            lastTrigger = now;
            return true;
        }
        return false;
    }
    
    void reset() { lastTrigger = 0; }
};


}
