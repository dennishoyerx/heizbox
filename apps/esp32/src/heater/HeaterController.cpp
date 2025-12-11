#include "heater/HeaterController.h"
#include "Config.h"
#include <Arduino.h>
#include "utils/Logger.h"
#include "core/EventBus.h"

HeaterController::HeaterController()
    : state(State::IDLE), 
      power(100),
      startTime(0), 
      pauseTime(0),
      autoStopTime(60000), 
      lastCycleDuration(0), 
      cycleFinishedFlag(false)
{
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
        temperature.update(K, true);
    });
    
    temperature.init();
    
    Serial.println("🔥 Heater initialized with ZVS driver");
}

void HeaterController::setPower(uint8_t _power) {
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
    return temperature.get(K);
}


uint16_t HeaterController::getIRTemperature() {
    return temperature.get(IR);
}

void HeaterController::startHeating() {
    if (state == State::IDLE) {
        startTime = millis();
        
        zvsDriver->setEnabled(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");

        hs().isHeating.set(true);
        hs().startTime.set(startTime);
        EventBus::instance().publish(EventType::HEATER_STARTED, nullptr);
    } else if (state == State::PAUSED) {
        startTime = millis() - (pauseTime - startTime);

        zvsDriver->setEnabled(true);
        hs().isHeating.set(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating resumed");
    }
}

void HeaterController::stopHeating(bool finalize) {
    if (state != State::HEATING) return;

    zvsDriver->setEnabled(false);
    hs().isHeating.set(false);

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
        hs().startTime.set(0);
        
        EventBus::instance().publish<HeaterStoppedData>(
            EventType::HEATER_STOPPED, {duration, startTime}
        );
    } else {
        pauseTime = millis();
        transitionTo(State::PAUSED);
        Serial.println("🔥 Heating paused");
    }
}

void HeaterController::update() {
    updateTemperature();
    zvsDriver->update();
    
    const uint32_t elapsed = getElapsedTime();
    hs().timer.set(elapsed / 1000);

    switch (state) {
        case State::HEATING:
            if (elapsed >= autoStopTime) {
                Serial.println("Auto-stop triggered.");
                stopHeating();
            }
            break;

        case State::PAUSED:
            if (millis() - pauseTime >= HeaterConfig::HEATCYCLE_MIN_DURATION_MS) {
                Serial.println("Pause timeout, finalizing cycle.");
                const uint32_t duration = pauseTime - startTime;
                lastCycleDuration = duration;
                cycleCounter++;
                cycleFinishedFlag = true;
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

void HeaterController::updateTemperature() {
    if (temperature.update(IR)) hs().tempIR.set(temperature.get(IR));
    if (temperature.update(K)) hs().tempK.set(temperature.get(K));
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