#include "ui/DeviceUI.h"

#include "BacklightController.h"
#include "TFT_eSPI_Driver.h"
#include "core/EventBus.h"

DeviceUI::DeviceUI(HeaterController& heater): 
    display(std::make_unique<DisplayDriver>(DisplayConfig::WIDTH, DisplayConfig::HEIGHT,
                                              std::make_unique<TFT_eSPI_Driver>(),
                                              std::make_unique<BacklightController>(HardwareConfig::TFT_BL_PIN))),
    input(),
    screenManager(*display, input),
    inputHandler(std::make_unique<InputHandler>(screenManager)),
    uiSetup(std::make_unique<UISetup>(screenManager, heater, display.get(), input)) {};

void DeviceUI::setup() {
    display->init();
    uiSetup->setup();
    screenManager.switchScreen(ScreenType::STARTUP);

    input.init();
    input.setCallback([this](InputEvent event) { inputHandler->handleInput(event); });

    EventBus::instance().subscribe(EventType::OTA_UPDATE_STARTED, [this](const Event& event) {
        switchScreen(ScreenType::OTA_UPDATE, ScreenTransition::FADE);
    });
    EventBus::instance().subscribe(EventType::OTA_UPDATE_FINISHED, [this](const Event& event) {
        switchScreen(ScreenType::FIRE, ScreenTransition::FADE);
    });
    EventBus::instance().subscribe(EventType::OTA_UPDATE_FAILED, [this](const Event& event) {
        switchScreen(ScreenType::FIRE, ScreenTransition::FADE);
    });
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