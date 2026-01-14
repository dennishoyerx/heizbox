#include "hardware/sensor/CapacitiveSensor.h"
#include "core/DeviceState.h"
#include "core/Device.h"
#include "heater/HeaterController.h"
#include <Arduino.h>

CapacitiveSensor::CapacitiveSensor(HeaterController& h, std::function<void(bool)> heatingCb)
    : heater(h), onHeatingTrigger(heatingCb) {}

void CapacitiveSensor::update() {
    state.currentValue = touchRead(32);
    bool rawState;

    if (heater.isHeating()) {
        // Heater is ON: value > 200 is active
        if (state.currentValue > state.onThreshold_HeaterOn) {
            rawState = true;
        } else if (state.currentValue < state.offThreshold_HeaterOn) {
            rawState = false;
        } else {
            rawState = state.lastRawState;
        }
    } else {
        // Heater is OFF: value < 45 is active
        if (state.currentValue < state.onThreshold_HeaterOff) {
            rawState = true;
        } else if (state.currentValue > state.offThreshold_HeaterOff) {
            rawState = false;
        } else {
            rawState = state.lastRawState;
        }
    }

    if (rawState != state.lastRawState) {
        state.lastStateChangeTime = millis();
    }

    if ((millis() - state.lastStateChangeTime) > state.debounceDelay) {
        if (rawState != state.capDetected) {
            state.capDetected = rawState;

            /*if (DeviceState::instance().smart.get()) {
                onHeatingTrigger(state.capDetected);
            }*/
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
