#include "net/NetworkSetup.h"
#include "utils/Logger.h"
#include "core/Config.h"

static bool firmware_logged = false;

NetworkSetup::NetworkSetup(
    WiFiManager& wifiManager,
    WebSocketManager& webSocketManager,
    std::function<void(const char*, const JsonDocument&)> handleWebSocketMessageCallback
)
    : wifiManager(wifiManager),
      webSocketManager(webSocketManager),
      handleWebSocketMessageCallback(handleWebSocketMessageCallback)
{}

void NetworkSetup::setupNetwork(const char* ssid, const char* password, const char* hostname) {
    // Setup WiFi
    wifiManager.init(ssid, password, hostname);
    wifiManager.onConnectionChange([this](bool connected) {
        if (connected) {
            webSocketManager.init(NetworkConfig::BACKEND_WS_URL, NetworkConfig::DEVICE_ID, "device");
        }
    });

    // Setup WebSocket
    webSocketManager.onMessage([this](const char* type, const JsonDocument& doc) {
        this->handleWebSocketMessageCallback(type, doc);
    });

    webSocketManager.onConnectionChange([](bool connected) {
        Serial.printf("🔌 WebSocket %s\n", connected ? "connected" : "disconnected");
        if (connected && !firmware_logged) {
            char firmwareInfo[64];
            snprintf(firmwareInfo, sizeof(firmwareInfo), "%s (%s)", FIRMWARE_VERSION, BUILD_DATE);
            logPrint("Firmware", firmwareInfo);
            firmware_logged = true;
        }
    });
}

