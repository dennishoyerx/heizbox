#ifndef STATSMANAGER_H
#define STATSMANAGER_H

#include <ArduinoJson.h>

class StatsManager {
private:
    unsigned long totalCycles;
    unsigned long sessionCycles;
    unsigned long totalDuration;
    float averageDuration;

    // New session data fields
    int clicks;
    int caps;
    String lastClick;
    String consumption;

    // Optimization: Batch NVS writes to reduce flash wear.
    // Benefit: Drastically increases the lifespan of the ESP32's flash memory.
    bool nvsDirty = false;
    uint32_t lastNvsWrite = 0;

    void flushToNvs();

public:
    StatsManager();
    void init();
    void update(); // Must be called in the main loop
    void addCycle(unsigned long duration);
    void resetSession();
    void updateSessionData(const JsonObjectConst& data);
    
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