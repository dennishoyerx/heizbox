// src/screens/ScreensaverScreen.cpp
#include "ScreensaverScreen.h"
#include "ScreenManager.h"
#include <TFT_eSPI.h>
#include "ScreenBase.h"

ScreensaverScreen::ScreensaverScreen(ClockManager& cm, unsigned long timeout, DisplayManager* dm)
    : TimedScreen(ScreenType::SCREENSAVER, timeout),
      clock(cm), displayManager(dm) {}

void ScreensaverScreen::draw(DisplayManager& display) {
    display.clear(TFT_BLACK);

    // Display time
    String time = clock.getFormattedTime();
    int16_t timeWidth = display.getTextWidth(time.c_str(), 3);
    int16_t timeX = (display.getTFTWidth() - timeWidth) / 2 + 25;
    display.drawText(timeX, 90, time.c_str(), TFT_WHITE, 3);

    // Display date
    String date = clock.getFormattedDate();
    int16_t dateWidth = display.getTextWidth(date.c_str(), 2);
    int16_t dateX = (display.getTFTWidth() - dateWidth) / 2 + 25;
    display.drawText(dateX, 130, date.c_str(), TFT_WHITE, 2);
}

void ScreensaverScreen::update() {
    // Base-Update (Timeout-Handling)
    TimedScreen::update();
    
    // Sekunden-Update für Uhr
    static unsigned long lastSecond = 0;
    if (millis() - lastSecond > 1000) {
        if (manager) manager->setDirty();
        lastSecond = millis();
    }
}

void ScreensaverScreen::handleInput(InputEvent event) {
    // Jede Eingabe weckt auf
    resetTimeout();
    displayManager->setBrightness(100);

    if (event.type == PRESS) {
        // FIRE-Button nicht sofort Heizen starten
        if (hasCallback()) {
            invokeCallback();
        }
    }
}
