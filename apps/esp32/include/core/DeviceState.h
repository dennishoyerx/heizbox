// include/core/DeviceState.h
#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include "core/Observable.h"

class DisplayDriver; // Forward declaration

struct DeviceState {
    // Display Settings
    PersistedObservable<uint8_t> brightness{"display", "brightness", 100};
    PersistedObservable<bool> darkMode{"display", "darkMode", true};

    // Heater Settings
    PersistedObservable<uint32_t> autoStopTime{"heater", "autostop", 90000};
    PersistedObservable<uint8_t> currentCycle{"heater", "cycle", 1};
    PersistedObservable<bool> smart{"heater", "smart", false};
    PersistedObservable<uint8_t> power{"heater", "power", 100};
    PersistedObservable<float> targetTemperature{"heater", "targetTemperature", 175};


    // System Settings
    PersistedObservable<uint32_t> sleepTimeout{"system", "sleepTimeout", 600000};
    PersistedObservable<int32_t> timezoneOffset{"clock", "gmtOffset", 3600};
    PersistedObservable<bool> enableCenterButtonForHeating{"system", "enableCenterBtn", false};

    // Runtime State (nicht persistiert)
    Observable<bool> isHeating{false};
    Observable<bool> isOnline{false};
    Observable<uint32_t> heatingStartTime{0};

    // Session State
    Observable<int> sessionClicks{0};
    Observable<int> sessionCaps{0};
    Observable<uint32_t> sessionCycles{0};
    Observable<float> sessionConsumption{0};
    Observable<float> todayConsumption{0};
    Observable<float> yesterdayConsumption{0};

    // Statistics
    PersistedObservable<uint32_t> totalCycles{"stats", "total_cycles", 0};
    PersistedObservable<uint32_t> totalDuration{"stats", "total_duration", 0};

    DisplayDriver* display = nullptr;

    // Singleton Access
    static DeviceState& instance();

private:
    DeviceState() = default;
};

#endif // DEVICESTATE_H
