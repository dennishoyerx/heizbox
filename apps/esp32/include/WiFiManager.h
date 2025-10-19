// include/WiFiManager.h
#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <functional>

class WiFiManager {
public:
    using ConnectionCallback = std::function<void(bool connected)>;

    WiFiManager();

    void init(const char* ssid, const char* password, const char* hostname = "heizbox");
    void update();

    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    IPAddress getIP() const { return WiFi.localIP(); }
    int8_t getRSSI() const { return WiFi.RSSI(); }

    void onConnectionChange(ConnectionCallback callback);

private:
    struct State {
        wl_status_t lastStatus;
        uint32_t lastReconnectAttempt;
        uint32_t reconnectCount;
        bool notifiedConnected;
    } state;

    ConnectionCallback connectionCallback;

    static constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
    static constexpr uint32_t STATUS_CHECK_INTERVAL_MS = 1000;

    uint32_t lastStatusCheck;

    void checkStatus();
    void handleDisconnection();
    static void onWiFiEvent(WiFiEvent_t event);

    static WiFiManager* instance;  // Für static callback
};

#endif
