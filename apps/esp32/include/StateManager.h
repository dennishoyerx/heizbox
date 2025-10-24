// include/StateManager.h
#ifndef STATEMANAGER_H
#define STATEMANAGER_H

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
    
    void set(T newValue) override {
        Observable<T>::set(newValue);
        save();
    }
    
    void load() {
        Preferences prefs;
        prefs.begin(namespace_, true);
        
        if (std::is_same<T, bool>::value) {
            this->setSilent(prefs.getBool(key_, this->get()));
        } else if (std::is_same<T, int>::value || std::is_same<T, uint8_t>::value) {
            this->setSilent(prefs.getInt(key_, this->get()));
        } else if (std::is_same<T, uint32_t>::value) {
            this->setSilent(prefs.getUInt(key_, this->get()));
        } else if (std::is_same<T, float>::value) {
            this->setSilent(prefs.getFloat(key_, this->get()));
        }
        
        prefs.end();
    }
    
    void save() {
        Preferences prefs;
        prefs.begin(namespace_, false);
        
        if (std::is_same<T, bool>::value) {
            prefs.putBool(key_, this->get());
        } else if (std::is_same<T, int>::value || std::is_same<T, uint8_t>::value) {
            prefs.putInt(key_, this->get());
        } else if (std::is_same<T, uint32_t>::value) {
            prefs.putUInt(key_, this->get());
        } else if (std::is_same<T, float>::value) {
            prefs.putFloat(key_, this->get());
        }
        
        prefs.end();
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
    PersistedObservable<uint32_t> autoStopTime{"heater", "autostop", 120000};
    PersistedObservable<uint32_t> currentCycle{"heater", "cycle", 1};
    
    // System Settings
    PersistedObservable<uint32_t> sleepTimeout{"system", "sleepTimeout", 600000};
    PersistedObservable<int32_t> timezoneOffset{"clock", "gmtOffset", 7200};
    
    // Runtime State (nicht persistiert)
    Observable<bool> isHeating{false};
    Observable<bool> isOnline{false};
    Observable<uint32_t> heatingStartTime{0};
    
    // Session State
    Observable<int> sessionClicks{0};
    Observable<int> sessionCaps{0};
    Observable<uint32_t> sessionCycles{0};
    
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

// ============================================================================
// State Bindings - UI ↔ State Synchronisation
// ============================================================================

class StateBinding {
public:
    // Display Brightness binden
    static void bindBrightness(DisplayManager* display) {
        auto& state = STATE;
        
        // Initial sync
        display->setBrightness(state.brightness.get());
        
        // Listen to state changes
        state.brightness.addListener([display](uint8_t value) {
            display->setBrightness(value);
        });
    }
    
    // Dark Mode binden
    static void bindDarkMode(DisplayManager* display) {
        auto& state = STATE;
        
        state.darkMode.addListener([display](bool enabled) {
            if (display->isDarkMode() != enabled) {
                display->toggleDarkMode();
            }
        });
    }
    
    // Timezone binden
    static void bindTimezone(ClockManager* clock) {
        auto& state = STATE;
        
        clock->setTimezoneOffset(state.timezoneOffset.get());
        
        state.timezoneOffset.addListener([clock](int32_t offset) {
            clock->setTimezoneOffset(offset);
        });
    }
    
    // Auto-Stop Time binden
    static void bindAutoStopTime(HeaterController* heater) {
        auto& state = STATE;
        
        heater->setAutoStopTime(state.autoStopTime.get());
        
        state.autoStopTime.addListener([heater](uint32_t time) {
            heater->setAutoStopTime(time);
        });
    }
    
    // Alle Bindings auf einmal
    static void bindAll(DisplayManager* display, ClockManager* clock, 
                       HeaterController* heater) {
        bindBrightness(display);
        bindDarkMode(display);
        bindTimezone(clock);
        bindAutoStopTime(heater);
    }
};

#endif // STATEMANAGER_H
