#include "heater/HeaterController.h"
#include "Config.h"
#include <Arduino.h>
#include "utils/Logger.h"
#include "core/EventBus.h"
#include "SysModule.h"

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

    mode = new HeatMode();
}

void HeaterController::init() {
    auto booted = SysModules::booting("heater");
    zvsDriver->init();
    zvsDriver->setPeriod(HeaterConfig::ZVS::DUTY_CYCLE_PERIOD_MS);
    zvsDriver->setSensorOffTime(HeaterConfig::KSensor::OFF_TIME_MS);
    zvsDriver->setPower(power);
    
    temperature.init();

    zvsDriver->onPhaseChange([this](ZVSDriver::Phase phase) {
        auto& hs = HeaterState::instance();
        hs.zvsOn.set(phase == ZVSDriver::Phase::ON_PHASE);
    });

    zvsDriver->onTempMeasure([this]() {
        //temperature.update(Sensors::Sensor::K, true);
    });
    
    
    Serial.println("🔥 Heater initialized with ZVS driver");
    booted();
}

void HeaterController::setPower(uint8_t _power) {
    power = _power;
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
        heatCycle.start();
        startTime = millis();
        //HeatLog::instance().start();

        zvsDriver->setEnabled(true);
        
        transitionTo(State::HEATING);
        Serial.println("🔥 Heating started");

        hs.isHeating.set(true);
        hs.startTime.set(startTime);
    } else if (state == State::PAUSED) {
        heatCycle.start();
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
        heatCycle.submit();
        startTime = millis();
        transitionTo(State::IDLE);
        Serial.println("🔥 Heating stopped (finalized)");
        hs.startTime.set(0);
    } else {
        heatCycle.stop();
        pauseTime = millis();
        transitionTo(State::PAUSED);
        Serial.println("🔥 Heating paused");
    }
}

namespace Safety {
    static const int xxx[] = { 1, 3, 5, 7, 11 };

    bool cutoffTemperatureReached() {
        auto& hs = HeaterState::instance();
        if (hs.tempLimit == 420) return false;
        return hs.temp > hs.tempLimit;
    }

    bool noTempClimb() {
        static const int values[] = { };

        auto& hs = HeaterState::instance();
        hs.temp;
        return false;
    };

    bool checkFailed() {
        return cutoffTemperatureReached();
    };
};

