#pragma once

#include "IBacklightController.h"

namespace DisplayPWMConfig {
    constexpr uint8_t PWM_CHANNEL = 1;
    constexpr uint32_t PWM_FREQUENCY = 500;
    constexpr uint8_t PWM_RESOLUTION = 8;
    constexpr uint8_t BRIGHTNESS_MIN = 20;
    constexpr uint8_t BRIGHTNESS_MAX = 100;
    constexpr uint8_t BRIGHTNESS_DEFAULT = 100;
}

class BacklightController : public IBacklightController {
public:
    BacklightController(uint8_t pin);

    void init() override;
    void setBrightness(uint8_t level) override;
    uint8_t getBrightness() const override;

private:
    uint8_t brightness;
    uint8_t pin;
};

