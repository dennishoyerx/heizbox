#pragma once 
#include "Observable.h"
#include "Config.h"

struct ConsumptionState {
    Observable<float> session{0};
    Observable<float> today{0};
    Observable<float> yesterday{0};
};

struct AudioSettings {
    PersistedObservable<uint8_t> volume{"audio", "volume", 100};
};

struct DisplaySettings {
    PersistedObservable<bool> flipOrientation{"display", "flip", false};
    PersistedObservable<uint8_t> brightness{"display", "brightness", 100};
    PersistedObservable<bool> darkMode{"display", "darkMode", true};
    PersistedObservable<uint8_t> idleBrightness{"display", "idle_brightness", 50};
    PersistedObservable<uint32_t> idleTimeout{"display", "idle_timeout", 12000000};

};

struct DebugSettings {
    Observable<bool> input{false};
    PersistedObservable<bool> zvs{"zvs", "debug", false};
    PersistedObservable<bool> osc{"zvs", "osc", false};
    PersistedObservable<bool> showRawTemp{"debug", "show_raw_temp", false};
};

struct Settings {
    PersistedObservable<uint32_t> sleepTimeout{"system", "sleepTimeout", 600000};
    PersistedObservable<int32_t> timezoneOffset{"clock", "gmtOffset", 3600};
};

struct DeviceState: public Settings {
    AudioSettings audio;
    DisplaySettings display;
    DebugSettings debug;
    ConsumptionState consumption;

    Observable<bool> isOnline{false};
    Observable<bool> locked{false};

    PersistedObservable<uint32_t> autoStopTime{"heater", "autostop", 90000};

    static DeviceState& instance();

private:
    DeviceState() = default;
};
