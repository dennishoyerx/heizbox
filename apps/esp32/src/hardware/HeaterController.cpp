// ==== OPTIMIZED FILE ====
// This file has been refactored to implement a type-safe, explicit state machine.
// Key improvements:
// - Replaced integer-based states with a scoped enum 'State' for type safety.
// - Centralized all state changes in a 'transitionTo()' method to ensure consistency.
// - Rewrote the 'update()' method with a clear switch-case structure.
// - Replaced magic numbers with named constants (e.g., COOLDOWN_DURATION_MS).

#include "hardware/HeaterController.h"
#include "core/Config.h"
#include <Arduino.h>



HeaterController::HeaterController()
    : state(State::IDLE), 
      startTime(0), 
      autoStopTime(60000), 
      cycleCounter(0), 
      lastCycleDuration(0), 
      cycleFinishedFlag(false) {
}

void HeaterController::init() {
    pinMode(HardwareConfig::HEATER_MOSFET_PIN, OUTPUT);
    digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);

    Serial.println("🔥 Heater initialized");
}

// Optimization: Centralized state transition logic.
// Benefit: Ensures state changes are atomic, logged, and always reset the state timer.
void HeaterController::transitionTo(State newState) {
    if (state == newState) return;

    Serial.printf("🔥 State: %d -> %d\n", static_cast<int>(state), static_cast<int>(newState));
    state = newState;
    startTime = millis(); // Reset timer on every state transition
}

void HeaterController::startHeating() {
    if (state == State::IDLE || state == State::COOLDOWN) {
        digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
        digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, HIGH);
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");
    }
}

void HeaterController::stopHeating() {
    if (state != State::HEATING) return;

    digitalWrite(HardwareConfig::STATUS_LED_PIN, LOW);
    digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);

    const uint32_t duration = millis() - startTime;
    lastCycleDuration = duration;

    // Only count cycles longer than a minimum threshold
    if (duration >= MIN_CYCLE_DURATION_MS) {
        cycleCounter++;
        cycleFinishedFlag = true; // Notify Device.cpp to send data
    }

    transitionTo(State::COOLDOWN);
    Serial.println("🔥 Heating stopped");
}

// Optimization: Replaced multiple if-statements with a clear switch-case state machine.
// Benefit: Improves readability and maintainability of the heater logic.
void HeaterController::update() {
    const uint32_t elapsed = millis() - startTime;

    switch (state) {
        case State::HEATING:
            if (elapsed >= autoStopTime) {
                Serial.println("Auto-stop triggered.");
                stopHeating();
            }
            break;

        case State::COOLDOWN:
            if (elapsed >= COOLDOWN_DURATION_MS) {
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

uint32_t HeaterController::getElapsedTime() const {
    if (state == State::HEATING) {
        return millis() - startTime;
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


