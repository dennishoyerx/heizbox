#include "MainMenuScreen.h"
#include "ScreenManager.h"
#include "DisplayManager.h"
#include "StatusBar.h"
#include "ScreenType.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Define missing color
#define ST77XX_GRAY 0x7BEF

ScreenType MainMenuScreen::getType() const {
    return ScreenType::MAIN_MENU;
}

MainMenuScreen::MainMenuScreen(DisplayManager* display, ScreenManager* screenManager)
    : selectedIndex(0), display(display), screenManager(screenManager) {
    // Initialize menu items
    items = {
        {"Brightness", [this]() { adjustBrightness(); }},
        {"AutoStop Time", [this]() { configureAutoStop(); }},
        {"Sleep Timeout", [this]() { configureSleepTimeout(); }},
        {"Hidden Mode", [this]() { enterHiddenMode(); }},
        {"Statistics", [this]() { /* Transition to stats screen */ }}
    };
}

void MainMenuScreen::draw(DisplayManager& display) {
    display.clear(ST77XX_BLACK);

    // Draw title
    display.drawText(70, 10, "MENU", ST77XX_WHITE, 2);

    // Draw menu items
    for (size_t i = 0; i < items.size(); i++) {
        if (i == selectedIndex) {
            display.drawText(30, 50 + i * 30, ">", ST77XX_WHITE, 2);
            display.drawText(50, 50 + i * 30, items[i].title, ST77XX_YELLOW, 2);
        }
        else {
            display.drawText(50, 50 + i * 30, items[i].title, ST77XX_WHITE, 2);
        }
    }

    // Draw instructions
    display.drawText(30, 200, "CENTER: Select", ST77XX_GRAY, 1);
}

void MainMenuScreen::update() {
    // Menu doesn't need continuous updates
}

void MainMenuScreen::handleInput(InputEvent event) {
    if (event.type == PRESS) {
        switch (event.button) {
        case UP:
            selectedIndex = (selectedIndex - 1 + items.size()) % items.size();
            screenManager->setDirty();
            break;
        case DOWN:
            selectedIndex = (selectedIndex + 1) % items.size();
            screenManager->setDirty();
            break;
        case CENTER:
            if (selectedIndex < items.size()) {
                items[selectedIndex].action();
				screenManager->setDirty();
			}
            break;
        default:
            break;
        }
    }
}

void MainMenuScreen::adjustBrightness() {
    const uint8_t brightnessLevels[] = {20, 40, 60, 80, 100};
    const int numLevels = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);

    uint8_t currentBrightness = display->getBrightness();
    int newIndex = -1;

    // Find the current brightness level in the array
    for (int i = 0; i < numLevels; ++i) {
        if (currentBrightness == brightnessLevels[i]) {
            newIndex = i;
            break;
        }
    }

    // If current brightness is not in our defined levels, start from the beginning
    if (newIndex == -1) {
        newIndex = 0;
    } else {
        newIndex = (newIndex + 1) % numLevels;
    }

    display->setBrightness(brightnessLevels[newIndex]);
}

void MainMenuScreen::configureAutoStop() {
    // This would typically open a submenu for setting auto-stop time
    // For simplicity, we'll just cycle through options
    // In a real implementation, this would interface with HeaterController
}

void MainMenuScreen::configureSleepTimeout() {
    // This would typically open a submenu for setting sleep timeout
}

void MainMenuScreen::enterHiddenMode() {
    // Transition to hidden mode screen
}