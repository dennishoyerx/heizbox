#ifndef HEATERCONTROLLER_H
#define HEATERCONTROLLER_H

#include <cstdint>



class HeaterController {
public:
    // Optimization: Use a scoped enum for type-safe state representation.
    // Benefit: Prevents bugs from implicit conversions and name clashes.
    enum class State : uint8_t {
        IDLE,
        HEATING,
        COOLDOWN,
        ERROR
    };

    HeaterController();
    void init();
    void startHeating();
    void stopHeating();
    void update();

    State getState() const;
    bool isHeating() const;
    uint32_t getElapsedTime() const;
    uint32_t getCycleCount() const;
    uint32_t getLastCycleDuration() const;
    bool isCycleFinished() const;
    void clearCycleFinishedFlag();
    void setAutoStopTime(uint32_t time);
    uint32_t getAutoStopTime() const;

private:
    // Optimization: Use explicit constants for magic numbers.
    // Benefit: Improves readability and makes tuning easier.
    static constexpr uint32_t COOLDOWN_DURATION_MS = 3000;
    static constexpr uint32_t MIN_CYCLE_DURATION_MS = 10000;

    void transitionTo(State newState);

    State state = State::IDLE;
    uint32_t startTime = 0;
    uint32_t autoStopTime = 60000; // Default 60 seconds
    uint32_t cycleCounter = 0;
    uint32_t lastCycleDuration = 0;
    bool cycleFinishedFlag = false;
};



#endif