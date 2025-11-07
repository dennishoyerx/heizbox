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


    // Build Info
    char short_date[6];
    snprintf(short_date, sizeof(short_date), "%.2s.%.2s", BUILD_DATE + 4, BUILD_DATE + 9);

    char short_time[6];
    memcpy(short_time, BUILD_TIME, 5);
    short_time[5] = '\0';

    display.drawText(20, 20, short_date, TFT_WHITE, 1);
    display.drawText(20, 40, short_time, TFT_WHITE, 1);

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
