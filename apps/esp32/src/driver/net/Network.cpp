#include "driver/net/Network.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include "Config.h"
#include <time.h>
#include "core/EventBus.h"
#include "SysModule.h"
#include "driver/net/WebSocketManager.h"

Network::Network() : wifi(), ota(), initialized(false) {}

void Network::init(const char* ssid, const char* password, const char* hostname) {
    auto booted = SysModules::booting("net");
    setupWifi(ssid, password, hostname);
    WebSocketManager& ws = WebSocketManager::instance();

    // Setup WebSocket
    ws.onMessage([this](const char* type, const JsonDocument& doc) {
        handleWebSocketMessage(type, doc);
    });

    ws.onConnectionChange([this](bool connected) {
        Serial.printf("🔌 WebSocket %s\n", connected ? "connected" : "disconnected");
        static bool submitted = false;
        if (!submitted && connected) {
            std::vector<SysModuleBoot> modules = SysModules::instance().get();
            
            for (const auto& boot : modules) {
                logPrint("boot", "%s %lu", boot.key, boot.time);
            }

            submitted = true;
        }
    });

    ota.setup();
    booted();
}

void Network::update() {
    wifi.update();
    WebSocketManager::instance().update();
    ota.handle();
}

void Network::setupWifi(const char* ssid, const char* password, const char* hostname) {
    wifi.init(ssid, password, hostname);
    wifi.onConnectionChange([this](bool connected) {
        EventBus::instance().publish(Event{connected ? EventType::WIFI_CONNECTED : EventType::WIFI_DISCONNECTED, nullptr});
        if (!initialized && connected) {
            configTime(DeviceState::instance().timezoneOffset.get(), 0, NetworkConfig::NTP_SERVER);
            WebSocketManager::instance().init(NetworkConfig::BACKEND_WS_URL, NetworkConfig::DEVICE_ID, "device");
            initialized = true;
        }
    });
}

void Network::handleWebSocketMessage(const char* type, const JsonDocument& doc) {
    if (strcmp(type, "sessionData") == 0 || strcmp(type, "sessionUpdate") == 0) {
        auto& ds = DeviceState::instance();
        if (!doc["consumption"].isNull()) {
            ds.consumption.session.set(doc["consumption"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            ds.consumption.today.set(doc["consumptionTotal"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            ds.consumption.yesterday.set(doc["consumptionYesterday"].as<float>());
        }
    }
}
