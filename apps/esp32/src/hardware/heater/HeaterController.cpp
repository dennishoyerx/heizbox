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
      cycleFinishedFlag(false),
      dutyCycleStartTime(0),
      heaterPhysicallyOn(false),
      tempSensor(new TempSensor(HardwareConfig::THERMO_SCK_PIN, HardwareConfig::THERMO_CS_PIN, HardwareConfig::THERMO_SO_PIN, HardwareConfig::SENSOR_TEMPERATURE_READ_INTERVAL_MS))
       {
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
    if (_power > 100) _power = 100;
    if (_power < 10) _power = 10;
    power = _power;
}

uint8_t HeaterController::getPower() {
    return power;
}

void HeaterController::transitionTo(State newState) {
    if (state == newState) return;

    Serial.printf("🔥 State: %d -> %d\n", static_cast<int>(state), static_cast<int>(newState));
    state = newState;
}

float HeaterController::getTemperature() {
    return tempSensor->getTemperature();
}


void HeaterController::startHeating() {
    if (state == State::IDLE || state == State::COOLDOWN) {
        digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
        startTime = millis();
        dutyCycleStartTime = millis();
        heaterPhysicallyOn = false; // Will be controlled by duty cycle
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");
    } else if (state == State::PAUSED) {
        digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
        // Adjust startTime to account for the pause duration
        startTime = millis() - (pauseTime - startTime);
        dutyCycleStartTime = millis();
        heaterPhysicallyOn = false;
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating resumed");
    }
}

void HeaterController::stopHeating(bool finalize) {
    if (state != State::HEATING) return;

    digitalWrite(HardwareConfig::STATUS_LED_PIN, LOW);
    digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);
    heaterPhysicallyOn = false;

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

void HeaterController::updateDutyCycle() {
    if (state != State::HEATING) {
        // Ensure heater is off when not in HEATING state
        if (heaterPhysicallyOn) {
            digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);
            heaterPhysicallyOn = false;
        }
        return;
    }

    const uint32_t dutyCycleElapsed = millis() - dutyCycleStartTime;
    const uint32_t onTime = (DUTY_CYCLE_PERIOD_MS * power) / 100;
    
    if (dutyCycleElapsed < onTime) {
        // ON phase
        if (!heaterPhysicallyOn) {
            digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, HIGH);
            heaterPhysicallyOn = true;
        }
    } else if (dutyCycleElapsed < DUTY_CYCLE_PERIOD_MS) {
        // OFF phase
        if (heaterPhysicallyOn) {
            digitalWrite(HardwareConfig::HEATER_MOSFET_PIN, LOW);
            heaterPhysicallyOn = false;
        }

        tempSensor->update(true); 
    } else {
        // Start new duty cycle
        dutyCycleStartTime = millis();
    }
}

void HeaterController::update() {
    // Update duty cycle first (controls physical heater switching)
    updateDutyCycle();

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
            tempSensor->update(); 
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