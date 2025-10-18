#ifndef STATSMANAGER_H
#define STATSMANAGER_H

#include <Preferences.h>
#include <ArduinoJson.h>

class StatsManager {
private:
    Preferences prefs;
    unsigned long totalCycles;
    unsigned long sessionCycles;
    unsigned long totalDuration;
    float averageDuration;

    // New session data fields
    int clicks;
    int caps;
    String lastClick;
    String consumption;

public:
    StatsManager();
    void init();
    void addCycle(unsigned long duration);
    void resetSession();
    void updateSessionData(const JsonObject& data);
    
    unsigned long getTotalCycles() const;
    unsigned long getSessionCycles() const;
    float getAverageDuration() const;

    // Getters for new session data
    int getClicks() const;
    int getCaps() const;
    String getLastClick() const;
    String getConsumption() const;
};

#endif