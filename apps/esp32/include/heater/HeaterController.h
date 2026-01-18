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
#include "heater/Temperature.h"
#include "heater/HeatMode.h"

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

    int16_t markIRClick(uint16_t actualTemp);
    void clearIRCalibration();
    float getIRCalibrationSlope() const;
    float getIRCalibrationOffset() const;
    void computeIRCalibration();

    State getState() const;
    bool isHeating() const;
    bool isPaused() const;
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
    //HeatMode* mode;

    HeatCycle heatCycle;

    void transitionTo(State newState);


    State state = State::IDLE;
    uint8_t power = 0;
    uint32_t startTime = 0;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000;
};