// src/hardware/CapacitiveSensor.cpp
#include "hardware/CapacitiveSensor.h"
#include "core/Device.h"
#include "core/StateManager.h"
#include "hardware/HeaterController.h"
#include "utils/Logger.h"
#include <Arduino.h>

CapacitiveSensor::CapacitiveSensor(HeaterController& h, std::function<void(bool)> heatingCb)
    : heater(h), onHeatingTrigger(heatingCb) {}

void CapacitiveSensor::update() {
    state.currentValue = touchRead(32);

    bool rawState;
    int effectiveOnThreshold = state.onThreshold;
    int effectiveOffThreshold = state.offThreshold;

    if (heater.isHeating()) {
        effectiveOnThreshold += state.interferenceOffset;
        effectiveOffThreshold += state.interferenceOffset;
    }

    if (state.currentValue < effectiveOnThreshold) {
        rawState = true;
    } else if (state.currentValue > effectiveOffThreshold) {
        rawState = false;
    } else {
        rawState = state.lastRawState;
    }

    if (rawState != state.lastRawState) {
        state.lastStateChangeTime = millis();
    }

    if ((millis() - state.lastStateChangeTime) > state.debounceDelay) {
        if (rawState != state.capDetected) {
            state.capDetected = rawState;

            if (DeviceState::instance().smart.get()) {
                onHeatingTrigger(state.capDetected);
            }

            if (state.capDetected) {
                logPrint("CapSensor", "Cap detected");
            } else {
                logPrint("CapSensor", "Cap released");
            }
        }
    }

    state.lastRawState = rawState;
}

bool CapacitiveSensor::isCapDetected() const {
    return state.capDetected;
}

int CapacitiveSensor::getCurrentValue() const {
    return state.currentValue;
}
