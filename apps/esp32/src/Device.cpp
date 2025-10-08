#include "Device.h"
#include "nvs_flash.h"
#include "ScreenType.h"
#include "credentials.h"
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// Static instance pointer initialization
Device* Device::instance = nullptr;

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    if (Device::instance) {
        Device::instance->handleWebSocketEvent(type, payload, length);
    }
}

Device::Device()
    : input(),
      heater(),
      display(&clockManager),
      clockManager(),
      preferences(),
      statsManager(),
      screenManager(display, input),
      fireScreen(heater, &screenManager, &screensaverScreen),
      mainMenuScreen(&display, &screenManager),
      hiddenModeScreen(&display),
      screensaverScreen(clockManager, 30000, &display),
      otaUpdateScreen(&display) {
    instance = this; // Set the static instance pointer
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
    display.init(&screenManager);

    // Set initial screen
    screenManager.setScreen(&fireScreen);

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

    if (heater.isCycleFinished()) {
        sendHeatingData(heater.getLastCycleDuration());
        heater.clearCycleFinishedFlag();
    }

    screenManager.update();
    screenManager.draw();

    ArduinoOTA.handle();

    delay(5);
}

void Device::initWebSocket() {
    webSocket.beginSSL("heizbox.dh19.workers.dev", 443, "/ws/status", "", "/", "wss");
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
            // Send device info on connect
            webSocket.sendTXT("{\"topic\":\"heizbox/device\",\"data\":{\"id\":\"esp32-heizbox-1\",\"status\":\"online\"}}");
            break;
        case WStype_TEXT:
            Serial.printf("[WS] get text: %s\n", payload);
            // Handle incoming messages if needed
            break;
        case WStype_BIN:
            Serial.printf("[WS] get binary length: %u\n", length);
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
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

void Device::sendHeatingData(unsigned long duration) {
    if (webSocket.isConnected()) {
        DynamicJsonDocument doc(128);
        doc["topic"] = "heizbox/status";
        JsonObject data = doc.createNestedObject("data");
        data["heating"] = "on"; // Example status
        data["state"] = "on";   // Example status
        data["duration"] = duration / 1000; // Convert milliseconds to seconds

        String output;
        serializeJson(doc, output);
        webSocket.sendTXT(output);
        Serial.printf("✅ WS Sent heating data: %s\n", output.c_str());
    } else {
        Serial.println("❌ WebSocket not connected, cannot send heating data.");
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
            if (instance && !instance->clockManager.isTimeSynced()) {
                instance->clockManager.init();
            }
            if (instance) {
                instance->webSocket.beginSSL("heizbox.dh19.workers.dev", 443, "/ws/status", "", "/", "wss");
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
