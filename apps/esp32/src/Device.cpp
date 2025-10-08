#include "Device.h"
#include "nvs_flash.h"
#include "ScreenType.h"
#include "credentials.h"
#include <HTTPClient.h>
#include <ArduinoOTA.h>

// Static instance pointer initialization
Device* Device::instance = nullptr;

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

    if (heater.isCycleFinished()) {
        sendHeatingData(heater.getLastCycleDuration());
        heater.clearCycleFinishedFlag();
    }

    screenManager.update();
    screenManager.draw();

    ArduinoOTA.handle();

    delay(5);
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
    if (WiFi.status() == WL_CONNECTED) {
        unsigned long* durationPtr = new unsigned long(duration);
        xTaskCreate(
            sendHeatingDataTask,       // Task function
            "HeatingDataSender",       // Name of the task
            10000,                     // Stack size in words
            (void*)durationPtr,        // Task input parameter
            1,                         // Priority of the task
            NULL                       // Task handle
        );
    } else {
        Serial.println("❌ WiFi not connected, cannot send heating data.");
    }
}

void Device::sendHeatingDataTask(void* pvParameters) {
    unsigned long duration = *((unsigned long*)pvParameters);
    delete (unsigned long*)pvParameters; // Free the allocated memory for the parameter

    instance->display.freeSprites(); // Free memory before HTTPS request
    
    HTTPClient http;
    unsigned long durationInSeconds = duration / 1000; // Convert milliseconds to seconds
    String url = "http://heizbox.dh19.workers.dev/api/create?duration=" + String(durationInSeconds);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        Serial.printf("✅ HTTP Request successful, code: %d\n", httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
    } else {
        Serial.printf("❌ HTTP Request failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
    
    instance->display.reallocateSprites(); // Reallocate sprites after request
    Serial.printf("Heap after reallocate: %d\n", ESP.getFreeHeap());

    vTaskDelete(NULL); // Delete the task when done
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
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("🌐 Verbindung verloren, versuche neu...");
            WiFi.reconnect();
        default:
            break;
    }
}
