#pragma once

#include "hardware/drivers/IBacklightController.h"
#include "core/Config.h"

namespace DisplayPWMConfig {
    constexpr uint8_t PWM_CHANNEL = 1;
    constexpr uint32_t PWM_FREQUENCY = 5000;
    constexpr uint8_t PWM_RESOLUTION = 8;
}

class BacklightController : public IBacklightController {
public:
    BacklightController();

    void init() override;
    void setBrightness(uint8_t level) override;
    uint8_t getBrightness() const override;

private:
    uint8_t brightness;
};

