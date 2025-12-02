#pragma once

#include "DisplayDriver.h"
#include "hardware/input/InputHandler.h"
#include "hardware/input/InputManager.h"
#include "ui/base/ScreenManager.h"
#include "ui/UISetup.h"


class DeviceUI {
public:
    DeviceUI(HeaterController& heater);
    void setup();
    void update();

    DisplayDriver* getDisplay();
    ScreenManager* getScreenManager();

    void switchScreen(ScreenType type, ScreenTransition transition = ScreenTransition::NONE) {
        screenManager.switchScreen(type, transition);
    }


private:
    std::unique_ptr<DisplayDriver> display;
    InputManager input;
    ScreenManager screenManager;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<UISetup> uiSetup;
};