// src/DisplayManager.cpp
#include "hardware/DisplayDriver.h"
#include "ui/base/ScreenManager.h"
#include "ui/components/StatusBar.h"
#include "core/Config.h"
#include "ui/ColorPalette.h"
#include <FS.h>
#include <LittleFS.h>
#include <cstdint>

//DisplayDriver Display(nullptr);

// ============================================================================
// Constructor & Destructor
// ============================================================================

DisplayDriver::DisplayDriver()
    : tft(),
      mainSprite(&tft),
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

    if (!spriteAllocated) {
        tft.fillScreen(heizbox_palette[COLOR_BG_DARK]);
    }

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
        mainSprite.deleteSprite();
        spriteAllocated = false;
        Serial.println("🗑️ Sprite buffer freed");
    }
}

void DisplayDriver::reallocateSprites() {
    freeSprites();

    mainSprite.setColorDepth(4);
    spriteAllocated = mainSprite.createSprite(DisplayConfig::WIDTH, DisplayConfig::SPRITE_HEIGHT);

    if (spriteAllocated) {
        
  mainSprite.createPalette(heizbox_palette, 16);
  

        /*uint16_t paletteRAM[140];
        memcpy_P(paletteRAM, heizbox_palette, sizeof(heizbox_palette));
        mainSprite.createPalette(paletteRAM, 140);*/

        mainSprite.fillSprite(9);

        const size_t bytes = DisplayConfig::WIDTH * DisplayConfig::SPRITE_HEIGHT;
        Serial.printf("✅ Sprite allocated with custom palette: %u bytes (%ux%u @8-bit)\n",
                      bytes, DisplayConfig::WIDTH, DisplayConfig::SPRITE_HEIGHT);
    } else {
        Serial.println("❌ Sprite allocation failed - fallback to direct rendering");
        Serial.printf("   Required: %u bytes\n", DisplayConfig::WIDTH * DisplayConfig::SPRITE_HEIGHT);
    }

    renderState.reset();
}


uint16_t DisplayDriver::getBackgroundColor() {
    return heizbox_palette[COLOR_BG_DARK];
}

// ============================================================================
// Rendering Pipeline
// ============================================================================

void DisplayDriver::clear() {
    if (spriteAllocated) {
        mainSprite.fillSprite(4);
    } else {
        tft.fillScreen(heizbox_palette[COLOR_BG_DARK]);
    }
    renderState.reset();
}


void DisplayDriver::render() {
    if (spriteAllocated) {
  for (uint8_t i = 0; i < 16; i++) {
      //mainSprite.fillRect(i * 5, 0, 5, 80, i); // Index = Palette
  }
        mainSprite.pushSprite(0, DisplayConfig::STATUS_BAR_HEIGHT);
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
        return reinterpret_cast<T&>(mainSprite);
    } else {
        return reinterpret_cast<T&>(tft);
    }
}

TFT_eSPI& DisplayDriver::getRenderer() {
    if (spriteAllocated) {
        return mainSprite;
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
                               uint8_t color, uint8_t size) {
    if (!text) return;

    auto& renderer = getRenderer();
    bool needsUpdate = false;

    setFont(size, renderer);

    if (spriteAllocated) {
        if (renderState.textColor != color || renderState.bgColor != COLOR_BG_DARK) {
            renderer.setTextColor(color, COLOR_TEXT_PRIMARY);
            renderState.textColor = color;
            renderState.bgColor = COLOR_BG_DARK;
            needsUpdate = true;
        }
    } else {
        uint16_t color16 = heizbox_palette[color];
        uint16_t bgColor16 = heizbox_palette[COLOR_BG_DARK];
        // This comparison is tricky because state is uint8_t. We'll just update.
        renderer.setTextColor(color16, bgColor16);
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
                                 int16_t w, int16_t h, uint8_t color) {
    if (!bitmap) return;
    uint16_t color16 = spriteAllocated ? color : heizbox_palette[color];
    getRenderer().drawBitmap(x, y, bitmap, w, h, color16);
}

void DisplayDriver::drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                                  int16_t w, int16_t h, uint8_t color) {
    if (!bitmap) return;
    uint16_t color16 = spriteAllocated ? color : heizbox_palette[color];
    getRenderer().drawXBitmap(x, y, bitmap, w, h, color16);
}

void DisplayDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    uint16_t color16 = spriteAllocated ? color : heizbox_palette[color];
    getRenderer().drawRect(x, y, w, h, color16);
}

void DisplayDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    uint16_t color16 = spriteAllocated ? color : heizbox_palette[color];
    getRenderer().fillRect(x, y, w, h, color16);
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
