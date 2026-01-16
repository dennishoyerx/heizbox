#pragma once

#include <functional>
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"

enum ButtonSources {
    PCF,
    ESP32_GPIO
};

class ButtonSource {
public:
    virtual bool isPressed(uint8_t index) = 0;
};


class Pcf8574ButtonSource : public ButtonSource {
public:
    Pcf8574ButtonSource(uint8_t addr) : address(addr) {}

    void begin() {
        //Wire.begin(InputConfig::PCF8574::SDA, InputConfig::PCF8574::SCL);
        // Set a sane clock for I2C (ESP32 TwoWire supports setClock)
        #if defined(TWO_WIRE_H) || defined(TWOWIRE_H)
        Wire.setClock(100000);
        #endif
    }

    void update();

    bool isPressed(uint8_t idx) override {
        // active LOW → invertieren
        return !(state & (1 << idx));
    }

private:
    uint8_t address;
    uint8_t state = 0xFF;
};
