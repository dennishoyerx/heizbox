#include "StatsManager.h"

StatsManager::StatsManager() 
    : totalCycles(0), sessionCycles(0), totalDuration(0), averageDuration(0) {
}

void StatsManager::init() {
    prefs.begin("stats", false);
    totalCycles = prefs.getULong("total_cycles", 0);
    totalDuration = prefs.getULong("total_duration", 0);
    prefs.end();
    
    // Calculate average duration
    if (totalCycles > 0) {
        averageDuration = totalDuration / (float)totalCycles / 1000.0f; // Convert to seconds
    }
    
    Serial.println("StatsManager initialized");
}

void StatsManager::addCycle(unsigned long duration) {
    sessionCycles++;
    totalCycles++;
    totalDuration += duration;
    
    // Update average
    averageDuration = totalDuration / (float)totalCycles / 1000.0f;
    
    // Save to NVS
    prefs.begin("stats", false);
    prefs.putULong("total_cycles", totalCycles);
    prefs.putULong("total_duration", totalDuration);
    prefs.end();
    
    Serial.printf("Cycle added: %lu ms, Total: %lu cycles\n", duration, totalCycles);
}

void StatsManager::resetSession() {
    sessionCycles = 0;
}

unsigned long StatsManager::getTotalCycles() const {
    return totalCycles;
}

unsigned long StatsManager::getSessionCycles() const {
    return sessionCycles;
}

float StatsManager::getAverageDuration() const {
    return averageDuration;
}