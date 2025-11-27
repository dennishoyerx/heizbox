#include <memory>
#include "DisplayDriver.h"
#include "TFT_eSPI_Driver.h"

DisplayDriver::DisplayDriver(uint16_t width, uint16_t height,
                               std::unique_ptr<ITFTDriver> tftDriver,
                               std::unique_ptr<IBacklightController> backlightController)
    : width(width),
      height(height),
      tft(std::move(tftDriver)),
      backlight(std::move(backlightController)) {
}

DisplayDriver::~DisplayDriver() {}

void DisplayDriver::init() {
    tft->init();
    tft->setRotation(1);
    backlight->init();

    tft->fillScreen(backgroundColor);

    TFT_eSPI& tft_spi = static_cast<TFT_eSPI_Driver*>(tft.get())->getTFT();
    
    Serial.println("📺 DisplayDriver initialized");
}


void DisplayDriver::clear() {
    tft->fillScreen(backgroundColor);
}

void DisplayDriver::setBrightness(uint8_t level) {
    backlight->setBrightness(level);
}

uint8_t DisplayDriver::getBrightness() const {
    return backlight->getBrightness();
}
