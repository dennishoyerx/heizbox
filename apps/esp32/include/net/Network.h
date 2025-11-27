// include/net/NetworkSetup.h
#pragma once

#include <functional>
#include "net/WiFiManager.h"
#include "net/WebSocketManager.h"
#include "Config.h"
#include <ArduinoJson.h>

/**
 * @brief Manages the setup of network-related functionalities, including WiFi and WebSocket.
 *
 * This class encapsulates the initialization and callback registration for WiFiManager
 * and WebSocketManager, centralizing network configuration.
 */
class Network {
public:
    /**
     * @brief Constructs a NetworkSetup instance.
     * @param wifi Reference to the WiFiManager instance.
     * @param webSocket Reference to the WebSocketManager instance.
     * @param handleWebSocketMessageCallback Callback to handle incoming WebSocket messages.
     */
    Network(
        WiFiManager& wifi,
        WebSocketManager& webSocket
    );

    /**
     * @brief Sets up WiFi and WebSocket connections with their respective callbacks.
     * @param ssid The WiFi SSID.
     * @param password The WiFi password.
     * @param hostname The device hostname for WiFi.
     */
    void setup(const char* ssid, const char* password, const char* hostname);
    void setupWifi(const char* ssid, const char* password, const char* hostname);
    void onReady(std::function<void()> callback);
    void update();
    void handleWebSocketMessage(const char* type, const JsonDocument& doc);


private:
    WiFiManager& wifi;
    WebSocketManager& webSocket;
    std::function<void()> onReadyCallback;
    bool initialized = false;
};
