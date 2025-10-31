// src/DisplayManager.cpp
#include "hardware/DisplayDriver.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/StatusBar.h"
#include "core/Config.h"
#include <FS.h>
#include <LittleFS.h>
#include <cstdint>

//DisplayDriver Display(nullptr);

// ============================================================================
// Constructor & Destructor
// ============================================================================

DisplayDriver::DisplayDriver()
    : tft(),
      sprTop(&tft),
      statusBar(nullptr),
      screenManager(nullptr),
      brightness(DisplayConfig::BRIGHTNESS_DEFAULT),
      spriteAllocated(false)
{
    renderState.reset();
}

DisplayDriver::~DisplayDriver() {
    freeSprites();
    delete statusBar;
}

// ============================================================================
// Initialization
// ============================================================================

void DisplayDriver::init(ScreenManager* mgr) {
    screenManager = mgr;
    tft.init();
    tft.setRotation(1);

    //tft.loadFont("fonts/josefin_20.vlw", LittleFS);

    initBacklight();
    statusBar = new StatusBar(&tft, DisplayConfig::WIDTH, DisplayConfig::STATUS_BAR_HEIGHT);
    reallocateSprites();

    uint16_t orange = tft.color565(255, 107, 43);
    tft.fillScreen(orange);

    Serial.println("📺 DisplayDriver initialized");
}

void DisplayDriver::initBacklight() {
    ledcAttachPin(HardwareConfig::TFT_BL_PIN, DisplayConfig::PWM_CHANNEL);
    ledcSetup(DisplayConfig::PWM_CHANNEL, DisplayConfig::PWM_FREQUENCY, DisplayConfig::PWM_RESOLUTION);
    setBrightness(brightness);
}

// ============================================================================
// Sprite Management
// ============================================================================

void DisplayDriver::freeSprites() {
    if (spriteAllocated) {
        sprTop.deleteSprite();
        spriteAllocated = false;
        Serial.println("🗑️ Sprite buffer freed");
    }
}

void DisplayDriver::reallocateSprites() {
    freeSprites();

    sprTop.setColorDepth(8);
    spriteAllocated = sprTop.createSprite(DisplayConfig::WIDTH, DisplayConfig::SPRITE_HEIGHT);

    if (spriteAllocated) {
        uint16_t orange = tft.color565(255, 107, 43);
        sprTop.fillSprite(orange);

        const size_t bytes = DisplayConfig::WIDTH * DisplayConfig::SPRITE_HEIGHT;
        Serial.printf("✅ Sprite allocated: %u bytes (%ux%u @8-bit, bg #FF6B2B)\n",
                      bytes, DisplayConfig::WIDTH, DisplayConfig::SPRITE_HEIGHT);
    } else {
        Serial.println("❌ Sprite allocation failed - fallback to direct rendering");
        Serial.printf("   Required: %u bytes\n", DisplayConfig::WIDTH * DisplayConfig::SPRITE_HEIGHT);
    }

    renderState.reset();
}


uint16_t DisplayDriver::getBackgroundColor() {
    return tft.color565(255, 107, 43);
}

// ============================================================================
// Rendering Pipeline
// ============================================================================

void DisplayDriver::clear(uint16_t color) {
    uint16_t orange = tft.color565(255, 107, 43);

    if (spriteAllocated) {
        sprTop.fillSprite(orange);
    } else {
        tft.fillScreen(orange);
    }
    renderState.reset();
}


void DisplayDriver::render() {
    if (spriteAllocated) {
        sprTop.pushSprite(0, DisplayConfig::STATUS_BAR_HEIGHT);
    }
}

void DisplayDriver::renderStatusBar() {
    if (statusBar) {
        statusBar->draw();
        statusBar->pushSprite(0, 0);
    }
}

// ============================================================================
// Drawing Primitives (Optimized)
// ============================================================================

template<typename T>
T& DisplayDriver::getRenderer() {
    if (spriteAllocated) {
        return reinterpret_cast<T&>(sprTop);
    } else {
        return reinterpret_cast<T&>(tft);
    }
}

