#pragma once

#include <cstdint>
#include "TempSensor.h"
#include "heater/ZVSDriver.h"

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

    // Expose components
    TempSensor* getTempSensor() { return tempSensor; }
    ZVSDriver* getZVSDriver() { return zvsDriver; }

private:
    TempSensor* tempSensor;
    ZVSDriver* zvsDriver;

    void transitionTo(State newState);

    State state = State::IDLE;
    uint8_t power = 0;
    uint32_t startTime = 0;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000;
    uint32_t cycleCounter = 0;
    uint32_t lastCycleDuration = 0;
    bool cycleFinishedFlag = false;
};


struct HeaterStoppedData {
    uint32_t duration;
    uint32_t startedAt;
};

