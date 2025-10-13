#include "TimezoneScreen.h"
#include "MainMenuScreen.h"

// Define missing color
#define ST77XX_GRAY 0x7BEF

TimezoneScreen::TimezoneScreen(ClockManager& cm, ScreenManager* sm)
    : clockManager(cm), screenManager(sm), mainMenuScreen(nullptr), timezoneOffsetHours(0) {
}

ScreenType TimezoneScreen::getType() const {
    return ScreenType::TIMEZONE;
}

void TimezoneScreen::onEnter() {
    // Load current offset when screen is entered
    timezoneOffsetHours = clockManager.getTimezoneOffset() / 3600;
    if (screenManager) screenManager->setDirty();
}

void TimezoneScreen::draw(DisplayManager& display) {
    display.clear(ST77XX_BLACK);

    // Draw title
    display.drawText(50, 10, "TIMEZONE", ST77XX_WHITE, 2);

    // Draw current offset
    char buffer[20];
    sprintf(buffer, "UTC %+d", timezoneOffsetHours);
    display.drawText(60, 80, buffer, ST77XX_WHITE, 3);

    // Draw instructions
    display.drawText(30, 160, "UP/DOWN: Adjust", ST77XX_GRAY, 1);
    display.drawText(30, 180, "CENTER: Save & Exit", ST77XX_GRAY, 1);
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
            clockManager.setTimezoneOffset(timezoneOffsetHours * 3600);
            if (exitCallback) {
                exitCallback();
            }
            break;
        default:
            break;
        }
    }
}

void TimezoneScreen::setMainMenuScreen(Screen* screen) {
    mainMenuScreen = screen;
}

void TimezoneScreen::onExit(std::function<void()> callback) {
    exitCallback = callback;
}
