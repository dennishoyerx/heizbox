#ifndef HEATERCONTROLLER_H
#define HEATERCONTROLLER_H

#include <cstdint>



class HeaterController {
public:
    enum class State : uint8_t {
        IDLE,
        HEATING,
        PAUSED,
        COOLDOWN,
        ERROR
    };

    HeaterController();
    void init();
    void startHeating();
    void stopHeating(bool finalize = true);
    void setPower(uint8_t power);
    uint8_t getPower();
    void update();

    State getState() const;
    bool isHeating() const;
    bool isPaused() const;
    uint32_t getElapsedTime() const;
    uint32_t getCycleCount() const;
    uint32_t getLastCycleDuration() const;
    bool isCycleFinished() const;
    void clearCycleFinishedFlag();
    void setAutoStopTime(uint32_t time);
    uint32_t getAutoStopTime() const;

private:
    static constexpr uint32_t COOLDOWN_DURATION_MS = 3000;
    static constexpr uint32_t MIN_CYCLE_DURATION_MS = 10000;
    static constexpr uint32_t PAUSE_TIMEOUT_MS = 5000;
    static constexpr uint32_t DUTY_CYCLE_PERIOD_MS = 1000; // 1 Sekunde pro Zyklus

    void transitionTo(State newState);

    State state = State::IDLE;
    uint8_t power = 0;
    uint32_t startTime = 0;
    uint32_t pauseTime = 0;
    uint32_t autoStopTime = 60000; // Default 60 seconds
    uint32_t cycleCounter = 0;
    uint32_t lastCycleDuration = 0;
    bool cycleFinishedFlag = false;

    uint32_t dutyCycleStartTime;
    bool heaterPhysicallyOn;
};



#endif