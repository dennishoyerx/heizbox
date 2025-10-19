// src/DisplayManager.cpp
#include "DisplayManager.h"
#include "ScreenManager.h"
#include "StatusBar.h"

// ============================================================================
// Constructor & Destructor
// ============================================================================

DisplayManager::DisplayManager(ClockManager* cm)
    : tft(),
      sprTop(&tft),
      statusBar(nullptr),
      clock(cm),
      screenManager(nullptr),
      brightness(DisplayConfig::BRIGHTNESS_DEFAULT),
      darkMode(true),
      spriteAllocated(false)
{
    renderState.reset();
}

DisplayManager::~DisplayManager() {
    freeSprites();
    delete statusBar;
}

// ============================================================================
// Initialization
// ============================================================================

void DisplayManager::init(ScreenManager* mgr) {
    screenManager = mgr;

    // Initialize TFT
    tft.init();
    tft.setRotation(1);

    // Load persistent settings
    loadSettings();

    // Initialize backlight PWM
    initBacklight();

    // Initialize StatusBar
    statusBar = new StatusBar(&tft, DisplayConfig::WIDTH, clock, DisplayConfig::STATUS_BAR_HEIGHT);

    // Allocate sprite buffer
    reallocateSprites();

    Serial.println("📺 DisplayManager initialized");
}

void DisplayManager::initBacklight() {
    ledcAttachPin(Config::Hardware::TFT_BL_PIN, DisplayConfig::PWM_CHANNEL);
    ledcSetup(DisplayConfig::PWM_CHANNEL, DisplayConfig::PWM_FREQUENCY, DisplayConfig::PWM_RESOLUTION);
    setBrightness(brightness);
}

void DisplayManager::loadSettings() {
    prefs.begin("display", true); // Read-only
    brightness = prefs.getUChar("brightness", DisplayConfig::BRIGHTNESS_DEFAULT);
    darkMode = prefs.getBool("darkMode", true);
    prefs.end();

    // Clamp brightness
    brightness = constrain(brightness, DisplayConfig::BRIGHTNESS_MIN, DisplayConfig::BRIGHTNESS_MAX);
}

void DisplayManager::saveSettings() {
    prefs.begin("display", false);
    prefs.putUChar("brightness", brightness);
    prefs.putBool("darkMode", darkMode);
    prefs.end();
}

// ============================================================================
// Sprite Management
// ============================================================================

void DisplayManager::freeSprites() {
    if (spriteAllocated) {
        sprTop.deleteSprite();
        spriteAllocated = false;
        Serial.println("🗑️ Sprite buffer freed");
    }
}

void DisplayManager::reallocateSprites() {
    // Ensure clean state
    freeSprites();

    // Allocate 8-bit sprite (256 colors, less memory than 16-bit)
    sprTop.setColorDepth(8);
    spriteAllocated = sprTop.createSprite(DisplayConfig::WIDTH, DisplayConfig::SPRITE_HEIGHT);

    if (spriteAllocated) {
        const size_t bytes = DisplayConfig::WIDTH * DisplayConfig::SPRITE_HEIGHT;
        Serial.printf("✅ Sprite allocated: %u bytes (%ux%u @ 8-bit)\n",
                      bytes, DisplayConfig::WIDTH, DisplayConfig::SPRITE_HEIGHT);
        sprTop.fillSprite(getBackgroundColor());
    } else {
        Serial.println("❌ Sprite allocation failed - fallback to direct rendering");
        Serial.printf("   Required: %u bytes\n", DisplayConfig::WIDTH * DisplayConfig::SPRITE_HEIGHT);
    }

    renderState.reset();
}

// ============================================================================
// Rendering Pipeline
// ============================================================================

void DisplayManager::clear(uint16_t color) {
    if (spriteAllocated) {
        sprTop.fillSprite(color);
    } else {
        tft.fillScreen(color);
    }
    renderState.reset();
}

