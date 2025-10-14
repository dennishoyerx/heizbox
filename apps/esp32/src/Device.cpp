#include "Device.h"
#include "nvs_flash.h"
#include "ScreenType.h"
#include "credentials.h"
#include "config.h" // Include config.h
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// Static instance pointer initialization
Device* Device::instance = nullptr;

void Device::setCurrentCycle(int cycle) {
    _lastSetCycle = cycle;
    Serial.printf("Current cycle set to: %d\n", _lastSetCycle);
}

Device::Device()
    : input(),
      heater(),
      display(&clockManager),
      clockManager(),
      preferences(),
      statsManager(),
      screenManager(display, input),
      fireScreen(heater, &screenManager, &screensaverScreen, [this](int cycle) { this->setCurrentCycle(cycle); }),
      mainMenuScreen(&display, &screenManager),
      hiddenModeScreen(&display),
      screensaverScreen(clockManager, 30000, &display),
      otaUpdateScreen(&display),
      statsScreen(statsManager),
      timezoneScreen(clockManager, &screenManager),
      startupScreen() { // Initialize startupScreen here
    instance = this; // Set the static instance pointer
}

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    if (Device::instance) {
        Device::instance->handleWebSocketEvent(type, payload, length);
    }
}

void Device::setup() {
    Serial.begin(115200);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Setup WiFi
	WiFi.setHostname("Heizbox");
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Setup core components
    input.init();
    heater.init();
    statsManager.init();
    display.init(&screenManager);
    clockManager.init(); // Initialize ClockManager here

    // Set initial screen
    screenManager.setScreen(&fireScreen); // Set fireScreen as the initial screen

    // Configure screen navigation
    mainMenuScreen.setStatsScreen(&statsScreen);
    mainMenuScreen.setTimezoneScreen(&timezoneScreen);
    timezoneScreen.onExit([this]() {
        screenManager.setScreen(&mainMenuScreen);
    });

    // Configure screensaver exit
    screensaverScreen.onExit([this]() {
        screenManager.setScreen(&fireScreen);
        fireScreen.resetActivityTimer();
    });

    // Setup input callback
    input.setCallback([this](InputEvent event) {
        this->handleInput(event);
    });

    // Initialize WebSocket
    initWebSocket();

    ArduinoOTA.setHostname("Heizbox");
    //ArduinoOTA.setPassword("esp_pass");

    ArduinoOTA.onStart([this]() {
        Serial.println("📲 OTA Start");
        this->screenManager.setScreen(&this->otaUpdateScreen);
    });
    ArduinoOTA.onEnd([this]() {
        Serial.println("📲 OTA Ende");
        this->screenManager.setScreen(&this->fireScreen);
    });
    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.printf("📲 Fehler[%u]\n", error);
        this->screenManager.setScreen(&this->fireScreen);
    });

    ArduinoOTA.begin();
    Serial.println("📲 OTA bereit");
}

void Device::loop() {
    input.update();
    heater.update();
    clockManager.update();
    display.renderStatusBar();
    webSocket.loop(); // Process WebSocket events

    // Check heating status and send update if changed
    bool currentHeatingStatus = heater.isHeating();
    if (currentHeatingStatus != _lastHeatingStatusSent) {
        sendHeatingStatus(currentHeatingStatus);
        _lastHeatingStatusSent = currentHeatingStatus;
    }

    // Send heartbeat
    if (millis() - _lastHeartbeatTime > HEARTBEAT_INTERVAL) {
        if (webSocket.isConnected()) {
            DynamicJsonDocument doc(128);
            doc["type"] = "heartbeat";
            doc["isOn"] = true; // Device is on and sending heartbeat
            String output;
            serializeJson(doc, output);
            webSocket.sendTXT(output);
            Serial.printf("❤️ WS Sent heartbeat: %s\n", output.c_str());
        } else {
            Serial.println("❌ WebSocket not connected, cannot send heartbeat.");
        }
        _lastHeartbeatTime = millis();
    }

    // Check if a heat cycle has finished and send data
    if (heater.isCycleFinished()) {
        unsigned long duration = heater.getLastCycleDuration();
        if (webSocket.isConnected()) {
            DynamicJsonDocument doc(128);
            doc["type"] = "heatCycleCompleted";
            doc["duration"] = duration / 1000; // Convert to seconds
            doc["cycle"] = _lastSetCycle; // Include the last set cycle
            String output;
            serializeJson(doc, output);
            webSocket.sendTXT(output);
            Serial.printf("✅ WS Sent heat cycle completed: %s\n", output.c_str());
        } else {
            Serial.println("❌ WebSocket not connected, cannot send heat cycle data.");
        }
        heater.clearCycleFinishedFlag();
    }

    screenManager.update();
    screenManager.draw();

    ArduinoOTA.handle();

    delay(5);
}

