#include "core/StateBinder.h"
#include "core/DeviceState.h"
#include "DisplayDriver.h"

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
    //display->setDarkMode(state.darkMode.get());

    // Listen to state changes
    state.darkMode.addListener([display](bool enabled) {
        //display->setDarkMode(enabled);
    });
}



void StateBinder::bindHeater(HeaterController* heater) {
    auto& state = DeviceState::instance();

    heater->setPower(state.power.get());
    state.power.addListener([heater](uint8_t val) {
        heater->setPower(val);
    });

    heater->setAutoStopTime(state.autoStopTime.get());
    state.autoStopTime.addListener([heater](uint32_t time) {
        heater->setAutoStopTime(time);
    });

    
    heater->getTempSensor()->setReadInterval(state.tempSensorOffTime.get());
    heater->getZVSDriver()->setSensorOffTime(state.tempSensorOffTime.get());
    state.tempSensorOffTime.addListener([heater](uint32_t time) {
        heater->getTempSensor()->setReadInterval(time);
        heater->getZVSDriver()->setSensorOffTime(time);
    });

    heater->getZVSDriver()->setPeriod(state.zvsDutyCyclePeriodMs.get());
    state.zvsDutyCyclePeriodMs.addListener([heater](uint32_t val) {
        heater->getZVSDriver()->setPeriod(val);
    });
}

void StateBinder::bindAll(DisplayDriver* display, HeaterController* heater) {
    bindBrightness(display);
    bindDarkMode(display);
    bindHeater(heater);
}
