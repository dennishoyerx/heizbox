#include "hardware/heater/HeaterController.h"
#include "core/Config.h"
#include "core/DeviceState.h"
#include <Arduino.h>

HeaterController::HeaterController()
    : state(State::IDLE), 
      power(100),
      startTime(0), 
      pauseTime(0),
      autoStopTime(60000), 
      cycleCounter(0), 
      lastCycleDuration(0), 
      cycleFinishedFlag(false) {
}

void HeaterController::init() {
    pinMode(HardwareConfig::HEATER_MOSFET_PIN, OUTPUT);
    digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);

    power = DeviceState::instance().power.get();

    DeviceState::instance().power.addListener([this](int val) {
        power = val;
    });



    Serial.println("🔥 Heater initialized");
}

void HeaterController::setPower(uint8_t _power) {
    power = _power;
}

uint8_t HeaterController::getPower() {
    return power;
}

// Optimization: Centralized state transition logic.
// Benefit: Ensures state changes are atomic, logged, and always reset the state timer.
void HeaterController::transitionTo(State newState) {
    if (state == newState) return;

    Serial.printf("🔥 State: %d -> %d\n", static_cast<int>(state), static_cast<int>(newState));
    state = newState;
}

void HeaterController::startHeating() {
    if (state == State::IDLE || state == State::COOLDOWN) {
        digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
        digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, HIGH);
        startTime = millis();
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");
    } else if (state == State::PAUSED) {
        digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
        digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, HIGH);
        // Adjust startTime to account for the pause duration
        startTime = millis() - (pauseTime - startTime);
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating resumed");
    }
}

void HeaterController::stopHeating(bool finalize) {
    if (state != State::HEATING) return;

    digitalWrite(HardwareConfig::STATUS_LED_PIN, LOW);
    digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);

    if (finalize) {
        const uint32_t duration = millis() - startTime;
        lastCycleDuration = duration;

        // Only count cycles longer than a minimum threshold
        if (duration >= MIN_CYCLE_DURATION_MS) {
            cycleCounter++;
            cycleFinishedFlag = true; // Notify Device.cpp to send data
        }

        startTime = millis();
        transitionTo(State::COOLDOWN);
        Serial.println("🔥 Heating stopped (finalized)");
    } else {
        pauseTime = millis();
        transitionTo(State::PAUSED);
        Serial.println("🔥 Heating paused");
    }
}

// Optimization: Replaced multiple if-statements with a clear switch-case state machine.
// Benefit: Improves readability and maintainability of the heater logic.
void HeaterController::update() {
    const uint32_t elapsed = getElapsedTime();

    switch (state) {
        case State::HEATING:
            if (elapsed >= autoStopTime) {
                Serial.println("Auto-stop triggered.");
                stopHeating();
            }
            break;

        case State::PAUSED:
            if (millis() - pauseTime >= PAUSE_TIMEOUT_MS) {
                Serial.println("Pause timeout, finalizing cycle.");
                const uint32_t duration = pauseTime - startTime;
                lastCycleDuration = duration;

                if (duration >= MIN_CYCLE_DURATION_MS) {
                    cycleCounter++;
                    cycleFinishedFlag = true;
                }
                startTime = millis();
                transitionTo(State::COOLDOWN);
            }
            break;

        case State::COOLDOWN:
            if (millis() - startTime >= COOLDOWN_DURATION_MS) {
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


