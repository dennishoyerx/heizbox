// src/screens/ScreensaverScreen.cpp
#include "ui/base/Screen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/base/ScreenManager.h"
#include "StateManager.h"
#include <TFT_eSPI.h>

ScreensaverScreen::ScreensaverScreen(unsigned long timeout, DisplayDriver* dm, std::function<void()> callback)
    : displayManager(dm) {
    startTimeout(timeout, callback);
}

void ScreensaverScreen::draw(DisplayDriver& display) {
    display.clear(TFT_BLACK);

    // Display time
    String time = Utils::getFormattedTime();
    int16_t timeWidth = display.getTextWidth(time.c_str(), 3);
    int16_t timeX = (display.getTFTWidth() - timeWidth) / 2 + 25;
    display.drawText(timeX, 90, time.c_str(), TFT_WHITE, 3);

    // Display date
    String date = Utils::getFormattedDate();
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
    this->resetTimeout();
    displayManager->setBrightness(100);

    if (event.type == PRESS) {
        // FIRE-Button nicht sofort Heizen starten
        if (this->onTimeoutCallback_) {
            this->onTimeoutCallback_();
        }
    }
}
