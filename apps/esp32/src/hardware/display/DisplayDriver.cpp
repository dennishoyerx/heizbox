#include <memory>
#include "hardware/display/DisplayDriver.h"
#include "ui/base/ScreenManager.h"
#include "ui/ColorPalette.h"
#include "hardware/drivers/TFT_eSPI_Driver.h"

DisplayDriver::DisplayDriver(std::unique_ptr<ITFTDriver> tftDriver,
                               std::unique_ptr<IBacklightController> backlightController)
    : tft(std::move(tftDriver)),
      backlight(std::move(backlightController)),
      screenManager(nullptr) {
}

DisplayDriver::~DisplayDriver() {
}

void DisplayDriver::init(ScreenManager* mgr) {
    screenManager = mgr;
    tft->init();
    tft->setRotation(1);
    backlight->init();

    tft->fillScreen(heizbox_palette_dark[COLOR_BG]);

    TFT_eSPI& tft_spi = static_cast<TFT_eSPI_Driver*>(tft.get())->getTFT();
    
    Serial.println("📺 DisplayDriver initialized");
}


void DisplayDriver::clear() {
    tft->fillScreen(heizbox_palette_dark[COLOR_BG]);

    if (screenManager) {
        screenManager->setDirty();
    }
}

void DisplayDriver::setBrightness(uint8_t level) {
    backlight->setBrightness(level);
}

uint8_t DisplayDriver::getBrightness() const {
    return backlight->getBrightness();
}
