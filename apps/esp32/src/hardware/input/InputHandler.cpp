// src/hardware/InputHandler.cpp
#include "hardware/input/InputHandler.h"
#include "utils/Logger.h"

InputHandler::InputHandler(ScreenManager& screenManager)
    : screenManager(screenManager)
{}

void InputHandler::handleInput(InputEvent event) {
    const char* typeStr = (event.type == PRESS) ? "PRESS" :
                         (event.type == RELEASE) ? "RELEASE" : "HOLD";
    const char* btnStr = (event.button == UP) ? "UP" :
                        (event.button == DOWN) ? "DOWN" :
                        (event.button == LEFT) ? "LEFT" :
                        (event.button == RIGHT) ? "RIGHT" :
                        (event.button == CENTER) ? "CENTER" : "FIRE";

    Serial.printf("Input: %s %s\n", btnStr, typeStr);

    // Handle global shortcuts
    if (handleGlobalShortcuts(event)) {
        return;
    }

    // Pass to screen manager
    screenManager.handleInput(event);
}

bool InputHandler::handleGlobalShortcuts(InputEvent event) {
    const ScreenType currentScreen = screenManager.getCurrentScreenType();

    // HOLD LEFT: Toggle between Fire <-> MainMenu
    if (event.button == CENTER && event.type == HOLD) {
        ScreenType targetType = (currentScreen == ScreenType::FIRE)
            ? ScreenType::MAIN_MENU : ScreenType::FIRE;
        screenManager.setScreen(screenManager.getScreen(targetType), ScreenTransition::FADE);
        return true;
    }

    return false;
}

