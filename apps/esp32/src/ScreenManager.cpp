#include "ScreenManager.h"

ScreenManager::ScreenManager(DisplayManager& disp, InputManager& inp)
    : display(disp), input(inp), currentScreen(nullptr), dirty(true), currentScreenType(ScreenType::STARTUP) {
}

void ScreenManager::setScreen(Screen* newScreen) {
    currentScreen = newScreen;
    dirty = true;
    if (currentScreen) {
        currentScreenType = currentScreen->getType();
        currentScreen->setManager(this);
        display.clear();
        currentScreen->draw(display);
        display.render();
    }
}

void ScreenManager::update() {
    if (currentScreen) {
        currentScreen->update();
    }
}

void ScreenManager::draw() {
    if (dirty && currentScreen) {
        currentScreen->draw(display);
        display.render();
        dirty = false;
    }
}

void ScreenManager::handleInput(InputEvent event) {
    if (currentScreen) {
        currentScreen->handleInput(event);
    }
}