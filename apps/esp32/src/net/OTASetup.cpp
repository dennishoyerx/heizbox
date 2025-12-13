#include "net/OTASetup.h"
#include "utils/Logger.h"
#include "core/EventBus.h"

OTASetup::OTASetup() {}

void OTASetup::setup() {
    ArduinoOTA.setHostname("Heizbox");

    ArduinoOTA.onStart([this]() {
        EventBus::instance().publish(EventType::OTA_UPDATE_STARTED, nullptr);
    });

    ArduinoOTA.onEnd([this]() {
        EventBus::instance().publish(EventType::OTA_UPDATE_FINISHED, nullptr);
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        EventBus::instance().publish<ota_error_t>(EventType::OTA_UPDATE_FAILED, error);
    });

    ArduinoOTA.begin();
    Serial.println("📲 OTA ready");
}

void OTASetup::handle() {
    ArduinoOTA.handle();
}
