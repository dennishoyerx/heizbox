#include "StartupScreen.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "StatusBar.h"
#include "ScreenType.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "bitmaps.h" // Include the bitmaps header

ScreenType StartupScreen::getType() const {
    return ScreenType::STARTUP;
}

StartupScreen::StartupScreen() : startTime(millis()), animationComplete(false) {
}

void StartupScreen::draw(DisplayManager& display) {
    display.clear(ST77XX_BLACK);

    // Assuming catBitmap is 24x48 pixels
    int16_t bitmapWidth = 24;
    int16_t bitmapHeight = 48;
    int16_t x = (display.getTFTWidth() - bitmapWidth) / 2;
    int16_t y = (display.getTFTHeight() - bitmapHeight) / 2;

    display.drawBitmap(x, y, catBitmap, bitmapWidth, bitmapHeight, ST77XX_WHITE);
}

void StartupScreen::update() {
    if (!animationComplete && (millis() - startTime > 3000)) {
        animationComplete = true;
        if (onAnimationCompleteCallback) {
            onAnimationCompleteCallback(); // Call the callback when animation is complete
        }
    }
}

void StartupScreen::handleInput(InputEvent event) {
    // Skip startup on any input
    if (event.type == PRESS) {
        animationComplete = true;
        if (onAnimationCompleteCallback) {
            onAnimationCompleteCallback(); // Call the callback on input as well
        }
    }
}

void StartupScreen::setOnAnimationCompleteCallback(std::function<void()> callback) {
    onAnimationCompleteCallback = callback;
}