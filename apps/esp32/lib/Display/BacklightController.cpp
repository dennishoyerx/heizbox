#include "BacklightController.h"
#include <Arduino.h>

BacklightController::BacklightController(uint8_t pin)
    : brightness(DisplayPWMConfig::BRIGHTNESS_DEFAULT),
      pin(pin),
      pwmEnabled(false) {}

void BacklightController::init() {
    pinMode(pin, OUTPUT);
    setBrightness(brightness);
    
    Serial.printf("✅ Backlight initialized on GPIO%u (Hybrid mode)\n", pin);
}

void BacklightController::setBrightness(uint8_t level) {
    brightness = constrain(level, DisplayPWMConfig::BRIGHTNESS_MIN, DisplayPWMConfig::BRIGHTNESS_MAX);

    // Bei 100% Helligkeit: Direktes HIGH ohne PWM (volle Spannung)
    if (brightness >= 100) {
        disablePWM();
        digitalWrite(pin, HIGH);
        Serial.printf("💡 Brightness: 100%% (Digital HIGH - full voltage)\n");
        return;
    }
    
    // Bei 0%: Komplett aus
    if (brightness == 0) {
        disablePWM();
        digitalWrite(pin, LOW);
        Serial.printf("💡 Brightness: 0%% (OFF)\n");
        return;
    }
    
    // Nur bei Zwischenwerten: PWM verwenden
    enablePWM();
    
    const uint16_t maxPwmValue = (1 << DisplayPWMConfig::PWM_RESOLUTION) - 1;
    const uint16_t pwmValue = map(brightness,
                                   DisplayPWMConfig::BRIGHTNESS_MIN,
                                   DisplayPWMConfig::BRIGHTNESS_MAX,
                                   0, maxPwmValue);

    ledcWrite(DisplayPWMConfig::PWM_CHANNEL, pwmValue);
    Serial.printf("💡 Brightness: %u%% (PWM: %u/%u)\n", brightness, pwmValue, maxPwmValue);
}

void BacklightController::enablePWM() {
    if (!pwmEnabled) {
        ledcSetup(DisplayPWMConfig::PWM_CHANNEL, 
                  DisplayPWMConfig::PWM_FREQUENCY, 
                  DisplayPWMConfig::PWM_RESOLUTION);
        ledcAttachPin(pin, DisplayPWMConfig::PWM_CHANNEL);
        pwmEnabled = true;
    }
}

void BacklightController::disablePWM() {
    if (pwmEnabled) {
        ledcDetachPin(pin);
        pinMode(pin, OUTPUT);  // Zurück zu Digital-Modus
        pwmEnabled = false;
    }
}

uint8_t BacklightController::getBrightness() const {
    return brightness;
}