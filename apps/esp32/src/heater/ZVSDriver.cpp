#include "heater/ZVSDriver.h"
#include "utils/Logger.h"

ZVSDriver::ZVSDriver(uint8_t mosfetPin, uint8_t statusLedPin)
    : mosfetPin(mosfetPin),
      statusLedPin(statusLedPin),
      power(100),
      periodMs(1000),
      sensorOffTimeMs(200),
      enabled(false),
      physicallyOn(false),
      currentPhase(Phase::OFF_IDLE),
      phaseStartTime(0),
      tempMeasureCallback(nullptr),
      phaseChangeCallback(nullptr),
      tempMeasureCalled(false)
{
    memset(&stats, 0, sizeof(stats));
}

void ZVSDriver::init() {
    // Configure MOSFET pin
    pinMode(mosfetPin, OUTPUT);
    digitalWrite(mosfetPin, LOW);
    
    // Configure status LED if provided
    if (statusLedPin != 255) {
        pinMode(statusLedPin, OUTPUT);
        digitalWrite(statusLedPin, LOW);
    }
    
    physicallyOn = false;
    currentPhase = Phase::OFF_IDLE;
    phaseStartTime = millis();
}

void ZVSDriver::update() {
    if (!enabled) {
        // Ensure everything is off when disabled
        if (currentPhase != Phase::OFF_IDLE) {
            transitionPhase(Phase::OFF_IDLE);
            setMosfet(false);
            updateStatusLed();
        }
        return;
    }
    
    if (sensorOffTimeMs == 0 && power == 100) return;

    const uint32_t now = millis();
    const uint32_t elapsed = now - phaseStartTime;
    
    switch (currentPhase) {
        case Phase::OFF_IDLE:
            // Should not happen when enabled, transition to ON
            transitionPhase(Phase::ON_PHASE);
            break;
            
        case Phase::ON_PHASE:
            {
                const uint32_t onTime = calculateOnTime();
                
                if (elapsed >= onTime) {
                    // ON phase complete, transition to OFF
                    transitionPhase(Phase::OFF_PHASE);
                    setMosfet(false);
                    tempMeasureCalled = false; // Reset for new cycle
                }
            }
            break;
            
        case Phase::OFF_PHASE:
            {
                const uint32_t offTime = calculateOffTime();
                
                // Check if we should enter sensor window
                if (tempMeasureCalled && elapsed >= (offTime - sensorOffTimeMs)) {
                    transitionPhase(Phase::SENSOR_WINDOW);
                }
                
                // Check if OFF phase is complete
                if (elapsed >= offTime) {
                    // Cycle complete, start new cycle
                    stats.cycleCount++;
                    transitionPhase(Phase::ON_PHASE);
                    setMosfet(true);
                }
            }
            break;
            
        case Phase::SENSOR_WINDOW:
            {
                // Call temperature measurement callback once
                if (!tempMeasureCalled && tempMeasureCallback) {
                    tempMeasureCallback();
                    tempMeasureCalled = true;
                    stats.tempMeasures++;
                }
                
                // Check if OFF phase is complete
                const uint32_t offTime = calculateOffTime();
                if (elapsed >= offTime) {
                    // Cycle complete, start new cycle
                    stats.cycleCount++;
                    transitionPhase(Phase::ON_PHASE);
                    setMosfet(true);
                }
            }
            break;
    }
    
    updateStatusLed();
}

void ZVSDriver::setEnabled(bool enable) {
    if (enabled == enable) return;
    
    enabled = enable;
    
    if (enabled) {
        phaseStartTime = millis();
        transitionPhase(Phase::ON_PHASE);
        setMosfet(true);
    } else {
        transitionPhase(Phase::OFF_IDLE);
        setMosfet(false);
    }
}

void ZVSDriver::setPower(uint8_t newPower) {
    power = constrain(newPower, 0, 100);
}

void ZVSDriver::setPeriod(uint32_t periodMs) {
    this->periodMs = max(100u, periodMs); // Min 100ms period
}

void ZVSDriver::setSensorOffTime(uint32_t timeMs) {
    sensorOffTimeMs = timeMs;
}

void ZVSDriver::onTempMeasure(TempMeasureCallback callback) {
    tempMeasureCallback = callback;
}

void ZVSDriver::onPhaseChange(PhaseChangeCallback callback) {
    phaseChangeCallback = callback;
}


uint32_t ZVSDriver::getPhaseElapsed() const {
    return millis() - phaseStartTime;
}

void ZVSDriver::resetStats() {
    memset(&stats, 0, sizeof(stats));
    //logPrint("ZVSDriver", "Statistics reset");
}

// Private methods

void ZVSDriver::setMosfet(bool on) {
    if (physicallyOn == on) return;
    
    physicallyOn = on;
    digitalWrite(mosfetPin, on ? HIGH : LOW);
    
    // Update statistics
    if (on) {
        // Track off time when turning on
        if (currentPhase != Phase::OFF_IDLE) {
            stats.totalOffTime += getPhaseElapsed();
        }
    } else {
        // Track on time when turning off
        stats.totalOnTime += getPhaseElapsed();
    }
}

void ZVSDriver::updateStatusLed() {
    if (statusLedPin == 255) return;
    
    // Status LED reflects MOSFET state when enabled
    if (enabled) {
        digitalWrite(statusLedPin, physicallyOn ? HIGH : LOW);
    } else {
        digitalWrite(statusLedPin, LOW);
    }
}

uint32_t ZVSDriver::calculateOnTime() const {
    // At 100% power, ensure we still have sensor window
    if (power >= 100) {
        return periodMs - sensorOffTimeMs;
    }
    
    // ON time = (period * power) / 100
    return (periodMs * power) / 100;
}

uint32_t ZVSDriver::calculateOffTime() const {
    // At 100% power, only OFF for sensor measurement
    if (power >= 100) {
        return sensorOffTimeMs;
    }
    
    // OFF time = period - ON time
    const uint32_t onTime = calculateOnTime();
    return periodMs - onTime;
}

void ZVSDriver::transitionPhase(Phase newPhase) {
    if (currentPhase == newPhase) return;
    
    currentPhase = newPhase;
    phaseStartTime = millis();

    if (phaseChangeCallback) phaseChangeCallback(newPhase);
}