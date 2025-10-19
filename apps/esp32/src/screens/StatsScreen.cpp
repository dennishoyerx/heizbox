// src/screens/StatsScreen.cpp
#include "StatsScreen.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include <TFT_eSPI.h>

#define TFT_GRAY 0x7BEF

StatsScreen::StatsScreen(StatsManager& sm) : stats(sm) {}

void StatsScreen::draw(DisplayManager& display) {
    display.clear(TFT_BLACK);

    centerText(display, 10, "STATISTICS", TFT_WHITE, 2);

    // Total cycles
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Total: %lu cycles", stats.getTotalCycles());
    display.drawText(30, 50, buffer, TFT_WHITE, 2);

    // Session cycles
    snprintf(buffer, sizeof(buffer), "Session: %lu cycles", stats.getSessionCycles());
    display.drawText(30, 80, buffer, TFT_WHITE, 2);

    // Average duration
    snprintf(buffer, sizeof(buffer), "Avg: %.1f seconds", stats.getAverageDuration());
    display.drawText(30, 110, buffer, TFT_WHITE, 2);

    // Instructions
    centerText(display, 180, "BACK: Hold CENTER", TFT_GRAY, 1);
}

void StatsScreen::update() {
    // No updates needed
}

void StatsScreen::handleInput(InputEvent event) {
    if (event.button == CENTER && event.type == HOLD) {
        // Will be handled by Device's global shortcuts
    }
}

ScreenType StatsScreen::getType() const {
    return ScreenType::STATS;
}
