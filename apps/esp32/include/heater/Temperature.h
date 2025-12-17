#pragma once

#include "Arduino.h"

class TemperatureControl {
    TemperatureControl();

private:
    uint16_t cutoffTemp;
};