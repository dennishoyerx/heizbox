#include "heater/HeaterController.h"
#include "core/Config.h"
#include <Arduino.h>

HeaterController::HeaterController()
    : state(State::IDLE), 
      power(100),
      startTime(0), 
      pauseTime(0),
      autoStopTime(60000), 
      lastCycleDuration(0), 
      cycleFinishedFlag(false)
{
    // Initialize temperature sensor
    tempSensor = new TempSensor(
        HardwareConfig::THERMO_SCK_PIN, 
        HardwareConfig::THERMO_CS_PIN, 
        HardwareConfig::THERMO_SO_PIN, 
        HeaterConfig::SENSOR_TEMPERATURE_READ_INTERVAL_MS
    );
    
    // Initialize ZVS driver
    zvsDriver = new ZVSDriver(
        HardwareConfig::HEATER_MOSFET_PIN,
        HardwareConfig::STATUS_LED_PIN
    );
}

void HeaterController::init() {
    // Initialize ZVS driver
    zvsDriver->init();
    zvsDriver->setPeriod(HeaterConfig::DUTY_CYCLE_PERIOD_MS);
    zvsDriver->setSensorOffTime(HeaterConfig::SENSOR_OFF_TIME_MS);
    zvsDriver->setPower(power);
    
    // Register temperature measurement callback
    zvsDriver->onTempMeasure([this]() {
        // This is called during the OFF phase - safe to measure temperature
        tempSensor->update(true); // Force immediate read
    });
    
    // Initialize temperature sensor
    if (!tempSensor->begin()) {
        Serial.println("⚠️  Temperature sensor initialization failed");
    }
    
    Serial.println("🔥 Heater initialized with ZVS driver");
}

void HeaterController::setPower(uint8_t _power) {
    if (_power > 100) _power = 100;
    if (_power < 10) _power = 10;
    power = _power;
    
    // Update ZVS driver
    zvsDriver->setPower(power);
}

uint8_t HeaterController::getPower() {
    return power;
}

void HeaterController::transitionTo(State newState) {
    if (state == newState) return;

    Serial.printf("🔥 State: %d -> %d\n", static_cast<int>(state), static_cast<int>(newState));
    state = newState;
}

uint16_t HeaterController::getTemperature() {
    return (uint16_t)tempSensor->getTemperature();
}

void HeaterController::startHeating() {
    if (state == State::IDLE) {
        startTime = millis();
        
        // Enable ZVS driver
        zvsDriver->setEnabled(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");
        
    } else if (state == State::PAUSED) {
        // Adjust startTime to account for the pause duration
        startTime = millis() - (pauseTime - startTime);
        
        // Enable ZVS driver
        zvsDriver->setEnabled(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating resumed");
    }
}

void HeaterController::stopHeating(bool finalize) {
    if (state != State::HEATING) return;

    // Disable ZVS driver
    zvsDriver->setEnabled(false);

    if (finalize) {
        const uint32_t duration = millis() - startTime;
        lastCycleDuration = duration;

        // Only count cycles longer than a minimum threshold
        if (duration >= HeaterConfig::HEATCYCLE_MIN_DURATION_MS) {
            cycleCounter++;
            cycleFinishedFlag = true;
        }

        startTime = millis();
        transitionTo(State::IDLE);
        Serial.println("🔥 Heating stopped (finalized)");
    } else {
        pauseTime = millis();
        transitionTo(State::PAUSED);
        Serial.println("🔥 Heating paused");
    }
}

void HeaterController::update() {
    // Update ZVS driver (handles duty cycle automatically)
    zvsDriver->update();
    
    // Update temperature sensor (additional updates outside duty cycle)
    if (state != State::HEATING) {
        tempSensor->update();
    }
    
    const uint32_t elapsed = getElapsedTime();

    switch (state) {
        case State::HEATING:
            if (elapsed >= autoStopTime) {
                Serial.println("Auto-stop triggered.");
                stopHeating();
            }
            break;

        case State::PAUSED:
            if (millis() - pauseTime >= HeaterConfig::PAUSE_TIMEOUT_MS) {
                Serial.println("Pause timeout, finalizing cycle.");
                const uint32_t duration = pauseTime - startTime;
                lastCycleDuration = duration;

                if (duration >= HeaterConfig::HEATCYCLE_MIN_DURATION_MS) {
                    cycleCounter++;
                    cycleFinishedFlag = true;
                }
                startTime = millis();
                transitionTo(State::IDLE);
            }
            break;

        case State::IDLE:
        case State::ERROR:
            // No automatic transitions from these states
            break;
    }
}

HeaterController::State HeaterController::getState() const {
    return state;
}

bool HeaterController::isHeating() const {
    return state == State::HEATING;
}

bool HeaterController::isPaused() const {
    return state == State::PAUSED;
}

uint32_t HeaterController::getElapsedTime() const {
    if (state == State::HEATING) {
        return millis() - startTime;
    }
    if (state == State::PAUSED) {
        return pauseTime - startTime;
    }
    return 0;
}

uint32_t HeaterController::getCycleCount() const {
    return cycleCounter;
}

void HeaterController::setAutoStopTime(uint32_t time) {
    autoStopTime = time;
}

uint32_t HeaterController::getAutoStopTime() const {
    return autoStopTime;
}

uint32_t HeaterController::getLastCycleDuration() const {
    return lastCycleDuration;
}

bool HeaterController::isCycleFinished() const {
    return cycleFinishedFlag;
}

void HeaterController::clearCycleFinishedFlag() {
    cycleFinishedFlag = false;
}