#include "StartupScreen.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "StatusBar.h"
#include "ScreenType.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

ScreenType StartupScreen::getType() const {
    return ScreenType::STARTUP;
}

StartupScreen::StartupScreen() : startTime(millis()), animationComplete(false) {
}

void StartupScreen::draw(DisplayManager& display) {
    display.clear(ST77XX_BLACK);

    unsigned long elapsed = millis() - startTime;
    float progress = min(1.0f, elapsed / 2000.0f); // 2 second animation

    // Draw logo or animation based on progress
    int width = progress * 100;
    display.drawText(70, 100, "STARTING...", ST77XX_WHITE, 2);
    display.drawRect(50, 130, 140, 20, ST77XX_WHITE);
    display.fillRect(50, 130, width, 20, ST77XX_GREEN);
}

void StartupScreen::update() {
    if (!animationComplete && (millis() - startTime > 2000)) {
        animationComplete = true;
        // Transition to main menu after animation
        // This would typically be handled by the ScreenManager
    }
}

void StartupScreen::handleInput(InputEvent event) {
    // Skip startup on any input
    if (event.type == PRESS) {
        animationComplete = true;
    }
}