void HeaterController::update() {
    auto& hs = HeaterState::instance();
    
    updateTemperature();
    if (Safety::checkFailed()) stopHeating(false);

    zvsDriver->update();
    
    const uint32_t elapsed = getElapsedTime();
    hs.timer.set(elapsed / 1000);

    switch (state) {
        case State::HEATING:
            if (elapsed >= autoStopTime) {
                Serial.println("Auto-stop triggered.");
                stopHeating(false);
            }
            break;

        case State::PAUSED:
            if (millis() - pauseTime >= hs.cycleTimeout) {
                Serial.println("Pause timeout, finalizing cycle.");
                heatCycle.submit();

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
    uint16_t irTemp = 0;
    uint16_t kTemp = 0;

    // IR Sensor
    if (temperature.update(Sensors::Type::IR)) {
        float raw = temperature.get(Sensors::Type::IR);
        if (!isfinite(raw) || raw < 0.0f || raw > 1000.0f) {
            return; // Messung verwerfen
        }

        hs.tempIRRaw.set(static_cast<uint16_t>(raw + 0.5f));

        // apply ambient correction if enabled in the sensor / state (existing behavior)
        float factor = 1.0f + (hs.irCorrection / 100.0f);
        float adjusted = raw * factor;

        // apply two-point calibration if present
        float slope = hs.irCalSlope;
        float offset = hs.irCalOffset;
        float calibrated = adjusted * slope + offset;

        irTemp = static_cast<uint16_t>(calibrated + 0.5f); // Rundung
        hs.tempIR.set(irTemp);
        hs.temp.set(irTemp);
    }
    // K-Sensor, nur wenn Intervall erreicht
    /*
    static u32_t lastTempUpdate = 0;
    if (hs.tempSensorOffTime < 0 && !hs.zvsOn &&
        millis() - lastTempUpdate >= hs.tempSensorReadInterval) {
        lastTempUpdate = millis();

        if (temperature.update(Sensors::Sensor::K)) {
            int16_t correction = isHeating() ? hs.tempCorrection : 0;
            kTemp = static_cast<uint16_t>(temperature.get(Sensors::Sensor::K) + correction);
            hs.tempK.set(kTemp);
        }
    }

    // Temperatur setzen
    if (hs.cutoffIr && irTemp > hs.tempK) {
        hs.temp.set(irTemp);
    } else if (kTemp > 0) {
        hs.temp.set(kTemp);
    }*/
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

// --- calibration helpers ---

int16_t HeaterController::markIRClick(uint16_t actualTemp) {
    auto& hs = HeaterState::instance();

    // Force an immediate IR read (ignore interval) to capture current measurement
    temperature.update(Sensors::Type::IR, true);
    float raw = temperature.get(Sensors::Type::IR);

    if (!isfinite(raw) || raw <= 0.0f || raw > 1000.0f) {
        Serial.println("IR click: invalid measurement, ignored.");
        return -1;
    }
    uint16_t measured = static_cast<uint16_t>(raw + 0.5f);
    int16_t returnVal = -1;

    // If actualTemp matches one of the stored actuals, use that slot. Otherwise pick an empty slot (A first).
    if (actualTemp == hs.irCalActualA) {
        returnVal = hs.irCalMeasuredA.set(measured);
        Serial.printf("IR click stored in A: measured=%u actual=%u\n", measured, actualTemp);
    } else if (actualTemp == hs.irCalActualB) {
        returnVal = hs.irCalMeasuredB.set(measured);
        Serial.printf("IR click stored in B: measured=%u actual=%u\n", measured, actualTemp);
    } else {
        if (hs.irCalMeasuredA == 0) {
            returnVal = hs.irCalMeasuredA.set(measured);
            hs.irCalActualA.set(actualTemp);
            Serial.printf("IR click stored in A (new actual): measured=%u actual=%u\n", measured, actualTemp);
        } else {
            returnVal = hs.irCalMeasuredB.set(measured);
            hs.irCalActualB.set(actualTemp);
            Serial.printf("IR click stored in B (new actual): measured=%u actual=%u\n", measured, actualTemp);
        }
    }

    computeIRCalibration();
    return returnVal;
}

void HeaterController::computeIRCalibration() {
    auto& hs = HeaterState::instance();
    uint16_t mA = hs.irCalMeasuredA;
    uint16_t mB = hs.irCalMeasuredB;
    uint16_t aA = hs.irCalActualA;
    uint16_t aB = hs.irCalActualB;

    if (mA == 0 || mB == 0) {
        Serial.println("IR calibration: need two measured points.");
        return;
    }
    if (mA == mB) {
        Serial.println("IR calibration: measured points identical, cannot compute.");
        return;
    }

    float slope = float(aB - aA) / float(mB - mA);
    float offset = float(aA) - slope * float(mA);

    hs.irCalSlope.set(slope);
    hs.irCalOffset.set(offset);

    Serial.printf("IR calibration computed: slope=%.6f offset=%.2f (mA=%u,aA=%u mB=%u,aB=%u)\n",
                  slope, offset, mA, aA, mB, aB);
}

void HeaterController::clearIRCalibration() {
    auto& hs = HeaterState::instance();
    hs.irCalMeasuredA.set(0);
    hs.irCalMeasuredB.set(0);
    hs.irCalActualA.set(150);
    hs.irCalActualB.set(200);
    hs.irCalSlope.set(1.0f);
    hs.irCalOffset.set(0.0f);
    Serial.println("IR calibration cleared.");
}

float HeaterController::getIRCalibrationSlope() const {
    return HeaterState::instance().irCalSlope;
}

float HeaterController::getIRCalibrationOffset() const {
    return HeaterState::instance().irCalOffset;
}