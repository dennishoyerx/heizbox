#include "core/Config.h"
#include "core/Device.h" 
#include "core/DeviceState.h" 
#include "hardware/display/DisplayDriver.h" // Explicit include to resolve incomplete type
#include "credentials.h"
#include <ArduinoOTA.h>
#include "core/StateManager.h"
#include "utils/Logger.h"
#include "utils/Logs.h"
#include <utility> 

// Network
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "net/Network.h"

// UI
#include "ui/base/ScreenManager.h"
#include "ui/screens/FireScreen.h"
#include "ui/screens/HiddenModeScreen.h"
#include "ui/screens/ScreensaverScreen.h"
#include "ui/screens/OtaUpdateScreen.h"
#include "ui/screens/StatsScreen.h"
#include "ui/screens/TimezoneScreen.h"
#include "ui/screens/StartupScreen.h"
#include "ui/base/ScreenTransition.h"
#include "ui/UISetup.h"

#include "hardware/drivers/TFT_eSPI_Driver.h"
#include "hardware/display/BacklightController.h"

Device::Device()
    : input(),
      heater(),
      display(std::make_unique<DisplayDriver>(
          std::make_unique<TFT_eSPI_Driver>(),
          std::make_unique<BacklightController>()
      )),
      statsManager(),
      wifiManager(),
      webSocketManager(),
      screenManager(*display, input),
      uiSetup(std::make_unique<UISetup>(
          screenManager, heater, display.get(), statsManager, input, _tempSensor
      )),
      capacitiveSensor(heater, [this](bool start) { uiSetup->getFireScreen()->_handleHeatingTrigger(start); }),
      _tempSensor(new TempSensor(HardwareConfig::THERMO_SCK_PIN, HardwareConfig::THERMO_CS_PIN, HardwareConfig::THERMO_SO_PIN, HardwareConfig::SENSOR_TEMPERATURE_READ_INTERVAL_MS)),
      network(std::make_unique<Network>(
          wifiManager, webSocketManager,
          [this](const char* type, const JsonDocument& doc) { this->handleWebSocketMessage(type, doc); }
      )),
      otaSetup(std::make_unique<OTASetup>(screenManager, *uiSetup->getOtaUpdateScreen(), *uiSetup->getFireScreen())),
      heaterMonitor(std::make_unique<HeaterMonitor>(
          heater, webSocketManager, statsManager, DeviceState::instance().currentCycle,
          [this]() { this->onHeatCycleFinalized(); }
      )),
      inputHandler(std::make_unique<InputHandler>(screenManager))
{}

Device::~Device() {
    delete _tempSensor; // Clean up TempSensor
}

void Device::setup() {
    Serial.begin(115200);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize core components
    display->init(&screenManager);
    input.init();
    heater.init();
    statsManager.init();
    _tempSensor->begin(); // Initialize TempSensor

    // Setup screens
    uiSetup->setupScreens();
    screenManager.setScreen(uiSetup->getStartupScreen());

    input.setCallback([this](InputEvent event) {
        inputHandler->handleInput(event);
    });

    StateBinder::bindAll(display.get(), &heater);
    DeviceState::instance().display = display.get();

    uiSetup->setupMainMenu();

    network->setup(WIFI_SSID, WIFI_PASSWORD, NetworkConfig::HOSTNAME);
    network->onReady([]() {
        static bool firmware_logged = false;
        if (!firmware_logged) {
            char firmwareInfo[64];
            snprintf(firmwareInfo, sizeof(firmwareInfo), "%s (%s)", FIRMWARE_VERSION, BUILD_DATE);
            logPrint("Firmware", firmwareInfo);
            firmware_logged = true;
        }
    });
    
    otaSetup->setupOTA();

    Serial.println("✅ Device initialized");
}

void Device::loop() {
    // Update managers
    wifiManager.update();
    webSocketManager.update();
    input.update();
    heater.update();
    statsManager.update();
    //capacitiveSensor.update();

    // Update UI
    screenManager.update();
    screenManager.draw();
    display->renderStatusBar();

    heaterMonitor->checkHeatingStatus();
    heaterMonitor->checkHeatCycle();

    otaSetup->handleOTA();
}

// ============================================================================
// Helper Methods
// ============================================================================

void Device::onHeatCycleFinalized() {
    uiSetup->getFireScreen()->onCycleFinalized();
}

void Device::handleWebSocketMessage(const char* type, const JsonDocument& doc) {
    if (strcmp(type, "sessionData") == 0 || strcmp(type, "sessionUpdate") == 0) {
        if (!doc["clicks"].isNull()) {
            DeviceState::instance().sessionClicks.set(doc["clicks"].as<int>());
        }
        if (!doc["caps"].isNull()) {
            DeviceState::instance().sessionCaps.set(doc["caps"].as<int>());
        }
        if (!doc["consumption"].isNull()) {
            DeviceState::instance().sessionConsumption.set(doc["consumption"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            DeviceState::instance().todayConsumption.set(doc["consumptionTotal"].as<float>());
        }
        screenManager.setDirty();
    }
}


