#pragma once

#include "Observable.h"
#include "Config.h"

struct HeaterState {
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    
    Observable<bool> isHeating{false};
    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};
    Observable<uint16_t> tempLimit{0};
    Observable<uint16_t> temp{0};
    Observable<uint16_t> tempK{0};
    Observable<uint16_t> tempIR{0};
    Observable<uint16_t> tempCorrection{0};

    static HeaterState& instance();

private:
    HeaterState() = default;
};

HeaterState& hs();