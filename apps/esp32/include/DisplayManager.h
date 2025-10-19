// include/DisplayManager.h
#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <Preferences.h>
#include "ClockManager.h"

// Forward declarations
class ScreenManager;
class StatusBar;

// Display-Konstanten
namespace DisplayConfig {
    constexpr uint16_t WIDTH = 280;
    constexpr uint16_t HEIGHT = 240;
    constexpr uint16_t SPRITE_HEIGHT = 230;
    constexpr uint8_t STATUS_BAR_HEIGHT = 50;

    constexpr uint8_t BRIGHTNESS_MIN = 20;
    constexpr uint8_t BRIGHTNESS_MAX = 100;
    constexpr uint8_t BRIGHTNESS_DEFAULT = 100;

    constexpr uint8_t PWM_CHANNEL = 1;
    constexpr uint32_t PWM_FREQUENCY = 5000;
    constexpr uint8_t PWM_RESOLUTION = 8;
}

// Custom colors
#define TFT_GRAY 0x7BEF

class DisplayManager {
public:
    explicit DisplayManager(ClockManager* cm);
    ~DisplayManager();

    // Lifecycle
    void init(ScreenManager* mgr);
    void clear(uint16_t color = TFT_BLACK);
    void render();
    void renderStatusBar();

    // Drawing primitives
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1);
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color);
    void drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // Settings
    void setBrightness(uint8_t level);
    uint8_t getBrightness() const { return brightness; }
    void toggleDarkMode();
    bool isDarkMode() const { return darkMode; }

    // Helpers
    int getTFTWidth() const { return DisplayConfig::WIDTH; }
    int getTFTHeight() const { return DisplayConfig::HEIGHT; }
    int getTextWidth(const char* text, uint8_t size = 1);

    // Sprite management
    void freeSprites();
    void reallocateSprites();

private:
    // Core components
    TFT_eSPI tft;
    TFT_eSprite sprTop;
    StatusBar* statusBar;
    ClockManager* clock;
    ScreenManager* screenManager;
    Preferences prefs;

    // State
    uint8_t brightness;
    bool darkMode;
    bool spriteAllocated;

    // Rendering state cache (eliminiert redundante Aufrufe)
    struct RenderState {
        uint8_t textSize;
        uint16_t textColor;
        uint16_t bgColor;

        void reset() {
            textSize = 0;
            textColor = 0xFFFF;
            bgColor = 0x0000;
        }
    } renderState;

    // Helper methods
    void loadSettings();
    void saveSettings();
    void initBacklight();
    uint16_t getBackgroundColor() const { return darkMode ? TFT_BLACK : TFT_WHITE; }

    // Renderer selection (Sprite vs Direct)
    template<typename T>
    T& getRenderer();
};

#endif
