#pragma once

#include "heater/Temperature.h"
#include "heater/HeaterState.h"

Temperature::Controller::Controller() {};

void Temperature::Controller::init() {
    sensors.init();
}

void Temperature::Controller::update() {
    sensors.update();
    state.current = sensors.get(Sensors::Type::IR);
    auto& hs = HeaterState::instance();

    if (sensors.update(Sensors::Type::IR)) {
        float raw = sensors.get(Sensors::Type::IR);
        if (!isfinite(raw) || raw < 0.0f || raw > 1000.0f) return;

        raw = static_cast<uint16_t>(raw + 0.5f);

        // apply ambient correction if enabled in the sensor / state (existing behavior)
        float factor = 1.0f + (hs.irCorrection / 100.0f);
        float adjusted = raw * factor;

        // apply two-point calibration if present
        float slope = hs.irCalSlope;
        float offset = hs.irCalOffset;
        float calibrated = adjusted * slope + offset;

        state.raw = raw;
        state.current = static_cast<uint16_t>(calibrated + 0.5f);
    }
}

bool Temperature::Controller::limitReached() {
    return state.current >= state.limit;
}