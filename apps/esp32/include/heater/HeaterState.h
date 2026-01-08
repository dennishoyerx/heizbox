#pragma once

#include "Observable.h"
#include "Config.h"

enum HeaterMode {
    TEMP_TARGET_CUTOFF,
    TEMP_TARGET
};
/*
struct HeaterSettings {
    PersistedObservable<uint8_t> mode{"heater", "mode", HeaterMode::TEMP_TARGET_CUTOFF};

    struct Temperature {
        PersistedObservable<uint16_t> limitCycle1{"temp", "cycle1", 210};
        PersistedObservable<uint16_t> limitCycle2{"temp", "cycle2", 225};
    };
};*/

struct HeaterState {
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<uint32_t> cycleTimeout{"heater", "cycleTimeout", HeaterConfig::CYCLE_TIMEOUT_MS};
    PersistedObservable<uint8_t> cycle{"heater", "cycle", 1};
    PersistedObservable<uint8_t> ambientCorrection{"heater", "ambcorrection", 15};


    Observable<bool> zvsOn{false};
    Observable<bool> isHeating{false};

    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};

    Observable<uint16_t> temp{0};
    Observable<uint16_t> tempK{0};
    Observable<uint16_t> tempIR{0};
    Observable<uint16_t> tempLimit{210};
    PersistedObservable<uint16_t> tempLimitCycle1{"temp", "cycle1", 210};
    PersistedObservable<uint16_t> tempLimitCycle2{"temp", "cycle2", 225};

    PersistedObservable<bool> alwaysMeasure{"heater", "alwaysMeasure", false};
    PersistedObservable<int8_t> tempCorrection{"heater", "tempCorrection", 0};
    PersistedObservable<uint8_t> irEmissivity{"heater", "irEmissivity", 95};
    PersistedObservable<uint16_t> irCorrection{"heater", "irCorrection", 37};
    PersistedObservable<bool> cutoffIr{"heater", "cutoffIr", true};
    PersistedObservable<uint32_t> zvsDutyCyclePeriodMs{"zvs", "dutyCyclePeriodMs", HeaterConfig::DUTY_CYCLE_PERIOD_MS};
    PersistedObservable<uint32_t> tempSensorOffTime{"heater", "tempSensorOffTime", HeaterConfig::SENSOR_OFF_TIME_MS};
    PersistedObservable<uint32_t> tempSensorReadInterval{"heater", "tempSensorReadInterval", HeaterConfig::SENSOR_TEMPERATURE_READ_INTERVAL_MS};



    static HeaterState& instance();

private:
    HeaterState() = default;
};
