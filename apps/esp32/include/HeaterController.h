#ifndef HEATERCONTROLLER_H
#define HEATERCONTROLLER_H

#include <Preferences.h>

enum HeaterState {
    IDLE,
    HEATING,
    COOLDOWN,
    ERROR
};

class HeaterController {
private:
    Preferences prefs;
    HeaterState state;
    unsigned long startTime;
    unsigned long autoStopTime; // in milliseconds
    unsigned long cycleCounter;
    unsigned long lastCycleDuration;
    bool cycleFinishedFlag;
    
    void setState(HeaterState newState);

public:
    HeaterController();
    void init();
    void startHeating();
    void stopHeating();
    void update();
    
    HeaterState getState() const;
	bool isHeating() const;
    unsigned long getElapsedTime() const;
    unsigned long getCycleCount() const;
    unsigned long getLastCycleDuration() const;
    bool isCycleFinished() const;
    void clearCycleFinishedFlag();
    void setAutoStopTime(unsigned long time);
    unsigned long getAutoStopTime() const;
};

#endif