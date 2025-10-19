// ==== OPTIMIZED FILE ==== 
// This file has been refactored to reduce flash wear by batching NVS writes.
// Key improvements:
// - 'addCycle()' no longer writes to flash on every call. It now only updates RAM and sets a 'dirty' flag.
// - A new 'update()' method, called from the main loop, checks if it's time to write.
// - 'flushToNvs()' performs the actual write, but only when necessary.

#include "StatsManager.h"
#include "config.h"
#include <Arduino.h>

StatsManager::StatsManager() 
    : totalCycles(0), sessionCycles(0), totalDuration(0), averageDuration(0), 
      clicks(0), caps(0), lastClick(""), consumption(""), 
      nvsDirty(false), lastNvsWrite(0) {
}

void StatsManager::init() {
    prefs.begin("stats", true); // Read-only is faster
    totalCycles = prefs.getULong("total_cycles", 0);
    totalDuration = prefs.getULong("total_duration", 0);
    prefs.end();
    
    if (totalCycles > 0) {
        averageDuration = totalDuration / (float)totalCycles / 1000.0f; // Convert to seconds
    }
    
    Serial.println("📊 StatsManager initialized");
}

// Optimization: Defer flash writes by setting a dirty flag.
// Benefit: Reduces flash writes from potentially hundreds per hour to just a few, extending hardware life.
void StatsManager::addCycle(unsigned long duration) {
    sessionCycles++;
    totalCycles++;
    totalDuration += duration;
    
    if (totalCycles > 0) {
        averageDuration = totalDuration / (float)totalCycles / 1000.0f;
    }
    
    nvsDirty = true; // Mark that we have unsaved changes
    Serial.printf("Cycle added: %lu ms, Total: %lu cycles. NVS write deferred.\n", duration, totalCycles);
}

// This method should be called in the main device loop.
void StatsManager::update() {
    if (nvsDirty && (millis() - lastNvsWrite > Config::Timing::NVS_FLUSH_INTERVAL_MS)) {
        flushToNvs();
    }
}

void StatsManager::flushToNvs() {
    if (!nvsDirty) return;

    prefs.begin("stats", false);
    prefs.putULong("total_cycles", totalCycles);
    prefs.putULong("total_duration", totalDuration);
    prefs.end();

    nvsDirty = false;
    lastNvsWrite = millis();
    Serial.println("💾 Stats saved to NVS");
}

void StatsManager::updateSessionData(const JsonObjectConst& data) {
    // Optimization: Use proper null-checking for JSON values.
    // Benefit: Prevents runtime errors and ensures data is only updated when valid.
    if (!data["clicks"].isNull()) {
        clicks = data["clicks"].as<int>();
    }
    if (!data["caps"].isNull()) {
        caps = data["caps"].as<int>();
    }
    if (!data["lastClick"].isNull()) {
        lastClick = data["lastClick"].as<String>();
    }
    if (!data["consumption"].isNull()) {
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
