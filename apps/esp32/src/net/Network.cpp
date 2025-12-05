#include "net/Network.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include "Config.h"
#include <time.h>
#include "core/EventBus.h"

Network::Network(
    WiFiManager& wifi,
    WebSocketManager& webSocket
)
    : wifi(wifi),
      webSocket(webSocket),
      initialized(false)
{}

void Network::setup(const char* ssid, const char* password, const char* hostname) {
    setupWifi(ssid, password, hostname);

    // Setup WebSocket
    webSocket.onMessage([this](const char* type, const JsonDocument& doc) {
        handleWebSocketMessage(type, doc);
    });

    webSocket.onConnectionChange([this](bool connected) {
        Serial.printf("🔌 WebSocket %s\n", connected ? "connected" : "disconnected");
        if (connected) {
            if (wifi.isConnected() && onReadyCallback) {
                onReadyCallback();
            }
        }
    });
}

void Network::update() {
    wifi.update();
    webSocket.update();
}

void Network::setupWifi(const char* ssid, const char* password, const char* hostname) {

    wifi.init(ssid, password, hostname);
    wifi.onConnectionChange([this](bool connected) {
        EventBus::instance()->publish(Event{connected ? EventType::WIFI_CONNECTED : EventType::WIFI_DISCONNECTED, nullptr});
        if (!initialized && connected) {
            configTime(DeviceState::instance().timezoneOffset.get(), 0, NetworkConfig::NTP_SERVER);
            webSocket.init(NetworkConfig::BACKEND_WS_URL, NetworkConfig::DEVICE_ID, "device");
            initialized = true;
        }
    });
}


void Network::onReady(std::function<void()> callback) {
    onReadyCallback = callback;
}

void Network::handleWebSocketMessage(const char* type, const JsonDocument& doc) {
    if (strcmp(type, "sessionData") == 0 || strcmp(type, "sessionUpdate") == 0) {
        if (!doc["consumption"].isNull()) {
            DeviceState::instance().sessionConsumption.set(doc["consumption"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            DeviceState::instance().todayConsumption.set(doc["consumptionTotal"].as<float>());
        }
        if (!doc["consumptionTotal"].isNull()) {
            DeviceState::instance().yesterdayConsumption.set(doc["consumptionYesterday"].as<float>());
        }
    }
}
