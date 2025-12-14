#ifndef WEBSOCKETMANAGER_H
#define WEBSOCKETMANAGER_H

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <functional>

class WebSocketManager {
public:
    using MessageCallback = std::function<void(const char* type, const JsonDocument& doc)>;
    using ConnectionCallback = std::function<void(bool connected)>;

    WebSocketManager();

    void init(const char* url, const char* deviceId, const char* clientType = "device");
    void update();

    bool isConnected() const { return connected; }

    // Send methods
    bool sendHeartbeat();
    bool sendStatusUpdate(bool isOn, bool isHeating);
    bool sendHeatCycleCompleted(uint32_t durationSec, uint8_t cycle);
    bool sendSessionUpdate(int clicks, int caps);

    // Callbacks
    void onMessage(MessageCallback callback);
    void onConnectionChange(ConnectionCallback callback);

    static WebSocketManager* instance();

public:
    WebSocketsClient webSocket;

private:

    struct State {
        bool connected;
        uint32_t lastHeartbeat;
        uint32_t reconnectAttempts;
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
    static WebSocketManager* _instance;

    bool connected;
};



#endif
