#include "core/StateBinder.h"
#include "core/DeviceState.h"
#include "heater/HeaterState.h"
#include "heater/HeaterCycle.h"
#include "heater/Sensors.h"
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

    heater->setPower(hs.power);
    hs.power.addListener([heater](uint8_t val) {
        heater->setPower(val);
    });

    heater->setAutoStopTime(state.autoStopTime.get());
    state.autoStopTime.addListener([heater](uint32_t time) {
        heater->setAutoStopTime(time);
    });

    
        
        /*if (hs.mode == HeaterMode::PRESET) {
            uint8_t preset = hs.cycle == 1 ? hs.cycle1preset : hs.cycle2preset;
            hs.currentPreset.set(preset);
            
            switch (preset) {
            case 0:
                hs.tempLimit.set(hs.preset1Temp);
                break;
            case 1:
                hs.tempLimit.set(hs.preset2Temp);
                break;
            case 2:
                hs.tempLimit.set(hs.preset3Temp);
                break;
            case 3:
                hs.tempLimit.set(hs.preset4Temp);
                break;
            }
            
            return;
        } else hs.tempLimit.set(hs.cycle == 1 ? hs.tempLimitCycle1 : hs.tempLimitCycle2);*/
    hs.cycle.addListener([&hs](uint16_t val) {
        if (hs.mode == HeaterMode::PRESET) {
            uint8_t preset = val == 1 ? hs.cycle1preset : hs.cycle2preset;
            hs.currentPreset.set(preset);
            
            switch (preset) {
            case 0:
                hs.tempLimit.set(hs.preset1Temp);
                break;
            case 1:
                hs.tempLimit.set(hs.preset2Temp);
                break;
            case 2:
                hs.tempLimit.set(hs.preset3Temp);
                break;
            case 3: 
                hs.tempLimit.set(hs.preset4Temp);
                break;
            }
            
            return;
        } else hs.tempLimit.set(val == 1 ? hs.tempLimitCycle1 : hs.tempLimitCycle2);
    });

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

    heater->getIRTempSensor()->enableAmbientCorrection(hs.ambientCorrection != 0, hs.ambientCorrection / 100.0f);
    hs.ambientCorrection.addListener([heater](int8_t val) {
        if (val == 0) heater->getIRTempSensor()->enableAmbientCorrection(false, 0.0f);
        else heater->getIRTempSensor()->enableAmbientCorrection(true, val / 100.0f);
    });
    
    heater->getTempSensor(Sensors::Type::K)->setReadInterval(hs.tempSensorReadInterval);
    hs.tempSensorReadInterval.addListener([heater](uint32_t time) {
        heater->getTempSensor(Sensors::Type::K)->setReadInterval(time);
    });
    
    heater->getZVSDriver()->setSensorOffTime(hs.tempSensorOffTime);
    hs.tempSensorOffTime.addListener([heater](uint32_t time) {
        heater->getZVSDriver()->setSensorOffTime(time);
    });

    heater->getZVSDriver()->setPeriod(hs.zvsDutyCyclePeriodMs);
    hs.zvsDutyCyclePeriodMs.addListener([heater](uint32_t val) {
        heater->getZVSDriver()->setPeriod(val);
    });
    
    hs.currentPreset.addListener([&hs](uint8_t val) {
        if (hs.mode != HeaterMode::PRESET) return;
        
        switch (val) {
        case 0:
            hs.tempLimit.set(hs.preset1Temp);
            break;
        case 1:
            hs.tempLimit.set(hs.preset2Temp);
            break;
        case 2:
            hs.tempLimit.set(hs.preset3Temp);
            break;
        case 3:
            hs.tempLimit.set(hs.preset4Temp);
            break;
        }
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
