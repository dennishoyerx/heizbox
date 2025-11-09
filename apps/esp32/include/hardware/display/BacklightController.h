// apps/esp32/include/hardware/display/BacklightController.h
#ifndef BACKLIGHTCONTROLLER_H
#define BACKLIGHTCONTROLLER_H

#include "hardware/drivers/IBacklightController.h"
#include "core/Config.h"

namespace DisplayConfig {
    constexpr uint8_t BRIGHTNESS_MIN = 20;
    constexpr uint8_t BRIGHTNESS_MAX = 100;
    constexpr uint8_t BRIGHTNESS_DEFAULT = 100;

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

#endif // BACKLIGHTCONTROLLER_H
