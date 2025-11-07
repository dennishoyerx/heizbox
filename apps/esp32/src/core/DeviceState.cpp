// src/core/DeviceState.cpp
#include "core/DeviceState.h"
#include <nvs_flash.h>

DeviceState& DeviceState::instance() {
    static bool nvsReady = false;
    if (!nvsReady) {
        nvs_flash_init();
        nvsReady = true;
    }
    static DeviceState state;
    return state;
}
