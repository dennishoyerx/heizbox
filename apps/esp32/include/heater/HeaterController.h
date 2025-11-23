#ifndef HEATERCONTROLLER_H
#define HEATERCONTROLLER_H

#include <cstdint>
#include "hardware/sensor/TempSensor.h"
#include "core/EventBus.h"

class ZVSDriver {
private:
    uint8_t zvs_pin;
    uint8_t dutyCycle;
public:
    ZVSDriver(uint8_t pin) : zvs_pin(pin) {}

    void init() {
        pinMode(zvs_pin, OUTPUT);
        digitalWrite(zvs_pin, LOW);
    }

    void enable() {
        digitalWrite(zvs_pin, HIGH);
    }

    void disable() {
        digitalWrite(zvs_pin, LOW);
    }

};

class HeaterController {
public:
    enum class State : uint8_t {
        IDLE,
        HEATING,
        PAUSED,
        ERROR
    };

    HeaterController();
    void init();
    void startHeating();
    void stopHeating(bool finalize = true);
    void setPower(uint8_t power);
    uint8_t getPower();
    void update();

    State getState() const;
    bool isHeating() const;
    bool isPaused() const;
    uint32_t getElapsedTime() const;
    uint32_t getCycleCount() const;
    uint32_t getLastCycleDuration() const;
    bool isCycleFinished() const;
    void clearCycleFinishedFlag();
    void setAutoStopTime(uint32_t time);
    uint32_t getAutoStopTime() const;
    uint16_t getTemperature();


private:
    TempSensor* tempSensor;
    EventBus eventBus;

    void transitionTo(State newState);
    void updateDutyCycle();


    State state = State::IDLE;
    uint8_t power = 0;
    uint32_t startTime = 0;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000; // Default 60 seconds
    uint32_t cycleCounter = 0;
    uint32_t lastCycleDuration = 0;
    bool cycleFinishedFlag = false;

    uint32_t dutyCycleStartTime;
    bool heaterPhysicallyOn;
};



#endif