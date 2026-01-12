#pragma once

#include <Arduino.h>
#include "heater/Sensors.h"

namespace Temperature {
    struct State {
        uint16_t current;
        uint16_t limit;
        uint16_t ambient;
        float raw;
    };

    class Controller {
    public:
        Controller();
        void init();
        void update();
        bool limitReached();

    private:
        State state;
        Sensors sensors;
        uint16_t cutoffTemp;
    };
};