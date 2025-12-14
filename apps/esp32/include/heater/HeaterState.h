#pragma once

#include "Observable.h"
#include "Config.h"

struct HeaterState {
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<uint32_t> cycleTimeout{"heater", "cycleTimeout", HeaterConfig::CYCLE_TIMEOUT_MS};
    PersistedObservable<uint8_t> cycle{"heater", "cycle", 1};
    
    Observable<bool> zvsOn{false};
    Observable<bool> isHeating{false};

    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};

    Observable<uint16_t> temp{0};
    Observable<uint16_t> tempK{0};
    Observable<uint16_t> tempIR{0};
    Observable<uint16_t> tempLimit{0};

    PersistedObservable<int8_t> tempCorrection{"heater", "tempCorrection", 0};
    PersistedObservable<uint8_t> irEmissivity{"heater", "irEmissivity", 95};
    PersistedObservable<uint32_t> zvsDutyCyclePeriodMs{"zvs", "dutyCyclePeriodMs", HeaterConfig::DUTY_CYCLE_PERIOD_MS};
    PersistedObservable<uint32_t> tempSensorOffTime{"heater", "tempSensorOffTime", HeaterConfig::SENSOR_OFF_TIME_MS};
    PersistedObservable<uint32_t> tempSensorReadInterval{"heater", "tempSensorReadInterval", HeaterConfig::SENSOR_TEMPERATURE_READ_INTERVAL_MS};



    static HeaterState& instance();

private:
    HeaterState() = default;
};

HeaterState& hs();