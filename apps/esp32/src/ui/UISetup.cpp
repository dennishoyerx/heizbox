// src/ui/UISetup.cpp
#include "ui/UISetup.h"
#include "utils/Logger.h"
#include <utility> // For std::move and std::make_unique

UISetup::UISetup(
    ScreenManager& screenManager,
    FireScreen& fireScreen,
    HiddenModeScreen& hiddenModeScreen,
    ScreensaverScreen& screensaverScreen,
    OtaUpdateScreen& otaUpdateScreen,
    StatsScreen& statsScreen,
    TimezoneScreen& timezoneScreen,
    StartupScreen& startupScreen,
    std::function<void(int)> setCurrentCycleCallback
)
    : screenManager(screenManager),
      fireScreen(fireScreen),
      hiddenModeScreen(hiddenModeScreen),
      screensaverScreen(screensaverScreen),
      otaUpdateScreen(otaUpdateScreen),
      statsScreen(statsScreen),
      timezoneScreen(timezoneScreen),
      startupScreen(startupScreen),
      setCurrentCycleCallback(std::move(setCurrentCycleCallback))
{}

void UISetup::setupScreenRegistry() {
    // Alle Screens registrieren
    screenManager.registerScreen(ScreenType::STARTUP, &startupScreen);
    screenManager.registerScreen(ScreenType::FIRE, &fireScreen);
    screenManager.registerScreen(ScreenType::STATS, &statsScreen);
    screenManager.registerScreen(ScreenType::TIMEZONE, &timezoneScreen);
    screenManager.registerScreen(ScreenType::SCREENSAVER, &screensaverScreen);
    screenManager.registerScreen(ScreenType::OTA_UPDATE, &otaUpdateScreen);
    screenManager.registerScreen(ScreenType::HIDDEN_MODE, &hiddenModeScreen);

    // Main menu wird später registriert (nach Erstellung)
}

std::unique_ptr<GenericMenuScreen> UISetup::setupMainMenu() {
    auto& state = DeviceState::instance();

    auto menuItems = MenuBuilder()
        // Observable-Integration: Kein Cast, kein Callback nötig!
        .addObservableRange("Brightness", state.brightness,
                           static_cast<uint8_t>(20),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")

        .addObservableToggle("Dark Mode", state.darkMode)

        .addObservableToggle("Center Heat", state.enableCenterButtonForHeating)

        .addAction("Timezone", [this]() {
            screenManager.setScreen(&timezoneScreen, ScreenTransition::FADE);
        })

        .addAction("Stats", [this]() {
            screenManager.setScreen(&statsScreen, ScreenTransition::FADE);
        })

        // Sleep Timeout: Intern Millisekunden, angezeigt als Sekunden
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
        })

        .build();

    std::unique_ptr<GenericMenuScreen> mainMenuScreen = std::make_unique<GenericMenuScreen>("SETTINGS", std::move(menuItems));
    screenManager.registerScreen(ScreenType::MAIN_MENU, mainMenuScreen.get());

    // Setup timezone exit callback
    timezoneScreen.setCallback([this, &mainMenuScreen]() {
        screenManager.setScreen(mainMenuScreen.get(), ScreenTransition::FADE);
    });

    return mainMenuScreen;
}
