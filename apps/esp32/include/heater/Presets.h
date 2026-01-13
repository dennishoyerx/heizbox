#pragma once

#include <Arduino.h>
#include "heater/HeaterState.h"

struct TempPreset {
    uint8_t temp;
    String name;
};

const TempPreset TempPresets[] = {
    { 170, "FLAVOR" },
    { 180, "BALANCED" },
    { 200, "EXTRACTION" },
    { 220, "FULL" },
};

namespace Presets {
    class Manager {};

    static uint8_t getPreset(uint8_t preset) { return HeaterState::instance().currentPreset; }
    static void setPreset(uint8_t preset) { HeaterState::instance().currentPreset.set(preset); }
    static void setPresetTemp(uint8_t preset, uint16_t temp) { 
        switch (preset) {
        case 0: HeaterState::instance().preset1Temp.set(temp); break;
        case 1: HeaterState::instance().preset2Temp.set(temp); break;
        case 2: HeaterState::instance().preset3Temp.set(temp); break;
        case 3: HeaterState::instance().preset4Temp.set(temp); break;
        }
    }

    static uint16_t getPresetTemp(uint8_t preset) {
        switch (preset) {
            case 0: return HeaterState::instance().preset1Temp;
            case 1: return HeaterState::instance().preset2Temp;
            case 2: return HeaterState::instance().preset3Temp;
            case 3: return HeaterState::instance().preset4Temp;
            default: return 0;
        }
    }


};