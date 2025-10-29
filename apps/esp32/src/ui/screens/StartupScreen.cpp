// src/screens/StartupScreen.cpp
#include "ui/base/Screen.h"
#include "ui/screens/StartupScreen.h"
#include "StateManager.h"
#include <TFT_eSPI.h>
#include "bitmaps.h"

// ============================================================================
// Modernisierte Implementation mit AnimatedScreen
// ============================================================================

StartupScreen::StartupScreen(std::function<void()> callback) {
    startAnimation(3000, nullptr, callback);
}

void StartupScreen::draw(DisplayDriver& display) {
    display.clear(TFT_BLACK);
    
    // Zentrale Position berechnen
    constexpr int16_t bitmapWidth = 96;
    constexpr int16_t bitmapHeight = 96;
    const int16_t x = (display.getTFTWidth() - bitmapWidth) / 2;
    const int16_t y = (display.getTFTHeight() - bitmapHeight) / 2;

    // Bitmap zeichnen
    display.drawBitmap(x, y, image_cat_96, bitmapWidth, bitmapHeight, TFT_WHITE);
    
    // Optional: Progress-Bar
    const float progress = this->getProgress();
    if (progress < 1.0f) {
        const int16_t barWidth = 200;
        const int16_t barHeight = 4;
        const int16_t barX = (display.getTFTWidth() - barWidth) / 2;
        const int16_t barY = y + bitmapHeight + 20;
        
        display.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
        display.fillRect(barX, barY, barWidth * progress, barHeight, TFT_WHITE);
    }
}

// update() wird von AnimatedScreen gehandhabt

void StartupScreen::handleInput(InputEvent event) {
    // Beliebige Eingabe überspringt Animation
    if (event.type == PRESS) {
        this->forceCompleteAnimation();
    }
}
