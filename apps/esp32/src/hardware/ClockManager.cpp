#include "hardware/ClockManager.h"
#include <WiFi.h>
#include <time.h>



ClockManager::ClockManager() : timeSynced(false), daylightOffset_sec(0), gmtOffset_sec(7200) { // Default to GMT+1
}

void ClockManager::init() {
    reconfigureTime();
}

void ClockManager::reconfigureTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.printf("🕐 Time configured with offset: %ld seconds\n", gmtOffset_sec);
}

void ClockManager::update() {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 500)) { // Small timeout to attempt sync
        if (!timeSynced) {
            timeSynced = true;
            Serial.printf("🕐 Time synchronized via NTP: %s", asctime(&timeinfo));
        }
    } else {
        if (timeSynced) {
            Serial.println("❌ Time synchronization lost");
        }
        timeSynced = false;
    }
}

String ClockManager::getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "--:--";
    }
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    return String(buffer);
}

String ClockManager::getFormattedDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "--.--.--";
    }
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d.%m.%Y", &timeinfo);
    return String(buffer);
}

bool ClockManager::isTimeSynced() const {
    return timeSynced;
}

void ClockManager::setTimezoneOffset(long offset_sec) {
    gmtOffset_sec = offset_sec;
    reconfigureTime(); // Re-apply the time configuration
}

long ClockManager::getTimezoneOffset() const {
    return gmtOffset_sec;
}
