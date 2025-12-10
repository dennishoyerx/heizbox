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
    if (debug) logPrint("Input", "%s %s\n", btnStr, typeStr);

    // Handle global shortcuts
    if (handleGlobalShortcuts(event)) {
        return;
    }

    // Pass to screen manager
    screenManager.handleInput(event);
}

bool InputHandler::handleGlobalShortcuts(InputEvent event) {
    const ScreenType currentScreen = screenManager.getCurrentScreenType();

    if (event.button == CENTER && event.type == HOLD_ONCE) {
        ScreenType targetType = (currentScreen == ScreenType::FIRE)
            ? ScreenType::MAIN_MENU : ScreenType::FIRE;
        screenManager.switchScreen(targetType, ScreenTransition::FADE);
        return true;
    }

    return false;
}

