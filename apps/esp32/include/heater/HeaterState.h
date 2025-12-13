#pragma once

#include "Observable.h"
#include "Config.h"

enum Test {
    TEST1,
    TEST2,
    TEST3
};


struct HeaterState {
    Observable<Test> xx{TEST1};
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<uint32_t> cycleTimeout{"heater", "cycleTimeout", HeaterConfig::CYCLE_TIMEOUT_MS};
    
    Observable<bool> zvsOn{false};
    Observable<bool> isHeating{false};

    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};

    Observable<uint16_t> temp{0};
    Observable<uint16_t> tempK{0};
    Observable<uint16_t> tempIR{0};
    Observable<uint16_t> tempLimit{0};
    Observable<int8_t> tempCorrection{0};

    static HeaterState& instance();

private:
    HeaterState() = default;
};

HeaterState& hs();