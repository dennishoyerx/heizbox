#include "BacklightController.h"
#include <Arduino.h>

BacklightController::BacklightController(uint8_t pin)
    : brightness(DisplayPWMConfig::BRIGHTNESS_DEFAULT),
    pin(pin) {}

void BacklightController::init() {
    ledcAttachPin(pin, DisplayPWMConfig::PWM_CHANNEL);
    ledcSetup(DisplayPWMConfig::PWM_CHANNEL, DisplayPWMConfig::PWM_FREQUENCY, DisplayPWMConfig::PWM_RESOLUTION);
    setBrightness(brightness);
}

void BacklightController::setBrightness(uint8_t level) {
    brightness = constrain(level, DisplayPWMConfig::BRIGHTNESS_MIN, DisplayPWMConfig::BRIGHTNESS_MAX);

    const uint8_t pwmValue = map(brightness,
                                  DisplayPWMConfig::BRIGHTNESS_MIN,
                                  DisplayPWMConfig::BRIGHTNESS_MAX,
                                  0, 255);

    ledcWrite(DisplayPWMConfig::PWM_CHANNEL, pwmValue);

    Serial.printf("💡 Brightness: %u%% (PWM: %u)\n", brightness, pwmValue);
}

uint8_t BacklightController::getBrightness() const {
    return brightness;
}