TFT_eSPI& DisplayDriver::getRenderer() {
    if (spriteAllocated) {
        return sprTop;
    } else {
        return tft;
    }
}

static void setFont(uint16_t size, TFT_eSPI& renderer) {
    switch (size) {
        case 1: renderer.setFreeFont(&FreeSans9pt7b); renderer.setTextSize(1);  break;
        case 2: renderer.setFreeFont(&FreeSans12pt7b); renderer.setTextSize(1); break;
        case 3: renderer.setFreeFont(&FreeSans18pt7b); renderer.setTextSize(1); break;
        case 4: renderer.setFreeFont(&FreeSans24pt7b); renderer.setTextSize(1); break;
        case 5: renderer.setFreeFont(&FreeSans18pt7b); renderer.setTextSize(2); break;
        case 6: renderer.setFreeFont(&FreeSans24pt7b); renderer.setTextSize(2); break;
        default: renderer.setFreeFont(&FreeSans9pt7b); renderer.setTextSize(1);
    }
}

void DisplayDriver::drawText(int16_t x, int16_t y, const char* text,
                               uint16_t color, uint8_t size) {
    if (!text) return;

    const uint16_t bgColor = getBackgroundColor();

    auto& renderer = getRenderer();

    // Optimize: Only update state if changed
    bool needsUpdate = false;


    setFont(size, renderer);

    if (renderState.textColor != color || renderState.bgColor != bgColor) {
        renderer.setTextColor(color, bgColor, false);
        renderState.textColor = color;
        renderState.bgColor = bgColor;
        needsUpdate = true;
    }

    if (renderState.textSize != size) {
        renderState.textSize = size;
        needsUpdate = true;
    }

    renderer.setCursor(x, y);
    renderer.print(text);

    if (screenManager && needsUpdate) {
        screenManager->setDirty();
    }
}

void DisplayDriver::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                                 int16_t w, int16_t h, uint16_t color) {
    if (!bitmap) return;

    if (spriteAllocated) {
        sprTop.drawBitmap(x, y, bitmap, w, h, color);
    } else {
        tft.drawBitmap(x, y, bitmap, w, h, color);
    }
}

void DisplayDriver::drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                                  int16_t w, int16_t h, uint16_t color) {
    if (!bitmap) return;

    if (spriteAllocated) {
        sprTop.drawXBitmap(x, y, bitmap, w, h, color);
    } else {
        tft.drawXBitmap(x, y, bitmap, w, h, color);
    }
}

void DisplayDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (spriteAllocated) {
        sprTop.drawRect(x, y, w, h, color);
    } else {
        tft.drawRect(x, y, w, h, color);
    }
}

void DisplayDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (spriteAllocated) {
        sprTop.fillRect(x, y, w, h, color);
    } else {
        tft.fillRect(x, y, w, h, color);
    }
}

int DisplayDriver::getTextWidth(const char* text, uint8_t size) {
    if (!text) return 0;

    auto& renderer = getRenderer();

    setFont(size, renderer);

    return renderer.textWidth(text);
}


// ============================================================================
// Settings Management
// ============================================================================

void DisplayDriver::setBrightness(uint8_t level) {
    brightness = constrain(level, DisplayConfig::BRIGHTNESS_MIN, DisplayConfig::BRIGHTNESS_MAX);

    // Map brightness to PWM (50-100% -> 0-255)
    const uint8_t pwmValue = map(brightness,
                                  DisplayConfig::BRIGHTNESS_MIN,
                                  DisplayConfig::BRIGHTNESS_MAX,
                                  0, 255);

    ledcWrite(DisplayConfig::PWM_CHANNEL, pwmValue);

    Serial.printf("💡 Brightness: %u%% (PWM: %u)\n", brightness, pwmValue);
}

void DisplayDriver::setDarkMode(bool enabled) {
    if (darkMode != enabled) {
        darkMode = enabled;
        renderState.reset(); // Force re-render mit neuen Farben

        Serial.printf("🌓 Dark Mode set to: %s\n", darkMode ? "ON" : "OFF");

        if (screenManager) {
            screenManager->setDirty();
        }
    }
}
