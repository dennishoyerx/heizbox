// src/screens/MainMenuScreen.cpp
#include "MainMenuScreen.h"
#include "TimezoneScreen.h"
#include <TFT_eSPI.h>

#define TFT_GRAY 0x7BEF

MainMenuScreen::MainMenuScreen(DisplayManager* display, ScreenManager* screenManager)
    : selectedIndex(0), display(display), screenManager(screenManager),
      statsScreen(nullptr), timezoneScreen(nullptr)
{
    items = {
        {"Brightness", [this]() { adjustBrightness(); }},
        {"Dark Mode", [this]() { this->display->toggleDarkMode(); this->markDirty(); }},
        {"Cycle Type", [this]() { this->display->toggleDarkMode(); this->markDirty(); }},
        {"Sleep Timeout", [this]() { configureSleepTimeout(); }},
        {"Timezone", [this]() {
            if (this->timezoneScreen) {
                this->timezoneScreen->onEnter();
                this->screenManager->setScreen(this->timezoneScreen, ScreenTransition::FADE);
            }
        }},
        /*{"Statistics", [this]() {
            if (this->statsScreen) {
                this->screenManager->setScreen(this->statsScreen, ScreenTransition::FADE);
            }
            }},*/
    };
}

void MainMenuScreen::draw(DisplayManager& display) {
    display.clear(TFT_BLACK);

    // Title
    centerText(display, 10, "MENU", TFT_WHITE, 2);

    // Update dark mode label
    items[5].title = display.isDarkMode() ? "Dark Mode: On" : "Dark Mode: Off";

    // Menu items
    for (size_t i = 0; i < items.size(); i++) {
        const int16_t y = 50 + i * 30;

        if (i == selectedIndex) {
            display.drawText(30, y, ">", TFT_WHITE, 2);
            display.drawText(50, y, items[i].title.c_str(), TFT_YELLOW, 2);
        } else {
            display.drawText(50, y, items[i].title.c_str(), TFT_WHITE, 2);
        }
    }

    // Instructions
    display.drawText(30, 220, "CENTER: Select", TFT_GRAY, 1);
}

void MainMenuScreen::update() {
    // No continuous updates needed
}

void MainMenuScreen::handleInput(InputEvent event) {
    if (event.type != PRESS) return;

    switch (event.button) {
        case UP:
            selectedIndex = (selectedIndex == 0) ? items.size() - 1 : selectedIndex - 1;
            markDirty();
            break;

        case DOWN:
            selectedIndex = (selectedIndex + 1) % items.size();
            markDirty();
            break;

        case CENTER:
            if (selectedIndex < items.size() && items[selectedIndex].action) {
                items[selectedIndex].action();
                markDirty();
            }
            break;

        default:
            break;
    }
}

void MainMenuScreen::adjustBrightness() {
    static constexpr uint8_t levels[] = {20, 40, 60, 80, 100};
    static constexpr size_t numLevels = sizeof(levels) / sizeof(levels[0]);

    const uint8_t current = display->getBrightness();

    // Find current level
    size_t idx = 0;
    for (size_t i = 0; i < numLevels; i++) {
        if (current == levels[i]) {
            idx = i;
            break;
        }
    }

    // Next level
    idx = (idx + 1) % numLevels;
    display->setBrightness(levels[idx]);
    display->saveSettings();
}

void MainMenuScreen::configureAutoStop() {
    // TODO: Implement submenu
}

void MainMenuScreen::configureSleepTimeout() {
    // TODO: Implement submenu
}

void MainMenuScreen::enterHiddenMode() {
    // TODO: Implement
}

void MainMenuScreen::setStatsScreen(Screen* screen) {
    statsScreen = screen;
}

void MainMenuScreen::setTimezoneScreen(TimezoneScreen* screen) {
    timezoneScreen = screen;
}

ScreenType MainMenuScreen::getType() const {
    return ScreenType::MAIN_MENU;
}
