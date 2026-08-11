#include "heater/HeaterController.h"
#include "heater/Safety.h"
#include "Config.h"
#include <Arduino.h>
#include "utils/Logger.h"
#include "core/EventBus.h"
#include "SysModule.h"
#include "driver/Audio.h"

HeaterController::HeaterController()
    : BaseClass("HeaterController"),
      state(State::IDLE), 
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
    zvsDriver->setPeriod(HeaterConfig::ZVS::DUTY_CYCLE_PERIOD_MS);
    zvsDriver->setSensorOffTime(HeaterConfig::KSensor::OFF_TIME_MS);

    temperature.init();

    zvsDriver->onPhaseChange([this](ZVSDriver::Phase phase) {
        auto& hs = HeaterState::instance();
        hs.zvsOn.set(phase == ZVSDriver::Phase::ON_PHASE);
    });

    zvsDriver->onTempMeasure([this]() {
        //temperature.update(Sensors::Sensor::K, true);
    });
    
    logger.info("Initialized");
    booted();
}

void HeaterController::transitionTo(State newState) {
    if (state == newState) return;

    Serial.printf("🔥 State: %d -> %d\n", static_cast<int>(state), static_cast<int>(newState));
    state = newState;
}

void HeaterController::startHeating() {
    auto& hs = HeaterState::instance();
    
    Audio::beepHeatStart();
    if (state == State::IDLE) {
        heatCycle.start();
        zvsDriver->setEnabled(true);
        
        transitionTo(State::HEATING);
        logger.info("🔥 Heating started");

        hs.isHeating.set(true);
        peakTemp = 0;
        heatStartTime = millis();
    } else if (state == State::PAUSED) {
        heatCycle.start();
        zvsDriver->setEnabled(true);
        hs.isHeating.set(true);
        
        transitionTo(State::HEATING);
        logger.info("🔥 Heating resumed");
        peakTemp = 0;
        heatStartTime = millis();
    }
}

void HeaterController::stopHeating(bool finalize) {
    if (state != State::HEATING) return;
    auto& hs = HeaterState::instance();

    zvsDriver->setEnabled(false);
    hs.isHeating.set(false);

    if (finalize) {
        heatCycle.submit();
        transitionTo(State::IDLE);
        logger.info("🔥 Heating stopped (finalized)");
    } else {
        Audio::beepHeatFinish();
        heatCycle.stop();
        pauseTime = millis();
        transitionTo(State::PAUSED);
        logger.info("🔥 Heating paused");
    }
}

void HeaterController::update() {
    auto& hs = HeaterState::instance();
    
    updateTemperature();

    if (state == State::HEATING) {
        if (Safety::checkFailed()) {
            stopHeating(false);
            return;
        }

        // Peak tracken (fuer Vape-Entfernung Erkennung)
        if (hs.temp > peakTemp) peakTemp = hs.temp;

        // Vape entfernt -> Temp fällt unter Peak -> Heater stoppen
        if (vaporRemoved(hs)) {
            logPrint("log", "🔥 Vape removed (temp %u -> %u), stopping", peakTemp, static_cast<unsigned int>(hs.temp));
            stopHeating(true);
            return;
        }

        zvsDriver->update();
        hs.timer.set(heatCycle.getTimer());
        return;
    }

    if (state == State::PAUSED && millis() - pauseTime >= hs.cycleTimeout) {
        logger.info("Pause timeout, finalizing cycle.");
        heatCycle.submit();

        transitionTo(State::IDLE);
    }
}

void HeaterController::updateTemperature() {
    auto& hs = HeaterState::instance();
    uint16_t irTemp = 0;
    uint16_t kTemp = 0;

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
        adjusted = adjusted * slope + offset;

        irTemp = static_cast<uint16_t>(adjusted + 0.5f); // Rundung
        hs.tempIR.set(irTemp);
        hs.temp.set(irTemp);
    }
}

bool HeaterController::vaporRemoved(HeaterState& hs) {
    // Nur nach Mindest-Heizdauer aktiv, damit der anfängliche Anstieg nicht als Drop zählt
    if (millis() - heatStartTime < HeaterConfig::TEMP_DROP_MIN_HEAT_MS) return false;
    // Erst ab einem Mindest-Peak aktiv (Gerät muss wirklich warm geworden sein)
    if (peakTemp < HeaterConfig::TEMP_DROP_MIN_PEAK) return false;
    // Signifikanter Abfall unter den Peak = Vape wurde entfernt
    return hs.temp <= peakTemp - HeaterConfig::TEMP_DROP_THRESHOLD;
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