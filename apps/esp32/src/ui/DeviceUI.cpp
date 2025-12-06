#include "ui/DeviceUI.h"

#include "BacklightController.h"
#include "TFT_eSPI_Driver.h"
#include "core/EventBus.h"
#include "heater/HeaterController.h"
#include "utils/Logger.h"

DeviceUI::DeviceUI(HeaterController& heater): 
    display(std::make_unique<DisplayDriver>(DisplayConfig::WIDTH, DisplayConfig::HEIGHT,
                                              std::make_unique<TFT_eSPI_Driver>(),
                                              std::make_unique<BacklightController>(HardwareConfig::TFT_BL_PIN))),
    screens(heater),
    screenManager(*display, input),
    inputHandler(std::make_unique<InputHandler>(screenManager)) {};

void DeviceUI::setup() {
    display->init();

    screens.setup(screenManager);
    screens.setupMenus(screenManager);

    screenManager.switchScreen(ScreenType::STARTUP);

    EventBus::instance().subscribe<HeaterStoppedData>(
        EventType::HEATER_STOPPED,
        [](const HeaterStoppedData& d){
            std::string msg = "Stopped: " + std::to_string(d.duration) +
                  " ms, started: " + std::to_string(d.startedAt) + "\n";
            logPrint("Heater", msg);
        }
    );

    input.setup();
    input.setCallback([this](InputEvent event) { inputHandler->handleInput(event); });
};

void DeviceUI::update() {
    input.update();
    screenManager.update();
    screenManager.draw();
};

ScreenManager* DeviceUI::getScreenManager() {
    return &screenManager;
}

DisplayDriver* DeviceUI::getDisplay() {
    return display.get();
}