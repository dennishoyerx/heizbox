#include "BacklightController.h"
#include <Arduino.h>

BacklightController::BacklightController(uint8_t pin)
    : brightness(DisplayPWMConfig::BRIGHTNESS_DEFAULT),
      pin(pin) {}

void BacklightController::init() {
    // WICHTIG: Erst Setup, DANN Pin attach!
    ledcSetup(DisplayPWMConfig::PWM_CHANNEL, DisplayPWMConfig::PWM_FREQUENCY, DisplayPWMConfig::PWM_RESOLUTION);
    ledcAttachPin(pin, DisplayPWMConfig::PWM_CHANNEL);
    
    // Initiale Helligkeit setzen
    setBrightness(brightness);
}

void BacklightController::setBrightness(uint8_t level) {
    brightness = constrain(level, DisplayPWMConfig::BRIGHTNESS_MIN, DisplayPWMConfig::BRIGHTNESS_MAX);

    // PWM-Wert basierend auf Resolution berechnen
    const uint16_t maxPwmValue = (1 << DisplayPWMConfig::PWM_RESOLUTION) - 1;
    const uint16_t pwmValue = map(brightness,
                                   DisplayPWMConfig::BRIGHTNESS_MIN,
                                   DisplayPWMConfig::BRIGHTNESS_MAX,
                                   0, maxPwmValue);

    // Moderne API: ledcWrite verwendet direkt den Pin
    ledcWrite(pin, pwmValue);

    Serial.printf("💡 Brightness: %u%% (PWM: %u/%u)\n", brightness, pwmValue, maxPwmValue);
}

uint8_t BacklightController::getBrightness() const {
    return brightness;
}