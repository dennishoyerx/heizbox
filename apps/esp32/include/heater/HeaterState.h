#pragma once

#include "Observable.h"
#include "Config.h"

enum HeaterMode {
    TEMP_TARGET_CUTOFF,
    TEMP_TARGET
};

struct HeaterSettings {
    PersistedObservable<uint8_t> mode{"heater", "mode", HeaterMode::TEMP_TARGET_CUTOFF};

    struct Temperature {
        PersistedObservable<uint16_t> limitCycle1{"temp", "cycle1", 210};
        PersistedObservable<uint16_t> limitCycle2{"temp", "cycle2", 225};
    };
};

struct HeaterState {
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<uint32_t> cycleTimeout{"heater", "cycletimeout", HeaterConfig::CYCLE_TIMEOUT_MS};
    PersistedObservable<uint8_t> cycle{"heater", "cycle", 1};

    PersistedObservable<int8_t> ambientCorrection{"ir", "ambcorrection", 0};
    PersistedObservable<uint8_t> irEmissivity{"ir", "emissivity", 96};
    PersistedObservable<int16_t> irCorrection{"ir", "correction", 0};

    Observable<bool> zvsOn{false};
    Observable<bool> isHeating{false};

    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};

    Observable<uint16_t> temp{0};
    Observable<uint16_t> tempK{0};
    Observable<uint16_t> tempIR{0};
    Observable<uint16_t> tempLimit{210};
    PersistedObservable<uint16_t> tempLimitCycle1{"temp", "cyclea", 210};
    PersistedObservable<uint16_t> tempLimitCycle2{"temp", "cycleb", 225};

    PersistedObservable<bool> alwaysMeasure{"heater", "alwaysmeasure", false};
    PersistedObservable<int8_t> tempCorrection{"temp", "correction", 0};
    PersistedObservable<bool> cutoffIr{"heater", "cutoffIr", true};
    PersistedObservable<uint32_t> zvsDutyCyclePeriodMs{"zvs", "dutycycleperiodms", HeaterConfig::ZVS::DUTY_CYCLE_PERIOD_MS};
    PersistedObservable<uint32_t> tempSensorOffTime{"heater", "tempSensorofftime", HeaterConfig::KSensor::OFF_TIME_MS};
    PersistedObservable<uint32_t> tempSensorReadInterval{"heater", "tempSensorreadinterval", HeaterConfig::KSensor::READ_INTERVAL_MS};

    // IR calibration (2-point)
    // gespeicherte, gemessene IR‑Werte bei zwei bekannten Referenztemperaturen
    PersistedObservable<uint16_t> irCalMeasuredA{"ir", "cal_meas_a", 0};
    PersistedObservable<uint16_t> irCalMeasuredB{"ir", "cal_meas_b", 0};
    // die zugehörigen tatsächlichen Temperaturen (z.B. 150 und 200)
    PersistedObservable<uint16_t> irCalActualA{"ir", "cal_act_a", 150};
    PersistedObservable<uint16_t> irCalActualB{"ir", "cal_act_b", 200};
    // berechnete Kalibrierungsparameter: Temp_true = slope * Temp_measured + offset
    PersistedObservable<float> irCalSlope{"ir", "cal_slope", 1.0f};
    PersistedObservable<float> irCalOffset{"ir", "cal_offset", 0.0f};

    static HeaterState& instance();

private:
    HeaterState() = default;
};