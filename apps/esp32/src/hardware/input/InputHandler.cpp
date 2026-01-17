#include "hardware/input/InputHandler.h"
#include "utils/Logger.h"

InputHandler::InputHandler(ScreenManager& screenManager)
    : screenManager(screenManager)
{}

void InputHandler::handleInput(InputEvent event) {
    const char* typeStr = (event.type == PRESS) ? "PRESS" :
                     (event.type == PRESSED) ? "PRESSED" :
                     (event.type == RELEASE) ? "RELEASE" : 
                     (event.type == HOLD) ? "HOLD" :
                     (event.type == HOLD_ONCE) ? "HOLD_ONCE" :
                     (event.type == ROTARY_CW) ? "ROTARY_CW" :
                     (event.type == ROTARY_CCW) ? "ROTARY_CCW" : "UNKNOWN";

    const char* btnStr = (event.button == UP) ? "UP" :
                    (event.button == DOWN) ? "DOWN" :
                    (event.button == LEFT) ? "LEFT" :
                    (event.button == RIGHT) ? "RIGHT" :
                    (event.button == CENTER) ? "CENTER" :
                    (event.button == FIRE) ? "FIRE" :
                    (event.button == ROTARY_ENCODER) ? "ROTARY_ENCODER" : "UNKNOWN";

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

