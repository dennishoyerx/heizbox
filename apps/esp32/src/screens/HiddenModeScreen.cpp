// src/screens/HiddenModeScreen.cpp
#include "HiddenModeScreen.h"
#include "ScreenManager.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include <Adafruit_GFX.h>
#include <TFT_eSPI.h>

ScreenType HiddenModeScreen::getType() const {
    return ScreenType::HIDDEN_MODE;
}

HiddenModeScreen::HiddenModeScreen(DisplayManager* dm) : enterTime(millis()), fireButtonHeld(false), displayManager(dm) {
}

void HiddenModeScreen::draw(DisplayManager& display) {
    // Hidden mode - turn off display completely
    display.clear(TFT_BLACK);
    display.setBrightness(50); // Turn off backlight
}

void HiddenModeScreen::update() {
    // Check if fire button has been held long enough to exit
    if (fireButtonHeld && (millis() - enterTime > 3000)) {
        // Exit hidden mode
        displayManager->setBrightness(100); // Restore brightness
        // Transition would be handled by ScreenManager
    }
}

void HiddenModeScreen::handleInput(InputEvent event) {
    if (event.button == FIRE) {
        if (event.type == PRESS) {
            fireButtonHeld = true;
            enterTime = millis(); // Reset timer on press
        }
        else if (event.type == RELEASE) {
            fireButtonHeld = false;
        }
    }
}
