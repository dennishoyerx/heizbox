#include <Arduino.h>
#include "Config.h"
#include "core/Device.h"

// For safe mode
#include "credentials.h"
#include "driver/net/WiFiManager.h"
#include <ArduinoOTA.h>

Device device;


void safeMode() {
    Serial.begin(115200);
    Serial.println("Entering safe mode for OTA update...");

    pinMode(HardwareConfig::STATUS_LED_PIN, OUTPUT);

    WiFiManager wifiManager;
    wifiManager.init(WIFI_SSID, WIFI_PASSWORD, "Heizbox-SafeMode");

    bool ota_initialized = false;

    while (true) {
        wifiManager.update();

        if (wifiManager.isConnected()) {
            if (!ota_initialized) {
                // Setup OTA
                ArduinoOTA.setHostname("Heizbox");
                ArduinoOTA.onStart([]() { Serial.println("OTA Start"); });
                ArduinoOTA.onEnd([]() { Serial.println("\nOTA End"); });
                ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
                });
                ArduinoOTA.onError([](ota_error_t error) {
                    Serial.printf("OTA Error[%u]: ", error);
                    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                    else if (error == OTA_END_ERROR) Serial.println("End Failed");
                });
                ArduinoOTA.begin();
                Serial.println("OTA Ready");
                ota_initialized = true;
            }

            ArduinoOTA.handle();

            // LED pattern for "OTA Ready": two quick blinks
            digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
            delay(50);
            digitalWrite(HardwareConfig::STATUS_LED_PIN, LOW);
            delay(50);
            digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
            delay(50);
            digitalWrite(HardwareConfig::STATUS_LED_PIN, LOW);
            delay(1000);
        } else {
            // LED pattern for "Connecting...": slow blink
            digitalWrite(HardwareConfig::STATUS_LED_PIN, HIGH);
            delay(100);
            digitalWrite(HardwareConfig::STATUS_LED_PIN, LOW);
            delay(900);
        }
    }
}

void setup() {
    pinMode(HardwareConfig::FIRE_BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(HardwareConfig::FIRE_BUTTON_PIN, HIGH);
    delay(50);
    if (digitalRead(HardwareConfig::FIRE_BUTTON_PIN) == LOW) safeMode();

    device.setup();
}

void loop() {
    device.loop();
}