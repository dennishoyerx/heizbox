// include/core/DeviceState.h
#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include "Observable.h"
#include "core/Config.h"

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
    PersistedObservable<uint8_t> targetTemperature{"heater", "temperature", 175};


    // System Settings
    PersistedObservable<uint32_t> sleepTimeout{"system", "sleepTimeout", 600000};
    PersistedObservable<int32_t> timezoneOffset{"clock", "gmtOffset", 3600};
    PersistedObservable<bool> enableCenterButtonForHeating{"system", "enableCenterBtn", false};

    // Runtime State (nicht persistiert)
    Observable<bool> isHeating{false};
    Observable<bool> isOnline{false};
    Observable<uint32_t> heatingStartTime{0};

    PersistedObservable<bool> zvsDebug{"zvs", "debug", true};
    PersistedObservable<uint32_t> zvsDutyCyclePeriodMs{"zvs", "dutyCyclePeriodMs", HeaterConfig::DUTY_CYCLE_PERIOD_MS};
    PersistedObservable<uint32_t> tempSensorOffTime{"heater", "tempSensorOffTime", HeaterConfig::SENSOR_OFF_TIME_MS};

    PersistedObservable<uint8_t> heatCycleTempDelta{"heater", "heatCycleTempDelta", 15};


    // Session State
    Observable<uint32_t> sessionCycles{0};
    Observable<float> sessionConsumption{0};
    Observable<float> todayConsumption{0};
    Observable<float> yesterdayConsumption{0};

    DisplayDriver* display = nullptr;

    static DeviceState& instance();

private:
    DeviceState() = default;
};

#endif // DEVICESTATE_H
