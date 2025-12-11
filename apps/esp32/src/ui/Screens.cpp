#include "ui/Screens.h"
#include "core/EventBus.h"

Screens::Screens(HeaterController& heater): heater(heater) {}

void Screens::setup(ScreenManager& screenManager) {
    fireScreen = std::make_unique<FireScreen>(heater);
    otaUpdateScreen = std::make_unique<OtaUpdateScreen>();
    timezoneScreen = std::make_unique<TimezoneScreen>();
    startupScreen = std::make_unique<StartupScreen>([&]() {
        screenManager.switchScreen(ScreenType::FIRE, ScreenTransition::FADE);
    });

    screenManager.registerScreen(ScreenType::STARTUP, startupScreen.get());
    screenManager.registerScreen(ScreenType::FIRE, fireScreen.get());
    screenManager.registerScreen(ScreenType::TIMEZONE, timezoneScreen.get());
    screenManager.registerScreen(ScreenType::OTA_UPDATE, otaUpdateScreen.get());

    
    EventBus::instance().subscribe(EventType::OTA_UPDATE_STARTED, [&](const Event& event) {
        screenManager.switchScreen(ScreenType::OTA_UPDATE);
    });
    EventBus::instance().subscribe(EventType::OTA_UPDATE_FINISHED, [&](const Event& event) {
        screenManager.switchScreen(ScreenType::FIRE);
    });
    EventBus::instance().subscribe(EventType::OTA_UPDATE_FAILED, [&](const Event& event) {
        screenManager.switchScreen(ScreenType::FIRE);
    });
};

void Screens::setupMenus(ScreenManager& screenManager) {
    auto& state = DeviceState::instance();

    auto menuItems = MenuBuilder()
         .addAction("Debug", [&]() {
             screenManager.switchScreen(ScreenType::DEBUG_MENU, ScreenTransition::FADE);
         })
         .addAction("Heater", [&]() {
             screenManager.switchScreen(ScreenType::HEAT_MENU, ScreenTransition::FADE);
         })
        .addHeadline("DISPLAY")
        .addObservableRange("Brightness", state.brightness,
                           static_cast<uint8_t>(20),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")
        .addObservableToggle("Dark Mode", state.darkMode)
        .addObservableToggle("Smart Mode", state.smart)
        .addObservableToggle("Center Heat", state.enableCenterButtonForHeating)
        .addAction("Timezone", [&]() {
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
         .addHeadline("ZVS")
         .addObservableRange("Power", state.power, static_cast<uint8_t>(0), static_cast<uint8_t>(100), static_cast<uint8_t>(10), "%")
         .addObservableRangeMs("Off Period", state.tempSensorOffTime, 0, 220, 20, true)
         .addObservableRangeMs("Duty Period", state.zvsDutyCyclePeriodMs, 200, 2000, 100, true)
         .addObservableToggle("Debug", state.zvsDebug)
         .addHeadline("Temperature")
         .addObservableRange("Heating Offset", state.heatingTempOffset, static_cast<int8_t>(-50), static_cast<int8_t>(50), static_cast<int8_t>(1), "°C")
         .addObservableRangeMs("Read interval", state.tempSensorReadInterval, 50, 220, 10, true)
         .addObservableRange("IR Emissivity", state.irEmissivity, static_cast<uint8_t>(0), static_cast<uint8_t>(100), static_cast<uint8_t>(1), "%")
         .addObservableRange("Temp Offset", state.temperatureOffset, static_cast<uint8_t>(0), static_cast<uint8_t>(100), static_cast<uint8_t>(1), "°C")
          .build();
    this->heaterMenuScreen = std::make_unique<GenericMenuScreen>("HEATER", std::move(heaterMenuItems));
    screenManager.registerScreen(ScreenType::HEAT_MENU, this->heaterMenuScreen.get());
    
    auto debugMenuItems = MenuBuilder()
         .addObservableToggle("Input", state.debugInput)
         .addObservableToggle("ZVS Debug", state.zvsDebug)
         .build();
    this->debugMenuScreen = std::make_unique<GenericMenuScreen>("DEBUG", std::move(debugMenuItems));
    screenManager.registerScreen(ScreenType::DEBUG_MENU, this->debugMenuScreen.get());

    // Setup timezone exit callback
    timezoneScreen->setCallback([&]() {
        screenManager.setScreen(this->mainMenuScreen.get(), ScreenTransition::FADE);
    });
}
