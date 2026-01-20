#pragma once

#include "Observable.h"
#include "Config.h"

namespace Heater {
    enum Mode {
        TEMP,
        PRESET
    };

    namespace Preset {
        struct Settings {
            PersistedObservable<uint8_t> temp1{"preset", "1_temp", 185};
            PersistedObservable<uint8_t> temp2{"preset", "2_temp", 200};
            PersistedObservable<uint8_t> temp3{"preset", "3_temp", 210};
            PersistedObservable<uint8_t> temp4{"preset", "4_temp", 220};
            PersistedObservable<uint8_t> cycle1preset{"preset", "cycle_1", 1};
            PersistedObservable<uint8_t> cycle2preset{"preset", "cycle_2", 2};
        };

        struct State: public Settings {
            Observable<uint8_t> current{0};
        };
    }

    namespace IRSensor {
        struct CalibrationSettings {
            PersistedObservable<uint16_t> measuredA{"ir", "cal_meas_a", 0};
            PersistedObservable<uint16_t> measuredB{"ir", "cal_meas_b", 0};
            PersistedObservable<uint16_t> actualA{"ir", "cal_act_a", 150};
            PersistedObservable<uint16_t> actualB{"ir", "cal_act_b", 200};
            PersistedObservable<float> slope{"ir", "cal_slope", 1.0f};
            PersistedObservable<float> offset{"ir", "cal_offset", 0.0f};
        };

        struct Settings {
            CalibrationSettings calibration;

            PersistedObservable<int8_t> ambientCorrection{"ir", "ambcorrection", 0};
            PersistedObservable<uint8_t> emissivity{"ir", "emissivity", 97};
            PersistedObservable<int16_t> correction{"ir", "correction", 0};
        };

        struct State: public Settings {
            Observable<uint16_t> temp{0};
            Observable<uint16_t> tempRaw{0};
            Observable<uint16_t> tempAmb{0};
        };

    }

    namespace Temperature {
    struct Settings {
        PersistedObservable<uint16_t> limitCycle1{"temp", "cyclea", 210};
        PersistedObservable<uint16_t> limitCycle2{"temp", "cycleb", 225};
    };

    struct State: public Settings {
        Observable<uint16_t> limit{210};
        Observable<uint16_t> temp{0};
        Observable<uint16_t> ktype{0};
        Observable<uint16_t> ir{0};
        Observable<uint16_t> irRaw{0};
        Observable<uint16_t> irAmb{0};
    };}

struct Settings {
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<uint8_t> mode{"heater", "mode", Mode::PRESET};

    PersistedObservable<uint32_t> cycleTimeout{"heater", "cycletimeout", HeaterConfig::CYCLE_TIMEOUT_MS};
    PersistedObservable<uint8_t> cycle{"heater", "cycle", 1};

    PersistedObservable<bool> alwaysMeasure{"heater", "alwaysmeasure", false};
    PersistedObservable<int8_t> tempCorrection{"temp", "correction", 0};
    PersistedObservable<uint32_t> zvsDutyCyclePeriodMs{"zvs", "dutycycleperiodms", HeaterConfig::ZVS::DUTY_CYCLE_PERIOD_MS};
    PersistedObservable<uint32_t> tempSensorOffTime{"heater", "tempSensorofftime", HeaterConfig::KSensor::OFF_TIME_MS};
    PersistedObservable<uint32_t> tempSensorReadInterval{"heater", "tempSensorreadinterval", HeaterConfig::KSensor::READ_INTERVAL_MS};
};


struct State: public Settings {
    Temperature::State temp;
    IRSensor::State ir;
    Preset::State preset;

    Observable<bool> zvsOn{false};
    Observable<bool> isHeating{false};
    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};

    static State& instance();
private:
    State() = default;
    void x() {
    }
};
}
