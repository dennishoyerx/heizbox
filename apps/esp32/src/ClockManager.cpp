#include "ClockManager.h"
#include <WiFi.h>
#include <time.h>




ClockManager::ClockManager() : timeSynced(false) {
}

void ClockManager::init() {
    // Note: WiFi connection would need to be established first
    // This is a simplified implementation

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void ClockManager::update() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 0)) { // Non-blocking check
        if (!timeSynced) {
            timeSynced = true;
            Serial.printf("🕐 Time synchronized via NTP: %s\n", asctime(&timeinfo));
        }
    } else {
        if (timeSynced) {
            Serial.println("❌ Time synchronization lost");
        }
        timeSynced = false; // Time is no longer synced
    }
}

String ClockManager::getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) {
        return "--:--";
    }

    time_t rawtime;
    time(&rawtime);
    rawtime += (gmtOffset_sec + daylightOffset_sec);
    localtime_r(&rawtime, &timeinfo);

    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    return String(buffer);
}

String ClockManager::getFormattedDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) {
        return "--.--.--";
    }

    time_t rawtime;
    time(&rawtime);
    rawtime += (gmtOffset_sec + daylightOffset_sec);
    localtime_r(&rawtime, &timeinfo);

    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y", &timeinfo);
    return String(buffer);
}

bool ClockManager::isTimeSynced() const {
    return timeSynced;
}