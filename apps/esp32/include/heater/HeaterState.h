#pragma once

#include "Observable.h"
#include "Config.h"

struct HeaterState {
    Observable<bool> isHeating{false};
    Observable<uint32_t> timer{0};
    Observable<uint32_t> startTime{0};
    Observable<uint16_t> temp{0};
    Observable<uint16_t> tempK{0};
    Observable<uint16_t> tempIR{0};

    static HeaterState& instance();

private:
    HeaterState() = default;
};

HeaterState& hs();