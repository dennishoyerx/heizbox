// apps/esp32/include/hardware/display/DisplayDriver.h
#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include "hardware/drivers/ITFTDriver.h"
#include "hardware/display/BacklightController.h"
#include "hardware/display/SpriteRenderer.h"
#include <memory>

class ScreenManager;
class StatusBar;

namespace DisplayConfig {
    constexpr uint16_t WIDTH = 280;
    constexpr uint16_t HEIGHT = 240;
    constexpr uint16_t SPRITE_HEIGHT = 230;
    constexpr uint8_t STATUS_BAR_HEIGHT = 50;
}

class DisplayDriver {
public:
    explicit DisplayDriver(std::unique_ptr<ITFTDriver> tftDriver, 
                           std::unique_ptr<IBacklightController> backlightController);
    ~DisplayDriver();

    void init(ScreenManager* mgr);
    void clear();
    void render();
    void renderStatusBar();

    void* getRenderer();
    TFT_eSprite& getSprite();

    TFT_eSprite* createSprite(int16_t width, int16_t height);

    void drawText(int16_t x, int16_t y, const char* text, uint8_t color, uint8_t size = 1);
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint8_t color);
    void drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint8_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

    void setBrightness(uint8_t level);
    uint8_t getBrightness() const;
    void setDarkMode(bool enabled);
    bool isDarkMode() const { return darkMode; }

    int getTFTWidth() const { return DisplayConfig::WIDTH; }
    int getTFTHeight() const { return DisplayConfig::HEIGHT; }
    int getTextWidth(const char* text, uint8_t size = 1);

    void reallocateSprites();

private:
    std::unique_ptr<ITFTDriver> tft;
    std::unique_ptr<IBacklightController> backlight;
    std::unique_ptr<SpriteRenderer> spriteRenderer;
    
    StatusBar* statusBar;
    ScreenManager* screenManager;

    bool darkMode;
};

#endif // DISPLAYDRIVER_H
