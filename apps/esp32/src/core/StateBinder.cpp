#include "core/StateBinder.h"
#include "core/DeviceState.h"
#include "heater/HeaterState.h"
#include "heater/HeaterCycle.h"
#include "heater/Sensors.h"
#include "heater/Presets.h"
#include "DisplayDriver.h"
#include "driver/Audio.h"

template <typename T>
void bind(Observable<T>& observable, std::function<void(const T&)> setter) {
    setter(observable.get());
    observable.addListener([&setter](T v) {
        setter(v);
    });
}

void StateBinder::bindDisplay(DisplayDriver* display) {
    auto& ds = DeviceState::instance();
    
    bind<uint8_t>(ds.display.brightness, [display](uint8_t val) {
        display->setBrightness(val);
    });


    //display->setDarkMode(ds.darkMode.get());
    ds.display.darkMode.addListener([display](bool enabled) {
        //display->setDarkMode(enabled);
    });

    ds.display.flipOrientation.addListener([display](bool enabled) {
        display->setOrientation(enabled ? 1 : 3);
    });
}


void StateBinder::bindHeater(HeaterController* heater) {
    auto& state = DeviceState::instance();
    auto& hs = HeaterState::instance();

    bind<uint8_t>(state.audio.volume, [](uint8_t val) {
        Audio::setVolume(val);
    });

    bind<uint8_t>(hs.power, [heater](uint8_t val) {
        heater->setPower(val);
    });

    bind<uint32_t>(state.autoStopTime, [heater](uint32_t time) {
        heater->setAutoStopTime(time);
    });
    
    hs.cycle.addListener([&hs](uint16_t val) {
        if (hs.mode == HeaterMode::PRESET) {
            uint8_t preset = val == 1 ? hs.cycle1preset : hs.cycle2preset;
            hs.currentPreset.set(preset);
            hs.tempLimit.set(Presets::getPresetTemp(preset));
        } else hs.tempLimit.set(val == 1 ? hs.tempLimitCycle1 : hs.tempLimitCycle2);
    });

    if (hs.mode == HeaterMode::PRESET) hs.tempLimit.set(Presets::getPresetTemp(hs.currentPreset));
    hs.tempLimit.addListener([&hs](uint16_t val) {
        if (hs.mode == HeaterMode::PRESET) return;
        if (val == HeaterConfig::MAX_TEMPERATURE) return;   
        if(HeaterCycle::is(1)) hs.tempLimitCycle1.set(val);
        else hs.tempLimitCycle2.set(val);
    });

    /*heater->getIRTempSensor()->setEmissivity(hs.irEmissivity  / 100.0f);
    hs.irEmissivity.addListener([heater](uint8_t val) {
        heater->getIRTempSensor()->setEmissivity(val / 100.0f);
    });*/

    
    bind<int8_t>(hs.ambientCorrection, [heater](int8_t val) {
        if (val == 0) heater->getIRTempSensor()->enableAmbientCorrection(false, 0.0f);
        else heater->getIRTempSensor()->enableAmbientCorrection(true, val / 100.0f);
    });
    

    bind<uint32_t>(hs.tempSensorReadInterval, [heater](uint32_t time) {
        heater->getTempSensor(Sensors::Type::K)->setReadInterval(time);
    });

    bind<uint32_t>(hs.tempSensorOffTime, [heater](uint32_t time) {
        heater->getZVSDriver()->setSensorOffTime(time);
    });

    bind<uint32_t>(hs.zvsDutyCyclePeriodMs, [heater](uint32_t val) {
        heater->getZVSDriver()->setPeriod(val);
    });
    
    hs.currentPreset.addListener([&hs](uint8_t val) {
        if (hs.mode != HeaterMode::PRESET) return;
        hs.tempLimit.set(Presets::getPresetTemp(val));            
    });
}

void StateBinder::bindDebug(DeviceUI* ui) {
    auto& state = DeviceState::instance();

    state.debug.input.addListener([ui] (bool val) {
        ui->getInputHandler()->setDebug(val);
    });
}

void StateBinder::bindAll(DeviceUI* ui, HeaterController* heater) {
    bindDisplay(ui->getDisplay());
    bindHeater(heater);
    bindDebug(ui);
}