void Device::sendHeatingStatus(bool heatingStatus) {
    if (webSocket.isConnected()) {
        DynamicJsonDocument doc(128);
        doc["type"] = "statusUpdate";
        doc["isHeating"] = heatingStatus;
        String output;
        serializeJson(doc, output);
        webSocket.sendTXT(output);
        Serial.printf("✅ WS Sent heating status: %s\n", output.c_str());
    } else {
        Serial.println("❌ WebSocket not connected, cannot send heating status.");
    }
}

void Device::initWebSocket() {
    String wsUrl = String(BACKEND_WS_URL) + "?deviceId=" + DEVICE_ID + "&type=device";
    Serial.printf("[WS] Connecting to: %s\n", wsUrl.c_str());

    // Parse URL to get host, port, and path
    String host = wsUrl.substring(wsUrl.indexOf("//") + 2, wsUrl.indexOf("/ws/status"));
    String path = wsUrl.substring(wsUrl.indexOf("/ws/status"));
    int port = 443; // Assuming SSL/WSS

    webSocket.beginSSL(host.c_str(), port, path.c_str(), "", "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000); // Try to reconnect every 5s
}

void Device::handleWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Disconnected!");
            break;
        case WStype_CONNECTED:
            Serial.printf("[WS] Connected to url: %s\n", payload);
            // Send initial status (isOn: true, isHeating: false)
            {
                DynamicJsonDocument doc(128);
                doc["type"] = "statusUpdate";
                doc["isOn"] = true;
                doc["isHeating"] = false;
                String output;
                serializeJson(doc, output);
                webSocket.sendTXT(output);
                Serial.printf("✅ WS Sent initial status: %s\n", output.c_str());
            }
            _lastHeartbeatTime = millis(); // Initialize heartbeat timer on connection
            break;
        case WStype_TEXT:
            // Serial.printf("[WS] get text: %s\n", payload);
            // Handle incoming messages if needed
            break;
        case WStype_BIN:
            // Serial.printf("[WS] get binary length: %u\n", length);
            break;
        case WStype_ERROR:
            Serial.printf("[WS] Error: %s\n", payload);
            break;
    }
}

void Device::handleInput(InputEvent event) {
    const char* typeStr = event.type == PRESS ? "PRESS" :
                          event.type == RELEASE ? "RELEASE" : "HOLD";
    const char* btnStr = event.button == UP ? "UP" :
                         event.button == DOWN ? "DOWN" :
                         event.button == LEFT ? "LEFT" :
                         event.button == RIGHT ? "RIGHT" :
                         event.button == CENTER ? "CENTER" : "FIRE";
    Serial.printf("🎮 %s %s\n", btnStr, typeStr);

    // Pass input to the screen manager
    screenManager.handleInput(event);

    // Handle global screen switching
    handleGlobalScreenSwitching(event);
}

void Device::handleGlobalScreenSwitching(InputEvent event) {
    if (screenManager.getCurrentScreenType() == ScreenType::MAIN_MENU && event.button == LEFT && event.type == HOLD) {
        screenManager.setScreen(&fireScreen);
    } else if (screenManager.getCurrentScreenType() == ScreenType::FIRE && event.button == LEFT && event.type == HOLD) {
        screenManager.setScreen(&mainMenuScreen);
    }
}

void Device::WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("🌐 WiFi gestartet, verbinde...");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("🌐 Verbunden");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("🌐 IP: ");
            Serial.println(WiFi.localIP());
            if (instance) {
                String wsUrl = String(BACKEND_WS_URL) + "?deviceId=" + DEVICE_ID + "&type=device";
                String host = wsUrl.substring(wsUrl.indexOf("//") + 2, wsUrl.indexOf("/ws/status"));
                String path = wsUrl.substring(wsUrl.indexOf("/ws/status"));
                int port = 443; // Assuming SSL/WSS
                instance->webSocket.beginSSL(host.c_str(), port, path.c_str(), "", "/");
            }
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("🌐 Verbindung verloren, versuche neu...");
            WiFi.reconnect();
        default:
            break;
    }
}

void Device::sendHeatingDataTask(void* pvParameters) {
    // This task is no longer needed as data is sent via WebSocket
    vTaskDelete(NULL); // Delete the task immediately
}
