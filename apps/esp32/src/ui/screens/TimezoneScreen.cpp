// src/screens/TimezoneScreen.cpp
#include "ui/screens/TimezoneScreen.h"
#include <TFT_eSPI.h>
#include "StateManager.h"

// Define missing color
#define TFT_GRAY 0x7BEF

TimezoneScreen::TimezoneScreen(ClockManager& cm, ScreenManager* sm)
    : clockManager(cm), screenManager(sm), timezoneOffsetHours(0) {}

ScreenType TimezoneScreen::getType() const {
    return ScreenType::TIMEZONE;
}

void TimezoneScreen::onEnter() {
    // Load current offset when screen is entered
    DeviceState::instance().timezoneOffset.get() / 3600;
    if (screenManager) screenManager->setDirty();
}

void TimezoneScreen::draw(DisplayDriver& display) {
    display.clear(TFT_BLACK);

    // Draw title
    display.drawText(50, 10, "TIMEZONE", TFT_WHITE, 2);

    // Draw current offset
    char buffer[20];
    sprintf(buffer, "UTC %+d", timezoneOffsetHours);
    display.drawText(60, 80, buffer, TFT_WHITE, 3);

    // Draw instructions
    display.drawText(30, 160, "UP/DOWN: Adjust", TFT_GRAY, 1);
    display.drawText(30, 180, "CENTER: Save & Exit", TFT_GRAY, 1);
}

void TimezoneScreen::update() {
    // No continuous update needed
}

void TimezoneScreen::handleInput(InputEvent event) {
    if (event.type == PRESS) {
        switch (event.button) {
        case UP:
            timezoneOffsetHours++;
            if (timezoneOffsetHours > 14) timezoneOffsetHours = -12;
            if (screenManager) screenManager->setDirty();
            break;
        case DOWN:
            timezoneOffsetHours--;
            if (timezoneOffsetHours < -12) timezoneOffsetHours = 14;
            if (screenManager) screenManager->setDirty();
            break;
        case CENTER:
            DeviceState::instance().timezoneOffset.set(timezoneOffsetHours * 3600);
            if (hasCallback()) {
                invokeCallback();
            }
            break;
        default:
            break;
        }
    }
}
