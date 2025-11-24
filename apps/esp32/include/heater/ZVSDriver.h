#pragma once

#include <Arduino.h>
#include <functional>

/**
 * @brief Low-level driver for ZVS (Zero Voltage Switching) heater control
 * 
 * Handles:
 * - MOSFET switching
 * - PWM duty cycle management
 * - Temperature sensor timing during OFF phase
 * - Hardware safety
 */
class ZVSDriver {
public:
    using TempMeasureCallback = std::function<void()>;
    
    /**
     * @brief Construct ZVS driver
     * @param mosfetPin GPIO pin for MOSFET control
     * @param statusLedPin GPIO pin for status LED (optional)
     */
    ZVSDriver(uint8_t mosfetPin, uint8_t statusLedPin = 255);
    
    /**
     * @brief Initialize hardware pins
     */
    void init();
    
    /**
     * @brief Update duty cycle state machine (call in loop)
     * Must be called frequently for accurate timing
     */
    void update();
    
    /**
     * @brief Enable/disable ZVS output
     * @param enable true to start duty cycle, false to force OFF
     */
    void setEnabled(bool enable);
    
    /**
     * @brief Set power level (duty cycle percentage)
     * @param power Power level 0-100%
     */
    void setPower(uint8_t power);
    
    /**
     * @brief Set duty cycle period
     * @param periodMs Period in milliseconds (default 1000ms)
     */
    void setPeriod(uint32_t periodMs);
    
    /**
     * @brief Set sensor measurement time during OFF phase
     * @param timeMs Time reserved for sensor reading in ms (default 200ms)
     */
    void setSensorOffTime(uint32_t timeMs);
    
    /**
     * @brief Register callback for temperature measurement
     * Called during OFF phase when sensor can safely read
     * @param callback Function to call for temp measurement
     */
    void onTempMeasure(TempMeasureCallback callback);
    
    /**
     * @brief Check if ZVS is currently enabled
     */
    bool isEnabled() const { return enabled; }
    
    /**
     * @brief Check if MOSFET is physically ON right now
     */
    bool isPhysicallyOn() const { return physicallyOn; }
    
    /**
     * @brief Get current power setting
     */
    uint8_t getPower() const { return power; }
    
    /**
     * @brief Get current duty cycle phase
     */
    enum class Phase : uint8_t {
        OFF_IDLE,       // Disabled, MOSFET off
        ON_PHASE,       // Active heating, MOSFET on
        OFF_PHASE,      // Duty cycle off time, MOSFET off
        SENSOR_WINDOW   // Sensor measurement window during off phase
    };
    
    Phase getCurrentPhase() const { return currentPhase; }
    
    /**
     * @brief Get time elapsed in current phase (ms)
     */
    uint32_t getPhaseElapsed() const;
    
    /**
     * @brief Get statistics
     */
    
    struct Stats {
        uint32_t totalOnTime;    // Total time MOSFET was ON (ms)
        uint32_t totalOffTime;   // Total time MOSFET was OFF (ms)
        uint32_t cycleCount;     // Number of completed duty cycles
        uint32_t tempMeasures;   // Number of temp measurements
    };

    const Stats& getStats() const { return stats; }
    void resetStats();

private:
    // Hardware pins
    uint8_t mosfetPin;
    uint8_t statusLedPin;
    
    // Configuration
    uint8_t power;              // 0-100%
    uint32_t periodMs;          // Duty cycle period
    uint32_t sensorOffTimeMs;   // Time reserved for sensor reading
    
    // State
    bool enabled;
    bool physicallyOn;
    Phase currentPhase;
    uint32_t phaseStartTime;
    
    // Callback
    TempMeasureCallback tempMeasureCallback;
    bool tempMeasureCalled;     // Flag to call callback only once per cycle
    
    // Statistics
    Stats stats;
    
    // Internal methods
    void setMosfet(bool on);
    void updateStatusLed();
    uint32_t calculateOnTime() const;
    uint32_t calculateOffTime() const;
    void transitionPhase(Phase newPhase);
};
