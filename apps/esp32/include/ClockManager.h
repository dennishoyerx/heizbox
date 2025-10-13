#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#include <WiFi.h>
#include <time.h>

#include <Preferences.h>

class ClockManager {
private:
    Preferences prefs;
    const char* ntpServer = "pool.ntp.org";
    long gmtOffset_sec;
    const int daylightOffset_sec = 0; // Daylight saving is handled by the user setting the correct offset

    bool timeSynced;
    void reconfigureTime();

public:
    ClockManager();
    void init();
    void update();
    String getFormattedTime();
    String getFormattedDate();
    bool isTimeSynced() const;
    void setTimezoneOffset(long offset_sec);
    long getTimezoneOffset() const;
};

#endif