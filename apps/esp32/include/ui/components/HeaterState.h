#pragma once

#include "Observable.h"
#include "Config.h"

struct HeaterState {
    // Heater Settings
    PersistedObservable<uint32_t> autoStopTime{"heater", "autostop", 90000};
    PersistedObservable<uint8_t> currentCycle{"heater", "cycle", 1};
    PersistedObservable<bool> smart{"heater", "smart", false};
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<uint8_t> targetTemperature{"heater", "temperature", 170};
    PersistedObservable<uint8_t> targetTemperatureCycle1{"heater", "temperatureCycle1", 200};
    PersistedObservable<uint8_t> targetTemperatureCycle2{"heater", "temperatureCycle2", 220};
    PersistedObservable<uint8_t> heatCycleTempDelta{"heater", "heatCycleTempDelta", 15};
    PersistedObservable<uint8_t> temperatureOffset{"heater", "temperatureOffset", 0};

    // Runtime State (nicht persistiert)
    Observable<bool> isHeating{false};
    Observable<bool> isOnline{false};
    
    Observable<uint32_t> heatingStartTime{0};
    Observable<uint16_t> temp{0};
    Observable<uint16_t> irTemp{0};
    Observable<uint16_t> thermoTemp{0};
    Observable<uint32_t> elapsedSeconds{0};
    Observable<float> progress{0};


    PersistedObservable<bool> zvsDebug{"zvs", "debug", true};
    PersistedObservable<uint32_t> zvsDutyCyclePeriodMs{"zvs", "dutyCyclePeriodMs", HeaterConfig::DUTY_CYCLE_PERIOD_MS};
    PersistedObservable<uint32_t> tempSensorOffTime{"heater", "tempSensorOffTime", HeaterConfig::SENSOR_OFF_TIME_MS};

    static HeaterState& instance();

private:
    HeaterState() = default;
};

