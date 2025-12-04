#include "ui/screens/StartupScreen.h"
#include "ui/ColorPalette.h"
#include "bitmaps.h"

StartupScreen::StartupScreen(std::function<void()> callback) {
    startAnimation(3000, nullptr, callback);
}

void StartupScreen::draw() {
    _ui->withSurface(280, 240, 0, 0, [this](RenderSurface& s) {
        // Zentrale Position berechnen
        constexpr int16_t bitmapWidth = 96;
        constexpr int16_t bitmapHeight = 96;
        const int16_t x = (s.width() - bitmapWidth) / 2;
        const int16_t y = (s.height() - bitmapHeight) / 2;

        // Bitmap zeichnen
        s.sprite->drawBitmap(x, y, image_cat_96, bitmapWidth, bitmapHeight, COLOR_TEXT_PRIMARY);


        // Build Info
        char short_date[6];
        snprintf(short_date, sizeof(short_date), "%.2s.%.2s", BUILD_DATE + 4, BUILD_DATE + 9);

        char short_time[6];
        memcpy(short_time, BUILD_TIME, 5);
        short_time[5] = '\0';

        s.text(20, 20, short_date);
        s.text(20, 40, short_time);

        // Optional: Progress-Bar
        const float progress = this->getProgress();
        if (progress < 1.0f) {
            const int16_t barWidth = 200;
            const int16_t barHeight = 4;
            const int16_t barX = (s.width() - barWidth) / 2;
            const int16_t barY = y + bitmapHeight + 20;

            s.sprite->drawRect(barX, barY, barWidth, barHeight, COLOR_TEXT_PRIMARY);
            s.sprite->fillRect(barX, barY, barWidth * progress, barHeight, COLOR_TEXT_PRIMARY);
        }
    });
}

// update() wird von AnimatedScreen gehandhabt

void StartupScreen::handleInput(InputEvent event) {
    // Beliebige Eingabe überspringt Animation
    if (event.type == PRESS) {
        this->forceCompleteAnimation();
    }
}
