#include "net/OTASetup.h"
#include "utils/Logger.h"
#include "core/EventBus.h"

OTASetup::OTASetup() {}

void OTASetup::setup() {
    ArduinoOTA.setHostname("Heizbox");

    ArduinoOTA.onStart([this]() {
        EventBus::instance()->publish(EventType::OTA_UPDATE_STARTED, nullptr);
    });

    ArduinoOTA.onEnd([this]() {
        EventBus::instance()->publish(EventType::OTA_UPDATE_FINISHED, nullptr);
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        switch (error) {
            case OTA_AUTH_ERROR:    logPrint("OTA", "ERROR: Auth Failed"); break;   
            case OTA_BEGIN_ERROR:   logPrint("OTA", "ERROR: Begin Failed"); break;
            case OTA_CONNECT_ERROR: logPrint("OTA", "ERROR: Connect Failed"); break;
            case OTA_RECEIVE_ERROR: logPrint("OTA", "ERROR: Receive Failed"); break;
            case OTA_END_ERROR:     logPrint("OTA", "ERROR: End Failed"); break;
        }
        EventBus::instance()->publish(EventType::OTA_UPDATE_FAILED, &error);
    });

    ArduinoOTA.begin();
    Serial.println("📲 OTA ready");
}

void OTASetup::handle() {
    ArduinoOTA.handle();
}
