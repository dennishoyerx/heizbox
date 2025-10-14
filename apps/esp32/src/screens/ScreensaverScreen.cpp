#include "ScreensaverScreen.h"
#include "ScreenManager.h"
#include "DisplayManager.h"
#include "StatusBar.h"
#include "ScreenType.h"
#include <Adafruit_GFX.h>
#include <TFT_eSPI.h>

#define TFT_GRAY 0x7BEF

ScreenType ScreensaverScreen::getType() const {
    return ScreenType::SCREENSAVER;
}

ScreensaverScreen::ScreensaverScreen(ClockManager& cm, unsigned long timeout, DisplayManager* dm)
    : clock(cm), lastActivity(millis()), sleepTimeout(timeout), displayManager(dm) {
}

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
    display.drawText(dateX, 130, date.c_str(), TFT_GRAY, 2);
}

void ScreensaverScreen::update() {
    static unsigned long lastSecond = 0;
    if (millis() - lastSecond > 1000) {
        if (manager) manager->setDirty();
        lastSecond = millis();
    }
}

void ScreensaverScreen::handleInput(InputEvent event) {
    // Any input wakes up the screen
    lastActivity = millis();
    displayManager->setBrightness(100);

    // Specific inputs might exit screensaver
    if (event.type == PRESS) {
        if (exitCallback) {
            exitCallback();
        }
    }
}

void ScreensaverScreen::onExit(std::function<void()> callback) {
    exitCallback = callback;
}