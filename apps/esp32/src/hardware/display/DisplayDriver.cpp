#include <memory>
#include "hardware/display/DisplayDriver.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/StatusBar.h"
#include "ui/ColorPalette.h"
#include "hardware/drivers/TFT_eSPI_Driver.h"
#include "ui/Font.h"

DisplayDriver::DisplayDriver(std::unique_ptr<ITFTDriver> tftDriver,
                               std::unique_ptr<IBacklightController> backlightController)
    : tft(std::move(tftDriver)),
      backlight(std::move(backlightController)),
      spriteRenderer(nullptr),
      statusBar(nullptr),
      screenManager(nullptr),
      darkMode(false) {
}

DisplayDriver::~DisplayDriver() {
    delete statusBar;
}

void DisplayDriver::init(ScreenManager* mgr) {
    screenManager = mgr;
    tft->init();
    tft->setRotation(1);
    backlight->init();

    tft->fillScreen(heizbox_palette[COLOR_ACCENT]);

    // Since TFT_eSPI_Driver returns a TFT_eSPI object, we can cast it here
    TFT_eSPI& tft_spi = static_cast<TFT_eSPI_Driver*>(tft.get())->getTFT();
    statusBar = new StatusBar(&tft_spi, DisplayConfig::WIDTH, DisplayConfig::STATUS_BAR_HEIGHT);
    
    spriteRenderer.reset(new SpriteRenderer(&tft_spi));
    reallocateSprites();

    Serial.println("📺 DisplayDriver initialized");
}

void DisplayDriver::reallocateSprites() {
    if (spriteRenderer) {
        spriteRenderer->reallocateSprites(1, 1, 4);
    }
    if (screenManager) {
        screenManager->setDirty();
    }
}

void DisplayDriver::clear() {
        tft->fillScreen(heizbox_palette[COLOR_ACCENT]);
        //renderStatusBar();

   /* if (spriteRenderer && spriteRenderer->isAllocated()) {
        spriteRenderer->getSprite().fillSprite(COLOR_ACCENT);
    } else {
        tft->fillScreen(heizbox_palette[COLOR_ACCENT]);
    }*/
    if (screenManager) {
        screenManager->setDirty();
    }
}

void DisplayDriver::render() {
    if (spriteRenderer) {
        spriteRenderer->push();
    }
}

void DisplayDriver::renderStatusBar() {
    if (statusBar) {
        statusBar->draw();
        statusBar->pushSprite(0, 0);
    }
}

void* DisplayDriver::getRenderer() {
    if (spriteRenderer && spriteRenderer->isAllocated()) {
        return &(spriteRenderer->getSprite());
    } else {
        return tft.get();
    }
}

TFT_eSprite& DisplayDriver::getSprite() {
    return spriteRenderer->getSprite();
}



void DisplayDriver::drawText(int16_t x, int16_t y, const char* text, uint8_t color, uint8_t size) {
    if (!text) return;

    if (spriteRenderer && spriteRenderer->isAllocated()) {
        TFT_eSprite& renderer = spriteRenderer->getSprite();
        setFont(size, renderer);
        renderer.setCursor(x, y);
        renderer.print(text);
    } else {
        // This is not ideal, as we are assuming the underlying type.
        TFT_eSPI& renderer = static_cast<TFT_eSPI_Driver*>(tft.get())->getTFT();
        setFont(size, renderer);
        renderer.setCursor(x, y);
        renderer.print(text);
    }
}

void DisplayDriver::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint8_t color) {
    uint16_t color16 = heizbox_palette[color];
    tft->drawBitmap(x, y, bitmap, w, h, color16);
}

void DisplayDriver::drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint8_t color) {
    uint16_t color16 = heizbox_palette[color];
    tft->drawXBitmap(x, y, bitmap, w, h, color16);
}

void DisplayDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    uint16_t color16 = heizbox_palette[color];
    tft->drawRect(x, y, w, h, color16);
}

void DisplayDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    uint16_t color16 = heizbox_palette[color];
    tft->fillRect(x, y, w, h, color16);
}

int DisplayDriver::getTextWidth(const char* text, uint8_t size) {
    if (!text) return 0;
    // Simplified
    return tft->textWidth(text);
}

void DisplayDriver::setBrightness(uint8_t level) {
    backlight->setBrightness(level);
}

uint8_t DisplayDriver::getBrightness() const {
    return backlight->getBrightness();
}

void DisplayDriver::setDarkMode(bool enabled) {
    if (darkMode != enabled) {
        darkMode = enabled;

        Serial.printf("🌓 Dark Mode set to: %s\n", darkMode ? "ON" : "OFF");

        if (screenManager) {
            screenManager->setDirty();
        }
    }
}
