// include/hardware/CapacitiveSensor.h
#ifndef CAPACITIVESENSOR_H
#define CAPACITIVESENSOR_H

#include <cstdint>
#include <functional>

class HeaterController; // Forward declaration

class CapacitiveSensor {
public:
    CapacitiveSensor(HeaterController& heater, std::function<void(bool)> heatingCallback);

    void update();
    bool isCapDetected() const;
    int getCurrentValue() const;

private:
    HeaterController& heater;
    std::function<void(bool)> onHeatingTrigger;

    struct {
        bool capDetected = false;
        uint32_t lastStateChangeTime = 0;
        const uint32_t debounceDelay = 150;
        bool lastRawState = false;
        int currentValue = 0;
        const int baseThreshold = 50;
        int interferenceOffset = 150;
        const int onThreshold = 50;
        const int offThreshold = 70;
    } state;
};

#endif // CAPACITIVESENSOR_H
