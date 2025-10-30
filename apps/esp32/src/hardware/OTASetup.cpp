// src/hardware/OTASetup.cpp
#include "hardware/OTASetup.h"

OTASetup::OTASetup(ScreenManager& screenManager, OtaUpdateScreen& otaUpdateScreen, FireScreen& fireScreen)
    : screenManager(screenManager),
      otaUpdateScreen(otaUpdateScreen),
      fireScreen(fireScreen)
{}

void OTASetup::setupOTA() {
    ArduinoOTA.setHostname("Heizbox");

    ArduinoOTA.onStart([this]() {
        logPrint("OTA", "Update started");
        screenManager.setScreen(&otaUpdateScreen);
    });

    ArduinoOTA.onEnd([this]() {
        logPrint("OTA", "Update completed");
        screenManager.setScreen(&fireScreen);
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        logPrint("OTA", "Error[%u]: ", error);
        switch (error) {
            case OTA_AUTH_ERROR:    logPrint("OTA", "Auth Failed"); break;
            case OTA_BEGIN_ERROR:   logPrint("OTA", "Begin Failed"); break;
            case OTA_CONNECT_ERROR: logPrint("OTA", "Connect Failed"); break;
            case OTA_RECEIVE_ERROR: logPrint("OTA", "Receive Failed"); break;
            case OTA_END_ERROR:     logPrint("OTA", "End Failed"); break;
        }
        screenManager.setScreen(&fireScreen);
    });

    ArduinoOTA.begin();
    Serial.println("📲 OTA ready");
}

void OTASetup::handleOTA() {
    ArduinoOTA.handle();
}
