#include "ui/UISetup.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include <utility>

UISetup::UISetup(
    ScreenManager& screenManager,
    HeaterController& heater,
    DisplayDriver* displayDriver,
    InputManager& inputManager
)
    : screenManager(screenManager),
      heater(heater),
      displayDriver(displayDriver),
      inputManager(inputManager)
{}

void UISetup::setup() {
    setupScreens();
    setupMainMenu();
}

void UISetup::setupScreens() {
    // Create screens
    fireScreen = std::make_unique<FireScreen>(heater);
    screensaverScreen = std::make_unique<ScreensaverScreen>(DeviceState::instance().sleepTimeout.get(), displayDriver, [this]() {
        screenManager.setScreen(fireScreen.get());
    });
    otaUpdateScreen = std::make_unique<OtaUpdateScreen>(displayDriver);
    timezoneScreen = std::make_unique<TimezoneScreen>(&screenManager);
    startupScreen = std::make_unique<StartupScreen>([this]() {
        screenManager.setScreen(fireScreen.get(), ScreenTransition::FADE);
    });

    // Register screens
    screenManager.registerScreen(ScreenType::STARTUP, startupScreen.get());
    screenManager.registerScreen(ScreenType::FIRE, fireScreen.get());
    screenManager.registerScreen(ScreenType::TIMEZONE, timezoneScreen.get());
    screenManager.registerScreen(ScreenType::SCREENSAVER, screensaverScreen.get());
    screenManager.registerScreen(ScreenType::OTA_UPDATE, otaUpdateScreen.get());
}

void UISetup::setupMainMenu() {
    auto& state = DeviceState::instance();

    auto menuItems = MenuBuilder()
        .addObservableRange("HC Temp", state.heatCycleTempDelta, static_cast<uint8_t>(0), static_cast<uint8_t>(40), static_cast<uint8_t>(1), "°C")

         .addAction("Heater", [&]() {
             screenManager.switchScreen(ScreenType::HEAT_MENU, ScreenTransition::FADE);
         })
        .addObservableToggle("ZVS Debug", state.zvsDebug)
        .addHeadline("DISPLAY")

        .addObservableRange("Brightness", state.brightness,
                           static_cast<uint8_t>(20),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")

        .addObservableToggle("Dark Mode", state.darkMode)

        .addObservableToggle("Smart Mode", state.smart)

        .addObservableToggle("Center Heat", state.enableCenterButtonForHeating)

        .addAction("Timezone", [this]() {
            screenManager.setScreen(timezoneScreen.get(), ScreenTransition::FADE);
        })

        .addObservableRangeMs("Sleep Timeout", state.sleepTimeout,
                             60000,    // 1 Minute min
                             1800000,  // 30 Minuten max
                             60000)    // 1 Minute step

        .addAction("FACTORY RESET", [this]() {
            nvs_flash_erase();
            esp_restart();
        })

        .addAction(BUILD_TIME, [this]() {
            esp_restart();
        })

        .build();

    this->mainMenuScreen = std::make_unique<GenericMenuScreen>("SETTINGS", std::move(menuItems));
    screenManager.registerScreen(ScreenType::MAIN_MENU, this->mainMenuScreen.get());


    auto heaterMenuItems = MenuBuilder()
         .addHeadline("ZVS ADVANCED")
            .addObservableRange("HC Temp", state.heatCycleTempDelta, static_cast<uint8_t>(0), static_cast<uint8_t>(40), static_cast<uint8_t>(1), "°C")
            .addObservableRangeMs("Duty Period", state.zvsDutyCyclePeriodMs, 200, 2000, 100, true)
            .addObservableRangeMs("Temp Sensor", state.tempSensorOffTime, 50, 220, 10, true)
         
         .build();

    this->heaterMenuScreen = std::make_unique<GenericMenuScreen>("SETTINGS", std::move(heaterMenuItems));
    screenManager.registerScreen(ScreenType::HEAT_MENU, this->heaterMenuScreen.get());

    // Setup timezone exit callback
    timezoneScreen->setCallback([this]() {
        screenManager.setScreen(this->mainMenuScreen.get(), ScreenTransition::FADE);
    });
}
