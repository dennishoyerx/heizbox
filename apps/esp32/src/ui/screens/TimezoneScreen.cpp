#include "ui/screens/TimezoneScreen.h"
#include "hardware/display/DisplayDriver.h"
#include "core/DeviceState.h"
#include "ui/ColorPalette.h"
#include <TFT_eSPI.h>
#include "StateManager.h"

TimezoneScreen::TimezoneScreen(ScreenManager* sm)
    : screenManager(sm), timezoneOffsetHours(0) {}

ScreenType TimezoneScreen::getType() const {
    return ScreenType::TIMEZONE;
}

void TimezoneScreen::onEnter() {
    // Load current offset when screen is entered
    timezoneOffsetHours = DeviceState::instance().timezoneOffset.get() / 3600;
    if (screenManager) screenManager->setDirty();
}

void TimezoneScreen::draw() {
    /*display.clear();

    // Draw title
    display.drawText(50, 10, "TIMEZONE", COLOR_TEXT_PRIMARY, 2);

    // Draw current offset
    char buffer[20];
    sprintf(buffer, "UTC %+d", timezoneOffsetHours);
    display.drawText(60, 80, buffer, COLOR_TEXT_PRIMARY, 3);

    // Draw instructions
    display.drawText(30, 160, "UP/DOWN: Adjust", COLOR_TEXT_SECONDARY, 1);
    display.drawText(30, 180, "CENTER: Save & Exit", COLOR_TEXT_SECONDARY, 1);*/
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
