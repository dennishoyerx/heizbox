// src/ui/UISetup.cpp
#include "ui/UISetup.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include <utility> // For std::move and std::make_unique

UISetup::UISetup(
    ScreenManager& screenManager,
    HeaterController& heater,
    DisplayDriver* displayDriver,
    StatsManager& statsManager,
    InputManager& inputManager,
    TempSensor* tempSensor
)
    : screenManager(screenManager),
      heater(heater),
      displayDriver(displayDriver),
      statsManager(statsManager),
      inputManager(inputManager),
      tempSensor(tempSensor) // Initialize TempSensor
{}

void UISetup::setupScreens() {
    // Create screens
    fireScreen = std::make_unique<FireScreen>(heater, &screenManager, screensaverScreen.get(), &statsManager, tempSensor);
    hiddenModeScreen = std::make_unique<HiddenModeScreen>(displayDriver);
    screensaverScreen = std::make_unique<ScreensaverScreen>(DeviceState::instance().sleepTimeout.get(), displayDriver, [this]() {
        fireScreen->resetActivityTimer();
        screenManager.setScreen(fireScreen.get());
    });
    otaUpdateScreen = std::make_unique<OtaUpdateScreen>(displayDriver);
    statsScreen = std::make_unique<StatsScreen>(statsManager);
    timezoneScreen = std::make_unique<TimezoneScreen>(&screenManager);
    startupScreen = std::make_unique<StartupScreen>([this]() {
        screenManager.setScreen(fireScreen.get(), ScreenTransition::FADE);
    });

    // Register screens
    screenManager.registerScreen(ScreenType::STARTUP, startupScreen.get());
    screenManager.registerScreen(ScreenType::FIRE, fireScreen.get());
    screenManager.registerScreen(ScreenType::STATS, statsScreen.get());
    screenManager.registerScreen(ScreenType::TIMEZONE, timezoneScreen.get());
    screenManager.registerScreen(ScreenType::SCREENSAVER, screensaverScreen.get());
    screenManager.registerScreen(ScreenType::OTA_UPDATE, otaUpdateScreen.get());
    screenManager.registerScreen(ScreenType::HIDDEN_MODE, hiddenModeScreen.get());
}

void UISetup::setupMainMenu() {
    auto& state = DeviceState::instance();

    auto menuItems = MenuBuilder()
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

        .addAction("Stats", [this]() {
            screenManager.setScreen(statsScreen.get(), ScreenTransition::FADE);
        })

        .addObservableRangeMs("Sleep Timeout", state.sleepTimeout,
                             60000,    // 1 Minute min
                             1800000,  // 30 Minuten max
                             60000)    // 1 Minute step

        .addAction("Reset Session", [this]() {
            DeviceState::instance().sessionCycles.set(0);
            DeviceState::instance().sessionClicks.set(0);
            DeviceState::instance().sessionCaps.set(0);
        })

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

    // Setup timezone exit callback
    timezoneScreen->setCallback([this]() {
        screenManager.setScreen(this->mainMenuScreen.get(), ScreenTransition::FADE);
    });
}
