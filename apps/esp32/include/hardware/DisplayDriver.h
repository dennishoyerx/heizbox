// include/DisplayManager.h
#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include <TFT_eSPI.h>

#include <SPI.h>



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

class DisplayDriver {
public:
    explicit DisplayDriver();
    ~DisplayDriver();

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
    void setDarkMode(bool enabled);
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
    ScreenManager* screenManager;

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
            bgColor = 1;
        }
    } renderState;

    // Helper methods
    void loadSettings();
    void initBacklight();
    uint16_t getBackgroundColor();

    // Renderer selection (Sprite vs Direct)
    TFT_eSPI& getRenderer();
    template<typename T>
    T& getRenderer();
};

extern DisplayDriver Display;
#endif
