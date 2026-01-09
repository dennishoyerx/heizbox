#pragma once

//#include "forward.h"

#include <cstdint>
#include "TempSensor.h" // CONVERTED: Use forward.h
#include "IRTempSensor.h" // CONVERTED: Use forward.h
#include "heater/ZVSDriver.h" // CONVERTED: Use forward.h
#include "heater/Sensors.h"
#include "heater/HeaterState.h"
#include "app/HeatCycle.h"
#include "ITemperatureSensor.h"

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

    // Mark a click event for IR calibration.
    // Pass the actual (known) temperature of the click, e.g. 150 or 200.
    // This will store the current IR measurement and recompute the 2‑point calibration if possible.
    int16_t markIRClick(uint16_t actualTemp);

    // Reset calibration to default (slope=1, offset=0)
    void clearIRCalibration();

    float getIRCalibrationSlope() const;
    float getIRCalibrationOffset() const;

    State getState() const;
    bool isHeating() const;
    bool isPaused() const;
    uint32_t getElapsedTime() const;
    void setAutoStopTime(uint32_t time);
    uint32_t getAutoStopTime() const;

    // Expose components
    ITemperatureSensor* getTempSensor(Sensors::Sensor sensor = Sensors::Sensor::K) { return temperature.getSensor(sensor); }
    TempSensor* getKTempSensor() { return temperature.getKSensor(); }
    IRTempSensor* getIRTempSensor() { return temperature.getIRSensor(); }
    ZVSDriver* getZVSDriver() { return zvsDriver; }

private:
    Sensors temperature;
    ZVSDriver* zvsDriver;

    HeatCycle heatCycle;

    void transitionTo(State newState);

    // compute calibration once two points exist
    void computeIRCalibration();

    State state = State::IDLE;
    uint8_t power = 0;
    uint32_t startTime = 0;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000;
};