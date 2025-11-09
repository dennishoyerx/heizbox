// apps/esp32/src/hardware/display/BacklightController.cpp
#include "hardware/display/BacklightController.h"
#include <Arduino.h>

BacklightController::BacklightController()
    : brightness(DisplayConfig::BRIGHTNESS_DEFAULT) {}

void BacklightController::init() {
    ledcAttachPin(HardwareConfig::TFT_BL_PIN, DisplayConfig::PWM_CHANNEL);
    ledcSetup(DisplayConfig::PWM_CHANNEL, DisplayConfig::PWM_FREQUENCY, DisplayConfig::PWM_RESOLUTION);
    setBrightness(brightness);
}

void BacklightController::setBrightness(uint8_t level) {
    brightness = constrain(level, DisplayConfig::BRIGHTNESS_MIN, DisplayConfig::BRIGHTNESS_MAX);

    const uint8_t pwmValue = map(brightness,
                                  DisplayConfig::BRIGHTNESS_MIN,
                                  DisplayConfig::BRIGHTNESS_MAX,
                                  0, 255);

    ledcWrite(DisplayConfig::PWM_CHANNEL, pwmValue);

    Serial.printf("💡 Brightness: %u%% (PWM: %u)\n", brightness, pwmValue);
}

uint8_t BacklightController::getBrightness() const {
    return brightness;
}
