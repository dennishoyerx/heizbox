#ifndef STATSMANAGER_H
#define STATSMANAGER_H

#include <Preferences.h>

class StatsManager {
private:
    Preferences prefs;
    unsigned long totalCycles;
    unsigned long sessionCycles;
    unsigned long totalDuration;
    float averageDuration;

public:
    StatsManager();
    void init();
    void addCycle(unsigned long duration);
    void resetSession();
    
    unsigned long getTotalCycles() const;
    unsigned long getSessionCycles() const;
    float getAverageDuration() const;
};

#endif