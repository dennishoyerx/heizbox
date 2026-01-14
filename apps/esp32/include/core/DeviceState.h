// include/core/DeviceState.h
#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include "Observable.h"
#include "Config.h"

class DisplayDriver; // Forward declaration

struct AudioSettings {
    PersistedObservable<uint8_t> volume{"audio", "volume", 100};
};


struct DeviceState {
    AudioSettings audio;

    // Display Settings
    PersistedObservable<uint8_t> brightness{"display", "brightness", 100};
    PersistedObservable<bool> darkMode{"display", "darkMode", true};

    // Heater Settings
    PersistedObservable<uint32_t> autoStopTime{"heater", "autostop", 90000};
    PersistedObservable<uint16_t> targetTemperature{"heater", "temperature", 170};
    PersistedObservable<uint16_t> targetTemperatureCycle1{"heater", "temperatureCycle1", 200};
    PersistedObservable<uint16_t> targetTemperatureCycle2{"heater", "temperatureCycle2", 220};


    // System Settings
    PersistedObservable<uint32_t> sleepTimeout{"system", "sleepTimeout", 600000};
    PersistedObservable<int32_t> timezoneOffset{"clock", "gmtOffset", 3600};
    PersistedObservable<bool> enableCenterButtonForHeating{"system", "enableCenterBtn", false};

    // Runtime State (nicht persistiert)
    Observable<bool> locked{false};
    Observable<bool> isHeating{false};
    Observable<bool> isOnline{false};
    Observable<uint32_t> heatingStartTime{0};

    // Debug
    Observable<bool> debugInput{false};
    PersistedObservable<bool> zvsDebug{"zvs", "debug", false};
    PersistedObservable<bool> oscDebug{"zvs", "osc", false};


    // Session State
    Observable<uint32_t> sessionCycles{0};
    Observable<float> sessionConsumption{0};
    Observable<float> todayConsumption{0};
    Observable<float> yesterdayConsumption{0};

        PersistedObservable<bool> showRawTemp{"debug", "show_raw_temp", false};

    static DeviceState& instance();

private:
    DeviceState() = default;
};

#endif // DEVICESTATE_H
