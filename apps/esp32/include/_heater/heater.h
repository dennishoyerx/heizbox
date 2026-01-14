#pragma once

#include "heater/ZVSDriver.h"
#include "heater/Sensors.h"

namespace Heater {
    class OvenTemperature {};

    class Oven {
    public:
        Oven(ZVSDriver* zvs);
        void on();
        void off();

    private:
        ZVSDriver* zvs;
        OvenTemperature temperature;

    };

    class Controller {
    public:
        Controller();
        void init();
        void update();

    private:
        Oven* oven;
        Sensors sensors;
        Timer timer;
    };

    class Timer {
    public:
        Timer();
        void start();
        void stop();
        void pause();
        void resume();
        void reset();
        unsigned long getElapsedTime();
        bool isRunning();

    private:
        unsigned long startTime;
        unsigned long pauseTime;
        bool running;
        bool paused;
    };
};