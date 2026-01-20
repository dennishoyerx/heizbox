#include "ui/Screens.h"
#include "core/EventBus.h"
#include "heater/HeaterState.h"

Screens::Screens(HeaterController& heater): heater(heater) {}

void Screens::setup(ScreenManager& screenManager) {
    fireScreen = std::make_unique<FireScreen>(heater);
    otaUpdateScreen = std::make_unique<OtaUpdateScreen>();
    timezoneScreen = std::make_unique<TimezoneScreen>();
    startupScreen = std::make_unique<StartupScreen>([&]() {
        screenManager.setStatusbarVisible(true);
        screenManager.switchScreen(ScreenType::FIRE, ScreenTransition::NONE);
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
        //screenManager.switchScreen(ScreenType::FIRE);
    });
};

void Screens::setupMenus(ScreenManager& screenManager) {
    auto& state = DeviceState::instance();
    auto& hs = HeaterState::instance();


    auto menuItems = MenuBuilder()
        .addAction(" ", [&]() {})
         .addAction("Heater", [&]() {
             screenManager.switchScreen(ScreenType::HEAT_MENU, ScreenTransition::NONE);
         })
         .addAction("Debug", [&]() {
             screenManager.switchScreen(ScreenType::DEBUG_MENU, ScreenTransition::NONE);
         })
        .addHeadline("DISPLAY")
        .addObservableRange("Brightness", state.display.brightness,
                           static_cast<uint8_t>(20),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")
                           
        .addObservableRange("Volume", state.audio.volume,
                           static_cast<uint8_t>(0),
                           static_cast<uint8_t>(100),
                           static_cast<uint8_t>(10), "%")
        .addObservableToggle("Dark Mode", state.display.darkMode)
        .addAction("Timezone", [&]() {
            screenManager.setScreen(timezoneScreen.get(), ScreenTransition::NONE);
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
         .addHeadline("Presets")
         .addObservableRange("Mode", hs.mode, static_cast<uint8_t>(0), static_cast<uint8_t>(1), static_cast<uint8_t>(1), "")
         .addObservableRange("Flavor Temp", hs.preset1Temp, static_cast<uint8_t>(150), static_cast<uint8_t>(240), static_cast<uint8_t>(1), "°C")
         .addObservableRange("Balanced Temp", hs.preset2Temp, static_cast<uint8_t>(150), static_cast<uint8_t>(240), static_cast<uint8_t>(1), "°C")
         .addObservableRange("Extraction Temp", hs.preset3Temp, static_cast<uint8_t>(150), static_cast<uint8_t>(240), static_cast<uint8_t>(1), "°C")
         .addObservableRange("Full Temp", hs.preset4Temp, static_cast<uint8_t>(150), static_cast<uint8_t>(240), static_cast<uint8_t>(1), "°C")
         .addHeadline("Calibration")
         .addObservableRange("Click 1 DV Temp", hs.irCalActualA, static_cast<uint16_t>(150), static_cast<uint16_t>(180), static_cast<uint16_t>(1), "°C")
         .addObservableRange("Click 2 DV Temp", hs.irCalActualB, static_cast<uint16_t>(180), static_cast<uint16_t>(220), static_cast<uint16_t>(1), "°C")
         .addObservableRange("Click 1 IR Temp", hs.irCalMeasuredA, static_cast<uint16_t>(0), static_cast<uint16_t>(500), static_cast<uint16_t>(1), "°C")
         .addObservableRange("Click 2 IR Temp", hs.irCalMeasuredB, static_cast<uint16_t>(0), static_cast<uint16_t>(500), static_cast<uint16_t>(1), "°C")
         .addObservableRange("Slope", hs.irCalSlope, static_cast<float>(0), static_cast<float>(1), static_cast<float>(0.01), "")
         .addObservableRange("Offset", hs.irCalOffset, static_cast<float>(0), static_cast<float>(1), static_cast<float>(0.01), "")
         .addAction("Recalculate", [&]() {
             heater.computeIRCalibration();
         })
         .addAction("Clear Calibration", [&]() {
             heater.clearIRCalibration();
         })
         .addHeadline("ZVS")
         .addObservableRange("Power", hs.power, static_cast<uint8_t>(0), static_cast<uint8_t>(100), static_cast<uint8_t>(10), "%")
         .addObservableRangeMs("Off Period", hs.tempSensorOffTime, 0, 220, 20, true)
         .addObservableRangeMs("Duty Period", hs.zvsDutyCyclePeriodMs, 200, 2000, 100, true)
         .addHeadline("Temperature")
         .addObservableRange("Heating Offset", hs.tempCorrection, static_cast<int8_t>(-50), static_cast<int8_t>(50), static_cast<int8_t>(1), "°C")
         .addObservableRangeMs("Read interval", hs.tempSensorReadInterval, 50, 220, 10, true)
         .addObservableRange("IR Emissivity", hs.irEmissivity, static_cast<uint8_t>(0), static_cast<uint8_t>(100), static_cast<uint8_t>(1), "%")
          .build();
    this->heaterMenuScreen = std::make_unique<GenericMenuScreen>("HEATER", std::move(heaterMenuItems));
    screenManager.registerScreen(ScreenType::HEAT_MENU, this->heaterMenuScreen.get());
    
    auto debugMenuItems = MenuBuilder()
         .addObservableToggle("Input logging", state.debug.input)
         .addObservableToggle("ZVS OSC", state.debug.osc)
         .addObservableToggle("ZVS Debug", state.debug.zvs)
         .addObservableToggle("Raw Temp", state.debug.showRawTemp)
         .build();
    this->debugMenuScreen = std::make_unique<GenericMenuScreen>("DEBUG", std::move(debugMenuItems));
    screenManager.registerScreen(ScreenType::DEBUG_MENU, this->debugMenuScreen.get());

    // Setup timezone exit callback
    timezoneScreen->setCallback([&]() {
        screenManager.setScreen(this->mainMenuScreen.get(), ScreenTransition::NONE);
    });
}
