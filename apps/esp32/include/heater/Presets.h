#pragma once

#include <Arduino.h>

struct TempPreset {
    uint8_t temp;
    String name;
};

TempPreset TempPresets[] = {
    { 170, "FLAVOR" },
    { 180, "BALANCED" },
    { 200, "EXTRACTION" },
    { 220, "FULL" },
};
