#include "StatsManager.h"

StatsManager::StatsManager() 
    : totalCycles(0), sessionCycles(0), totalDuration(0), averageDuration(0), 
      clicks(0), caps(0), lastClick(""), consumption("") {
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

void StatsManager::updateSessionData(const JsonObject& data) {
    if (data.containsKey("clicks")) {
        clicks = data["clicks"].as<int>();
    }
    if (data.containsKey("caps")) {
        caps = data["caps"].as<int>();
    }
    if (data.containsKey("lastClick")) {
        lastClick = data["lastClick"].as<String>();
    }
    if (data.containsKey("consumption")) {
        consumption = data["consumption"].as<String>();
    }

    Serial.printf("[Stats] Session updated: Clicks=%d, Caps=%d, LastClick=%s, Consumption=%s\n",
                  clicks, caps, lastClick.c_str(), consumption.c_str());
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

// Implementation of new getters
int StatsManager::getClicks() const {
    return clicks;
}

int StatsManager::getCaps() const {
    return caps;
}

String StatsManager::getLastClick() const {
    return lastClick;
}

String StatsManager::getConsumption() const {
    return consumption;
}