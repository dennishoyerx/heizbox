#include "HeaterController.h"
#include <Arduino.h>

#define LED_PIN             2
#define MOSFET_PIN          13

HeaterController::HeaterController()
    : state(IDLE), startTime(0), autoStopTime(60000), cycleCounter(0), lastCycleDuration(0), cycleFinishedFlag(false) { // Default 2 minutes
}

void HeaterController::init() {
    prefs.begin("heater", false);
    cycleCounter = prefs.getULong("cycles", 0);
    autoStopTime = prefs.getULong("autostop", 120000);
    prefs.end();

    pinMode(LED_PIN, OUTPUT);
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);

    Serial.println("🔥 Heater initialized");
}

void HeaterController::setState(HeaterState newState) {
    if (state != newState) {
        state = newState;
        Serial.printf("🔥 Heater state changed to: %d\n", state);
    }
}

void HeaterController::startHeating() {
    if (state == IDLE) {
        setState(HEATING);
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(MOSFET_PIN, HIGH);
        startTime = millis();
        Serial.println("🔥 Heating started");
    }
}

void HeaterController::stopHeating() {
    if (state == HEATING) {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(MOSFET_PIN, LOW);

        // Calculate duration and update stats
        unsigned long duration = millis() - startTime;
        lastCycleDuration = duration; // Store the duration

        if (duration >= 10000) { // Only count cycles longer than 10 seconds
            cycleCounter++;
            prefs.begin("heater", false);
            prefs.putULong("cycles", cycleCounter);
            prefs.end();
            cycleFinishedFlag = true; // Set flag for external notification

            // Here we would also update StatsManager
        }

        setState(COOLDOWN);
        Serial.println("🔥 Heating stopped");

        // Set a cooldown period before returning to IDLE
        startTime = millis();
    }
}

void HeaterController::update() {
    // Handle cooldown state
    if (state == COOLDOWN && (millis() - startTime > 3000)) { // 3 second cooldown
        setState(IDLE);
    }

    // Handle auto-stop
    if (state == HEATING && (millis() - startTime >= autoStopTime)) {
        stopHeating();
    }
}

HeaterState HeaterController::getState() const {
    return state;
}

bool HeaterController::isHeating() const {
    return state == HEATING;
}

unsigned long HeaterController::getElapsedTime() const {
    if (state == HEATING) {
        return millis() - startTime;
    }
    return 0;
}

unsigned long HeaterController::getCycleCount() const {
    return cycleCounter;
}

void HeaterController::setAutoStopTime(unsigned long time) {
    autoStopTime = time;
    prefs.begin("heater", false);
    prefs.putULong("autostop", autoStopTime);
    prefs.end();
}

unsigned long HeaterController::getAutoStopTime() const {
    return autoStopTime;
}

unsigned long HeaterController::getLastCycleDuration() const {
    return lastCycleDuration;
}

bool HeaterController::isCycleFinished() const {
    return cycleFinishedFlag;
}

void HeaterController::clearCycleFinishedFlag() {
    cycleFinishedFlag = false;
}