void DisplayManager::render() {
    if (spriteAllocated) {
        sprTop.pushSprite(0, DisplayConfig::STATUS_BAR_HEIGHT);
    }
}

void DisplayManager::renderStatusBar() {
    if (statusBar) {
        statusBar->draw();
        statusBar->pushSprite(0, 0);
    }
}

// ============================================================================
// Drawing Primitives (Optimized)
// ============================================================================

template<typename T>
T& DisplayManager::getRenderer() {
    if (spriteAllocated) {
        return reinterpret_cast<T&>(sprTop);
    } else {
        return reinterpret_cast<T&>(tft);
    }
}

void DisplayManager::drawText(int16_t x, int16_t y, const char* text,
                               uint16_t color, uint8_t size) {
    if (!text) return;

    const uint16_t bgColor = getBackgroundColor();

    // Select renderer (sprite or direct)
    auto& renderer = spriteAllocated ? static_cast<TFT_eSPI&>(sprTop)
                                      : static_cast<TFT_eSPI&>(tft);

    // Optimize: Only update state if changed
    bool needsUpdate = false;

    if (renderState.textColor != color || renderState.bgColor != bgColor) {
        renderer.setTextColor(color, bgColor, false);
        renderState.textColor = color;
        renderState.bgColor = bgColor;
        needsUpdate = true;
    }

    if (renderState.textSize != size) {
        renderer.setTextSize(size);
        renderState.textSize = size;
        needsUpdate = true;
    }

    renderer.setCursor(x, y);
    renderer.print(text);

    if (screenManager && needsUpdate) {
        screenManager->setDirty();
    }
}

void DisplayManager::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                                 int16_t w, int16_t h, uint16_t color) {
    if (!bitmap) return;

    if (spriteAllocated) {
        sprTop.drawBitmap(x, y, bitmap, w, h, color);
    } else {
        tft.drawBitmap(x, y, bitmap, w, h, color);
    }
}

void DisplayManager::drawXBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                                  int16_t w, int16_t h, uint16_t color) {
    if (!bitmap) return;

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

int DisplayManager::getTextWidth(const char* text, uint8_t size) {
    if (!text) return 0;

    auto& renderer = spriteAllocated ? static_cast<TFT_eSPI&>(sprTop)
                                      : static_cast<TFT_eSPI&>(tft);

    // Temporär Größe setzen für Messung
    const uint8_t oldSize = renderState.textSize;
    if (oldSize != size) {
        renderer.setTextSize(size);
    }

    const int width = renderer.textWidth(text);

    // Zurücksetzen falls geändert
    if (oldSize != size) {
        renderer.setTextSize(oldSize);
    }

    return width;
}

// ============================================================================
// Settings Management
// ============================================================================

void DisplayManager::setBrightness(uint8_t level) {
    brightness = constrain(level, DisplayConfig::BRIGHTNESS_MIN, DisplayConfig::BRIGHTNESS_MAX);

    // Map brightness to PWM (50-100% -> 0-255)
    const uint8_t pwmValue = map(brightness,
                                  DisplayConfig::BRIGHTNESS_MIN,
                                  DisplayConfig::BRIGHTNESS_MAX,
                                  0, 255);

    ledcWrite(DisplayConfig::PWM_CHANNEL, pwmValue);

    // Save asynchronously (nur bei signifikanten Änderungen)
    static uint8_t lastSaved = brightness;
    if (abs(brightness - lastSaved) >= 10) {
        saveSettings();
        lastSaved = brightness;
        Serial.printf("💡 Brightness: %u%% (PWM: %u)\n", brightness, pwmValue);
    }
}

void DisplayManager::toggleDarkMode() {
    darkMode = !darkMode;
    saveSettings();
    renderState.reset(); // Force re-render mit neuen Farben

    Serial.printf("🌓 Dark Mode: %s\n", darkMode ? "ON" : "OFF");

    if (screenManager) {
        screenManager->setDirty();
    }
}
