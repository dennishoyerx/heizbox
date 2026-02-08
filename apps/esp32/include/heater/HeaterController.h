#pragma once

#include <cstdint>
#include "TempSensor.h"
#include "IRTempSensor.h"
#include "driver/ZVSDriver.h"
#include "heater/Sensors.h"
#include "heater/HeaterState.h"
#include "services/HeatCycle.h"
#include "ITemperatureSensor.h"
#include "heater/Temperature.h"

#include <BaseClass.h>

namespace Heater {

    enum class State : uint8_t {
        IDLE,
        HEATING,
        PAUSED,
        ERROR
    };

    struct Status {
        void set(State s) { state = s; }
      private:
        State state = State::IDLE;
    };

struct AutoStopTime {
    int8_t limit;
    AutoStopTime(int8_t s) : limit(s) {}
    bool hasExceeded(int8_t passedSeconds) { return passedSeconds > limit; }
};
};

struct HeaterController: public dh::BaseClass {
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
    void update();
    void updateTemperature();

    int16_t markIRClick(uint16_t actualTemp);
    void clearIRCalibration();
    float getIRCalibrationSlope() const;
    float getIRCalibrationOffset() const;
    void computeIRCalibration();

    bool isHeating() const { return state == State::HEATING; }
    bool isPaused() const { return state == State::PAUSED; }
    
    void setAutoStopTime(uint32_t time);
    uint32_t getAutoStopTime() const;
    
    // Expose components
    ITemperatureSensor* getTempSensor(Sensors::Type sensor = Sensors::Type::K) { return temperature.getSensor(sensor); }
    TempSensor* getKTempSensor() { return temperature.getKSensor(); }
    IRTempSensor* getIRTempSensor() { return temperature.getIRSensor(); }
    ZVSDriver* getZVSDriver() { return zvsDriver; }

private:
    Temperature::Controller _temperature;
    Sensors temperature;
    ZVSDriver* zvsDriver;
    HeatCycle heatCycle;

    void transitionTo(State newState);

    State state = State::IDLE;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000;
};