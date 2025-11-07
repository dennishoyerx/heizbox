#include "net/Network.h"
#include "core/DeviceState.h"
#include "core/StateManager.h"
#include "utils/Logger.h"
#include "core/Config.h"
#include <time.h>

Network::Network(
    WiFiManager& wifiManager,
    WebSocketManager& webSocketManager,
    std::function<void(const char*, const JsonDocument&)> handleWebSocketMessageCallback
)
    : wifiManager(wifiManager),
      webSocketManager(webSocketManager),
      handleWebSocketMessageCallback(handleWebSocketMessageCallback),
      initialized(false)
{}

void Network::setup(const char* ssid, const char* password, const char* hostname) {
    wifiManager.init(ssid, password, hostname);
    wifiManager.onConnectionChange([this](bool connected) {
        if (!initialized && connected) {
            configTime(DeviceState::instance().timezoneOffset.get(), 0, NetworkConfig::NTP_SERVER);
            webSocketManager.init(NetworkConfig::BACKEND_WS_URL, NetworkConfig::DEVICE_ID, "device");
            initialized = true;
        }
    });

    // Setup WebSocket
    webSocketManager.onMessage([this](const char* type, const JsonDocument& doc) {
        this->handleWebSocketMessageCallback(type, doc);
    });

    webSocketManager.onConnectionChange([this](bool connected) {
        Serial.printf("🔌 WebSocket %s\n", connected ? "connected" : "disconnected");
        if (connected) {
            if (wifiManager.isConnected() && onReadyCallback) {
                onReadyCallback();
            }
        }
    });
}

void Network::onReady(std::function<void()> callback) {
    onReadyCallback = callback;
}
