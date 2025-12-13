#pragma once

#include <cstdint>
#include "TempSensor.h"
#include "IRTempSensor.h"
#include "heater/ZVSDriver.h"
#include "heater/HeaterTemperature.h"
#include "heater/HeaterState.h"

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
    void updateTemperature();

    State getState() const;
    bool isHeating() const;
    bool isPaused() const;
    uint32_t getElapsedTime() const;
    void setAutoStopTime(uint32_t time);
    uint32_t getAutoStopTime() const;
    uint16_t getTemperature();
    uint16_t getIRTemperature();

    // Expose components
    TempSensor* getTempSensor() { return temperature.getKSensor(); }
    IRTempSensor* getIRTempSensor() { return temperature.getIRSensor(); }
    ZVSDriver* getZVSDriver() { return zvsDriver; }

private:
    HeaterTemperature temperature;
    ZVSDriver* zvsDriver;

    void transitionTo(State newState);

    State state = State::IDLE;
    uint8_t power = 0;
    uint32_t startTime = 0;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000;
};


struct HeaterStoppedData {
    uint32_t duration;
    uint32_t startedAt;
};

