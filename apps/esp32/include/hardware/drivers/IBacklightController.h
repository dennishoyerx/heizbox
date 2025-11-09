// apps/esp32/include/hardware/drivers/IBacklightController.h
#ifndef IBACKLIGHTCONTROLLER_H
#define IBACKLIGHTCONTROLLER_H

#include <cstdint>

class IBacklightController {
public:
    virtual ~IBacklightController() = default;
    virtual void init() = 0;
    virtual void setBrightness(uint8_t level) = 0;
    virtual uint8_t getBrightness() const = 0;
};

#endif // IBACKLIGHTCONTROLLER_H
