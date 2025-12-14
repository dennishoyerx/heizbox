#pragma once

#include "DisplayDriver.h"
#include "hardware/input/InputHandler.h"
#include "hardware/input/InputManager.h"
#include "ui/base/ScreenManager.h"
#include "ui/base/UI.h"
#include "ui/Screens.h"


class DeviceUI {
public:
    DeviceUI(HeaterController& heater);
    void init();
    void update();

    DisplayDriver* getDisplay();
    ScreenManager* getScreenManager();
    InputHandler* getInputHandler();

    void switchScreen(ScreenType type, ScreenTransition transition = ScreenTransition::NONE) {
        screenManager.switchScreen(type, transition);
    }


private:
    std::unique_ptr<DisplayDriver> display;
    InputManager input;
    ScreenManager screenManager;
    Screens screens;
    std::unique_ptr<InputHandler> inputHandler;
};