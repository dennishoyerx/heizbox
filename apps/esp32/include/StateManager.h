// include/StateManager.h
#ifndef STATEMANAGER_H
#define STATEMANAGER_H

class DisplayManager;
class ClockManager;

#include <functional>
#include <vector>
#include <Preferences.h>
#include <type_traits>
#include "HeaterController.h"

// ============================================================================
// Observable Pattern - Automatische Benachrichtigungen
// ============================================================================

template<typename T>
class Observable {
public:
    using Listener = std::function<void(const T&)>;

    Observable() = default;
    explicit Observable(T initialValue) : value_(std::move(initialValue)) {}

    // Wert setzen und Listener benachrichtigen
    virtual void set(T newValue) {
        if (value_ != newValue) {
            value_ = std::move(newValue);
            notifyListeners();
        }
    }

    // Wert ohne Benachrichtigung setzen (für Initialisierung)
    void setSilent(T newValue) {
        value_ = std::move(newValue);
    }

    // Wert abrufen
    const T& get() const { return value_; }

    // Non-const get für Menu-Integration
    T& getRef() { return value_; }
    operator const T&() const { return value_; }

    // Listener registrieren
    void addListener(Listener listener) {
        listeners_.push_back(std::move(listener));
    }

    // Update mit Transformation
    template<typename Fn>
    void update(Fn&& fn) {
        T newValue = fn(value_);
        set(std::move(newValue));
    }

private:
    void notifyListeners() {
        for (auto& listener : listeners_) {
            listener(value_);
        }
    }

    T value_;
    std::vector<Listener> listeners_;
};

// ============================================================================
// Persisted Observable - Mit NVS-Backing
// ============================================================================

template<typename T>
class PersistedObservable : public Observable<T> {
public:
    PersistedObservable(const char* ns, const char* key, T defaultValue)
        : Observable<T>(defaultValue), namespace_(ns), key_(key) {
        load();
    }

    void set(T newValue) {
        Serial.printf("PersistedObservable::set() for %s::%s\n", namespace_, key_);
        Observable<T>::set(newValue);
        save();
    }

    void load() {
        Preferences prefs;
        prefs.begin(namespace_, true);

        T value;
        if (std::is_same<T, bool>::value) {
            value = prefs.getBool(key_, this->get());
        } else if (std::is_same<T, int>::value || std::is_same<T, uint8_t>::value) {
            value = prefs.getInt(key_, this->get());
        } else if (std::is_same<T, uint32_t>::value) {
            value = prefs.getUInt(key_, this->get());
        } else if (std::is_same<T, float>::value) {
            value = prefs.getFloat(key_, this->get());
        }

        this->setSilent(value);
        prefs.end();
        Serial.printf("PersistedObservable::load() for %s::%s -> loaded value %d\n", namespace_, key_, static_cast<int>(value));
    }

    void save() {
        Preferences prefs;
        prefs.begin(namespace_, false);

        T value = this->get();
        if (std::is_same<T, bool>::value) {
            prefs.putBool(key_, value);
        } else if (std::is_same<T, int>::value || std::is_same<T, uint8_t>::value) {
            prefs.putInt(key_, value);
        } else if (std::is_same<T, uint32_t>::value) {
            prefs.putUInt(key_, value);
        } else if (std::is_same<T, float>::value) {
            prefs.putFloat(key_, value);
        }

        prefs.end();
        Serial.printf("PersistedObservable::save() for %s::%s -> saved value %d\n", namespace_, key_, static_cast<int>(value));
    }

private:
    const char* namespace_;
    const char* key_;
};

// ============================================================================
// Device State - Zentrale State-Verwaltung
// ============================================================================

struct DeviceState {
    // Display Settings
    PersistedObservable<uint8_t> brightness{"display", "brightness", 100};
    PersistedObservable<bool> darkMode{"display", "darkMode", true};

    // Heater Settings
    PersistedObservable<uint32_t> autoStopTime{"heater", "autostop", 90000};
    PersistedObservable<uint32_t> currentCycle{"heater", "cycle", 1};

    // System Settings
    PersistedObservable<uint32_t> sleepTimeout{"system", "sleepTimeout", 600000};
    PersistedObservable<int32_t> timezoneOffset{"clock", "gmtOffset", 3600};

    // Runtime State (nicht persistiert)
    Observable<bool> isHeating{false};
    Observable<bool> isOnline{false};
    Observable<uint32_t> heatingStartTime{0};

    // Session State
    Observable<int> sessionClicks{0};
    Observable<int> sessionCaps{0};
    Observable<uint32_t> sessionCycles{0};
    Observable<double> sessionConsumption{0};
    Observable<double> todayConsumption{0};

    // Statistics
    PersistedObservable<uint32_t> totalCycles{"stats", "total_cycles", 0};
    PersistedObservable<uint32_t> totalDuration{"stats", "total_duration", 0};

    // Singleton Access
    static DeviceState& instance() {
        static DeviceState state;
        return state;
    }

private:
    DeviceState() = default;
};

// Globaler Zugriff (optional, für Convenience)
#define STATE DeviceState::instance()

#endif // STATEMANAGER_H
