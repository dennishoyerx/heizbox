#include "StatsScreen.h"
#include "ScreenManager.h"
#include "StatusBar.h"
#include "ScreenType.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Define missing color
#define ST77XX_GRAY 0x7BEF

ScreenType StatsScreen::getType() const {
    return ScreenType::STATS;
}

StatsScreen::StatsScreen(StatsManager& sm) : stats(sm) {
}

void StatsScreen::draw(DisplayManager& display) {
    display.clear(ST77XX_BLACK);

    // Draw title
    display.drawText(80, 10, "STATISTICS", ST77XX_WHITE, 2);

    // Draw stats
    char buffer[50];

    sprintf(buffer, "Total: %lu cycles", stats.getTotalCycles());
    display.drawText(30, 50, buffer, ST77XX_WHITE, 2);

    sprintf(buffer, "Session: %lu cycles", stats.getSessionCycles());
    display.drawText(30, 80, buffer, ST77XX_WHITE, 2);

    sprintf(buffer, "Avg: %.1f seconds", stats.getAverageDuration());
    display.drawText(30, 110, buffer, ST77XX_WHITE, 2);

    // Draw instructions
    display.drawText(70, 180, "BACK: Hold CENTER", ST77XX_GRAY, 1);
}

void StatsScreen::update() {
    // Stats screen doesn't need continuous updates
}

void StatsScreen::handleInput(InputEvent event) {
    if (event.button == CENTER && event.type == HOLD) {
        // Return to main menu
        // This would be handled by ScreenManager
    }
}