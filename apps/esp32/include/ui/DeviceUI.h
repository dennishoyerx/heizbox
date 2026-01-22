#pragma once

#include "DisplayDriver.h"
#include "driver/input/InputHandler.h"
#include "driver/input/InputManager.h"
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

    void switchScreen(ScreenType type) {
        screenManager.switchScreen(type);
    }


private:
    std::unique_ptr<DisplayDriver> display;
    InputManager input;
    ScreenManager screenManager;
    Screens screens;
    std::unique_ptr<InputHandler> inputHandler;
};