#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#include <WiFi.h>
#include <time.h>

class ClockManager {
private:
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 3600; // GMT+1
    const int daylightOffset_sec = 3600; // Daylight saving time

    bool timeSynced;

public:
    ClockManager();
    void init();
    void update();
    String getFormattedTime();
    String getFormattedDate();
    bool isTimeSynced() const;
};

#endif