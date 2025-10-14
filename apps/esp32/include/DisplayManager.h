#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "ClockManager.h"

// Forward declarations
class ScreenManager;
class StatusBar;

// Define missing colors
#define TFT_GRAY     0x7BEF

class DisplayManager {
    ScreenManager* screenManager = nullptr;
private:
    uint8_t brightness;
    bool darkMode;
    StatusBar* statusBar = nullptr;
    ClockManager* clock;
    TFT_eSprite sprTop;
    bool spriteAllocated = false;

public:
    DisplayManager(ClockManager* cm);
    ~DisplayManager(); // Destruktor hinzuf�gen
    void init(ScreenManager* mgr);
    void clear(uint16_t color = TFT_BLACK);
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1);
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color);
    void drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void render();
    void renderStatusBar();
    void setBrightness(uint8_t level);
    uint8_t getBrightness() const;
    void toggleDarkMode();
    bool isDarkMode() const;
    int getTFTWidth() const { return TFT_WIDTH; }
    int getTFTHeight() const { return TFT_HEIGHT; }
    int getTextWidth(const char* text, uint8_t size = 1);
    void freeSprites();
    void reallocateSprites();
};

#endif