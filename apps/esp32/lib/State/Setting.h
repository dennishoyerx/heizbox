#pragma once

#include <Arduino.h>

template<typename T>
struct Setting {
    String ns;
    String key;
    T initialValue;
};

struct Settings {
    Setting<bool> isOnline{"device", "online", false};
};

template<typename T>
class Settingx {
public:
    Settingx() = default;
    explicit Settingx(T initialValue) : value_(std::move(initialValue)) {}

    // Wert setzen und Listener benachrichtigen
    virtual T set(T newValue) {
        if (value_ != newValue) {
            value_ = std::move(newValue);
        }
        return newValue;
    }
    
    // Wert abrufen
    const T& get() const { return value_; }

    // Non-const get für Menu-Integration
    T& getRef() { return value_; }
    operator const T&() const { return value_; }

    // Update mit Transformation
    template<typename Fn>
    T update(Fn&& fn) {
        T newValue = fn(value_);
        set(std::move(newValue));
        return newValue;
    }

private:
    T value_;
};