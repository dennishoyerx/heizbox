#pragma once

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <functional>

// Pending message types (gepuffert, damit im Event-Callback-Kontext
// kein JSON gebaut/gesendet wird - Stack-Overflow vermeiden)
enum class WsMsgType : uint8_t {
    NONE = 0,
    HEARTBEAT,
    STATUS_UPDATE,
    HEAT_CYCLE_COMPLETED,
    SESSION_UPDATE,
    TEMP_READING
};

struct WsPendingMsg {
    WsMsgType type = WsMsgType::NONE;
    bool isOn = false;
    bool isHeating = false;
    uint32_t durationSec = 0;
    uint8_t cycle = 0;
    int clicks = 0;
    int caps = 0;
    float tempRaw = 0;
    float tempCalibrated = 0;
};

class WebSocketManager {
public:
    using MessageCallback = std::function<void(const char* type, const JsonDocument& doc)>;
    using ConnectionCallback = std::function<void(bool connected)>;

    void init(const char* url, const char* deviceId, const char* clientType = "device");
    void update();

    bool isConnected() const { return state.connected; }

    // Send methods - puffern NUR in die Queue (kein JSON im Aufrufer-Kontext!)
    bool sendHeartbeat();
    bool sendStatusUpdate(bool isOn, bool isHeating);
    bool sendHeatCycleCompleted(uint32_t durationSec, uint8_t cycle);
    bool sendSessionUpdate(int clicks, int caps);
    bool sendTempReading(float tempRaw, float tempCalibrated, bool isHeating);

    // Callbacks
    void onMessage(MessageCallback callback);
    void onConnectionChange(ConnectionCallback callback);

    WebSocketsClient webSocket;

    static WebSocketManager& instance();

private:
    WebSocketManager() = default;

    struct State {
        bool connected = false;
        bool initialized = false;
        uint32_t lastHeartbeat = 0;
        uint32_t reconnectAttempts = 0;
        uint32_t lastReconnectAttempt = 0;
    } state;

    const char* host;
    const char* path;

    MessageCallback messageCallback;
    ConnectionCallback connectionCallback;

    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 30000;
    static constexpr uint8_t PENDING_MAX = 8;

    // Ringbuffer fuer gepufferte Messages (kein Heap, keine STL-Allocs)
    WsPendingMsg pending[PENDING_MAX];
    uint8_t pendingHead = 0;
    uint8_t pendingCount = 0;

    bool queuePush(const WsPendingMsg& msg);
    bool queuePop(WsPendingMsg& msg);

    // Send helper - NUR aus update() (Loop-Kontext, grosser Stack) aufrufen!
    bool sendJson(const JsonDocument& doc);
    bool flushQueue();

    // Event handler
    void handleEvent(WStype_t type, uint8_t* payload, size_t length);

    // Static wrapper f\u00fcr WebSocket-Callback
    static void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);
};
