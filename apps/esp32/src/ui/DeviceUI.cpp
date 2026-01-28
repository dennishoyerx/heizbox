#include "ui/DeviceUI.h"

#include "BacklightController.h"
#include "TFT_eSPI_Driver.h"
#include "core/EventBus.h"
#include "core/DeviceState.h"
#include "heater/HeaterController.h"
#include "utils/Logger.h"
#include "SysModule.h"

DeviceUI::DeviceUI(HeaterController& heater): 
    display(std::make_unique<DisplayDriver>(DisplayConfig::WIDTH, DisplayConfig::HEIGHT,
                                              std::make_unique<TFT_eSPI_Driver>(),
                                              std::make_unique<BacklightController>(HardwareConfig::TFT_BL_PIN))),
    screens(heater),
    screenManager(*display, input),
    inputHandler(std::make_unique<InputHandler>(screenManager)) {};

void DeviceUI::init() {
    auto booted = SysModules::booting("ui");
    display->init();

    screens.setup(screenManager);
    screens.setupMenus(screenManager);
    screenManager.switchScreen(ScreenType::STARTUP);

    input.setup();
    input.setCallback([this](InputEvent event) { inputHandler->handleInput(event); });
    booted();
};

void DeviceUI::update() {
    input.update();
    screenManager.update();
    screenManager.draw();

    if (DeviceState::instance().display.idleTimeout == 0) return;

    static bool displayDimmed;
    if (!displayDimmed && input.getLastInputMs() >= DeviceState::instance().display.idleTimeout) {
        displayDimmed = true;
        display->setBrightness(DeviceState::instance().display.idleBrightness);
    } else if (displayDimmed) {
        displayDimmed = false;
        display->setBrightness(DeviceState::instance().display.brightness);
    }
};

ScreenManager* DeviceUI::getScreenManager() {
    return &screenManager;
}

DisplayDriver* DeviceUI::getDisplay() {
    return display.get();
}

InputHandler* DeviceUI::getInputHandler() {
    return inputHandler.get();
}