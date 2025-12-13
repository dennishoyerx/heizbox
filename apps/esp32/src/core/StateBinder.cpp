#include "core/StateBinder.h"
#include "core/DeviceState.h"
#include "heater/HeaterState.h"
#include "heater/HeaterCycle.h"
#include "DisplayDriver.h"

void StateBinder::bindDisplay(DisplayDriver* display) {
    auto& ds = DeviceState::instance();

    display->setBrightness(ds.brightness.get());
    ds.brightness.addListener([display](uint8_t value) {
        display->setBrightness(value);
    });

    //display->setDarkMode(ds.darkMode.get());
    ds.darkMode.addListener([display](bool enabled) {
        //display->setDarkMode(enabled);
    });
}

void StateBinder::bindHeater(HeaterController* heater) {
    auto& state = DeviceState::instance();
    auto& hs = HeaterState::instance();

    heater->setPower(state.power.get());
    state.power.addListener([heater](uint8_t val) {
        heater->setPower(val);
    });

    heater->setAutoStopTime(state.autoStopTime.get());
    state.autoStopTime.addListener([heater](uint32_t time) {
        heater->setAutoStopTime(time);
    });

    hs.tempLimit.set(state.currentCycle == 1 ? state.targetTemperatureCycle1 : state.targetTemperatureCycle2);
    state.currentCycle.addListener([&hs, &state](uint8_t val) {
        hs.tempLimit.set(val == 1 ? state.targetTemperatureCycle1 : state.targetTemperatureCycle2);
    });

    state.targetTemperatureCycle1.addListener([&hs](uint16_t val) {
        if(HeaterCycle::is(1)) hs.tempLimit.set(val);
    });

    state.targetTemperatureCycle2.addListener([&hs](uint16_t val) {
        if(HeaterCycle::is(2)) hs.tempLimit.set(val);
    });

    heater->getIRTempSensor()->setEmissivity(hs.irEmissivity  / 100.0f);
    hs.irEmissivity.addListener([heater](uint8_t val) {
        heater->getIRTempSensor()->setEmissivity(val / 100.0f);
    });
    
    heater->getTempSensor()->setReadInterval(hs.tempSensorReadInterval.get());
    hs.tempSensorReadInterval.addListener([heater](uint32_t time) {
        heater->getTempSensor()->setReadInterval(time);
    });
    
    heater->getZVSDriver()->setSensorOffTime(hs.tempSensorOffTime.get());
    hs.tempSensorOffTime.addListener([heater](uint32_t time) {
        heater->getZVSDriver()->setSensorOffTime(time);
    });

    heater->getZVSDriver()->setPeriod(hs.zvsDutyCyclePeriodMs.get());
    hs.zvsDutyCyclePeriodMs.addListener([heater](uint32_t val) {
        heater->getZVSDriver()->setPeriod(val);
    });
}
void StateBinder::bindDebug(DeviceUI* ui) {
    auto& state = DeviceState::instance();

    state.debugInput.addListener([ui] (bool val) {
        ui->getInputHandler()->setDebug(val);
    });
}

void StateBinder::bindAll(DeviceUI* ui, HeaterController* heater) {
    bindDisplay(ui->getDisplay());
    bindHeater(heater);
    bindDebug(ui);
}
