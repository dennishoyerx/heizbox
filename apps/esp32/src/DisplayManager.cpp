#include "DisplayManager.h"
#include "ScreenManager.h"
#include "StatusBar.h"
#include <TFT_eSPI.h>   // Use TFT_eSPI instead of Adafruit_ST7789
#include <SPI.h>
#include <Preferences.h>

Preferences prefs;

void saveBrightness(uint8_t brightness) {
    prefs.begin("display", false);
    prefs.putUChar("brightness", brightness);
    prefs.end();
}

uint8_t loadBrightness() {
    prefs.begin("display", true); // Read-only
    uint8_t b = prefs.getUChar("brightness", 100); // Default 100
    prefs.end();
    return b;
}

void saveDarkMode(bool darkMode) {
    prefs.begin("display", false);
    prefs.putBool("darkMode", darkMode);
    prefs.end();
}

bool loadDarkMode() {
    prefs.begin("display", true); // Read-only
    bool d = prefs.getBool("darkMode", true); // Default true
    prefs.end();
    return d;
}

#define SPRITE_HEIGHT 230  

// TFT_eSPI display instance (global)
TFT_eSPI tft = TFT_eSPI();  // TFT_eSPI handles pin config via User_Setup.h

DisplayManager::DisplayManager(ClockManager* cm)
    : brightness(100), clock(cm), sprTop(&tft) {
    darkMode = loadDarkMode();
}

DisplayManager::~DisplayManager() {
    delete statusBar; // Clean up allocated StatusBar
}

void DisplayManager::init(ScreenManager* mgr) {
    screenManager = mgr;
    statusBar = new StatusBar(&tft, 280, clock, STATUS_BAR_HEIGHT); // Initialize StatusBar with TFT_eSPI instance and width
    tft.init();
    tft.setRotation(1);

    // PWM für Backlight
    ledcAttachPin(TFT_BL, 1);     // Channel 1
    ledcSetup(1, 5000, 8);        // 5 kHz, 8 Bit
    setBrightness(loadBrightness());

    reallocateSprites();
}

void DisplayManager::freeSprites() {
    sprTop.deleteSprite();
}

void DisplayManager::reallocateSprites() {
    // Sprite Allocation
    sprTop.setColorDepth(8); // Use 8-bit color to save memory
    if (sprTop.createSprite(280, SPRITE_HEIGHT)) {
        spriteAllocated = true;
        sprTop.fillSprite(TFT_RED);
    } else {
        spriteAllocated = false;
        Serial.println("Top Sprite allocation failed");
    }
}

void DisplayManager::clear(uint16_t color) {
    if (spriteAllocated) {
        sprTop.fillSprite(darkMode ? TFT_BLACK : TFT_WHITE);
    } else {
        tft.fillScreen(darkMode ? TFT_BLACK : TFT_WHITE);
    }
}

void DisplayManager::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    uint16_t bg = darkMode ? TFT_BLACK : TFT_WHITE;
    if (spriteAllocated) {
        sprTop.setTextColor(color, bg, false);
        sprTop.setTextSize(size);
        sprTop.setCursor(x, y);
        sprTop.print(text);
    } else {
        tft.setTextColor(color, bg, false);
        tft.setTextSize(size);
        tft.setCursor(x, y);
        tft.print(text);
    }
    if (screenManager) screenManager->setDirty();
}

void DisplayManager::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color) {
    if (spriteAllocated) {
        sprTop.drawBitmap(x, y, bitmap, w, h, color);
    } else {
        tft.drawBitmap(x, y, bitmap, w, h, color);
    }
}

void DisplayManager::drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color) {
    if (spriteAllocated) {
        sprTop.drawXBitmap(x, y, bitmap, w, h, color);
    } else {
        tft.drawXBitmap(x, y, bitmap, w, h, color);
    }
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (spriteAllocated) {
        sprTop.drawRect(x, y, w, h, color);
    } else {
        tft.drawRect(x, y, w, h, color);
    }
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (spriteAllocated) {
        sprTop.fillRect(x, y, w, h, color);
    } else {
        tft.fillRect(x, y, w, h, color);
    }
}

void DisplayManager::render() {
    if (spriteAllocated) {
        sprTop.pushSprite(0, STATUS_BAR_HEIGHT);
    }
}

void DisplayManager::renderStatusBar() {
    if (statusBar) {
        statusBar->draw(); // Draw to its own sprite
        statusBar->pushSprite(0, 0); // Push the status bar sprite to the display
    }
}

void DisplayManager::setBrightness(uint8_t level) {
    brightness = constrain(level, 0, 100);
	saveBrightness(brightness);
    ledcWrite(1, map(brightness, 50, 100, 0, 255));
}

uint8_t DisplayManager::getBrightness() const {
    return brightness;
}

void DisplayManager::toggleDarkMode() {
    darkMode = !darkMode;
    saveDarkMode(darkMode);
}

bool DisplayManager::isDarkMode() const {
    return darkMode;
}

int DisplayManager::getTextWidth(const char* text, uint8_t size) {
    if (spriteAllocated) {
        sprTop.setTextSize(size);
        return sprTop.textWidth(text);
    } else {
        tft.setTextSize(size);
        return tft.textWidth(text);
    }
}
