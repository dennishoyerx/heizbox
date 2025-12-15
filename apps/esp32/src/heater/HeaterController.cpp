#include "heater/HeaterController.h"
#include "Config.h"
#include <Arduino.h>
#include "utils/Logger.h"
#include "core/EventBus.h"
#include "SysModule.h"
#include "heater\HeatData.h"

HeaterController::HeaterController()
    : state(State::IDLE), 
      power(100),
      startTime(0), 
      pauseTime(0),
      autoStopTime(60000) {
    zvsDriver = new ZVSDriver(
        HardwareConfig::HEATER_MOSFET_PIN,
        HardwareConfig::STATUS_LED_PIN
    );
}

void HeaterController::init() {
    auto booted = SysModules::booting("heater");
    zvsDriver->init();
    zvsDriver->setPeriod(HeaterConfig::DUTY_CYCLE_PERIOD_MS);
    zvsDriver->setSensorOffTime(HeaterConfig::SENSOR_OFF_TIME_MS);
    zvsDriver->setPower(power);
    
    temperature.init();

    zvsDriver->onPhaseChange([this](ZVSDriver::Phase phase) {
        auto& hs = HeaterState::instance();
        hs.zvsOn.set(phase == ZVSDriver::Phase::ON_PHASE);
    });

    zvsDriver->onTempMeasure([this]() {
        temperature.update(HeaterTemperature::Sensor::K, true);
    });
    
    
    Serial.println("🔥 Heater initialized with ZVS driver");
    booted();
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

void HeaterController::startHeating() {
    auto& hs = HeaterState::instance();
    if (state == State::IDLE) {
        startTime = millis();
        //HeatLog::instance().start();

        zvsDriver->setEnabled(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");

        hs.isHeating.set(true);
        hs.startTime.set(startTime);
    } else if (state == State::PAUSED) {
        startTime = millis() - (pauseTime - startTime);

        zvsDriver->setEnabled(true);
        hs.isHeating.set(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating resumed");
    }
}

void HeaterController::stopHeating(bool finalize) {
    if (state != State::HEATING) return;
    auto& hs = HeaterState::instance();

    zvsDriver->setEnabled(false);
    hs.isHeating.set(false);

    if (finalize) {
        const uint32_t duration = millis() - startTime;
        
        startTime = millis();
        transitionTo(State::IDLE);
        Serial.println("🔥 Heating stopped (finalized)");
        hs.startTime.set(0);
    } else {
        pauseTime = millis();
        transitionTo(State::PAUSED);
        Serial.println("🔥 Heating paused");
    }
}

void HeaterController::update() {
    auto& hs = HeaterState::instance();
    
    updateTemperature();
    if (hs.temp > hs.tempLimit) stopHeating(false);

    zvsDriver->update();
    
    const uint32_t elapsed = getElapsedTime();
    hs.timer.set(elapsed / 1000);

    switch (state) {
        case State::HEATING:
            if (elapsed >= autoStopTime) {
                Serial.println("Auto-stop triggered.");
                stopHeating();
            }
            break;

        case State::PAUSED:
            if (millis() - pauseTime >= hs.cycleTimeout) {
                Serial.println("Pause timeout, finalizing cycle.");
                const uint32_t duration = pauseTime - startTime;
                
                EventBus::instance().publish<CycleFinishedData>(
                    EventType::CYCLE_FINISHED, {duration, startTime}
                );

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
    auto& hs = HeaterState::instance();
    uint16_t temp;

    if (temperature.update(HeaterTemperature::Sensor::IR)) hs.tempIR.set(temperature.get(HeaterTemperature::Sensor::IR));

    if (hs.tempSensorOffTime > 0 && hs.zvsOn) return;

    static u32_t lastTempUpdate = 0;

    if (millis() - lastTempUpdate < hs.tempSensorReadInterval) return;
    lastTempUpdate = millis();


    if (temperature.update(HeaterTemperature::Sensor::K)) {
        temp = hs.tempK.set(temperature.get(HeaterTemperature::Sensor::K));
        if (temp <= 3) return;
        if (isHeating()) temp += hs.tempCorrection;
        hs.temp.set(temp);
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


void HeaterController::setAutoStopTime(uint32_t time) {
    autoStopTime = time;
}

uint32_t HeaterController::getAutoStopTime() const {
    return autoStopTime;
}
