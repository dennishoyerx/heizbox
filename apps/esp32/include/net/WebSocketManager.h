#pragma once

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <functional>

class WebSocketManager {
public:
    using MessageCallback = std::function<void(const char* type, const JsonDocument& doc)>;
    using ConnectionCallback = std::function<void(bool connected)>;

    void init(const char* url, const char* deviceId, const char* clientType = "device");
    void update();

    bool isConnected() const { return state.connected; }

    // Send methods
    bool sendHeartbeat();
    bool sendStatusUpdate(bool isOn, bool isHeating);
    bool sendHeatCycleCompleted(uint32_t durationSec, uint8_t cycle);
    bool sendSessionUpdate(int clicks, int caps);

    // Callbacks
    void onMessage(MessageCallback callback);
    void onConnectionChange(ConnectionCallback callback);

    WebSocketsClient webSocket;

    static WebSocketManager& instance();

private:
    WebSocketManager() = default;

    struct State {
        bool connected = false;
        uint32_t lastHeartbeat = 0;
        uint32_t reconnectAttempts = 0;
    } state;

    MessageCallback messageCallback;
    ConnectionCallback connectionCallback;

    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000;

    // Send helper
    bool sendJson(const JsonDocument& doc);

    // Event handler
    void handleEvent(WStype_t type, uint8_t* payload, size_t length);

    // Static wrapper für WebSocket-Callback
    static void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);
};
