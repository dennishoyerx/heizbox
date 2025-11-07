// src/core/StateBinder.cpp
#include "core/StateBinder.h"
#include "core/DeviceState.h"

// ============================================================================
// State Bindings - UI ↔ State Synchronisation
// ============================================================================

void StateBinder::bindBrightness(DisplayDriver* display) {
    auto& state = DeviceState::instance();

    // Initial sync
    display->setBrightness(state.brightness.get());

    // Listen to state changes
    state.brightness.addListener([display](uint8_t value) {
        display->setBrightness(value);
    });
}

void StateBinder::bindDarkMode(DisplayDriver* display) {
    auto& state = DeviceState::instance();

    // Initial sync
    display->setDarkMode(state.darkMode.get());

    // Listen to state changes
    state.darkMode.addListener([display](bool enabled) {
        display->setDarkMode(enabled);
    });
}



void StateBinder::bindAutoStopTime(HeaterController* heater) {
    auto& state = DeviceState::instance();

    heater->setAutoStopTime(state.autoStopTime.get());

    state.autoStopTime.addListener([heater](uint32_t time) {
        heater->setAutoStopTime(time);
    });
}

void StateBinder::bindAll(DisplayDriver* display, HeaterController* heater) {
    bindBrightness(display);
    bindDarkMode(display);
    bindAutoStopTime(heater);
}
