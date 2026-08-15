#include "driver/net/Network.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include "Config.h"
#include <time.h>
#include "core/EventBus.h"
#include "SysModule.h"
#include "driver/net/WebSocketManager.h"

Network::Network() : wifi(), ota(), initialized(false), pendingUpdateCheck(false) {}

void Network::init(const char* ssid, const char* password, const char* hostname) {
    auto booted = SysModules::booting("net");
    setupWifi(ssid, password, hostname);
    WebSocketManager& ws = WebSocketManager::instance();

    // Setup WebSocket
    ws.onMessage([this](const char* type, const JsonDocument& doc) {
        handleWebSocketMessage(type, doc);
    });

    ws.onConnectionChange([this](bool connected) {
        Serial.printf("WS %s\n", connected ? "connected" : "disconnected");
        static bool submitted = false;
        if (!submitted && connected) {
            std::vector<SysModuleBoot> modules = SysModules::instance().get();
            
            for (const auto& boot : modules) {
                logPrint("boot", "%s %lu", boot.key, boot.time);
            }

            submitted = true;
        }

        // Kein HTTP hier im Event-Callback! (Stack-Overflow).
        // Flag setzen, update() macht den Check im Loop-Kontext.
        if (connected) {
            pendingUpdateCheck = true;
        }
    });

    // Manueller Update-Check aus dem Men\u00fc - ebenfalls nur Flag setzen
    EventBus::instance().subscribe(EventType::CHECK_FOR_UPDATES, [this](const Event&) {
        pendingUpdateCheck = true;
    });

    booted();
}

void Network::update() {
    wifi.update();
    WebSocketManager::instance().update();
    ota.handle();
    firmwareUpdater.update();

    // Deferred Firmware-Check im Loop-Kontext (grosser Stack)
    if (pendingUpdateCheck) {
        pendingUpdateCheck = false;
        firmwareUpdater.checkNow();
    }
}

void Network::setupWifi(const char* ssid, const char* password, const char* hostname) {
    wifi.init(ssid, password, hostname);
    wifi.onConnectionChange([this](bool connected) {
        EventBus::instance().publish(Event{connected ? EventType::WIFI_CONNECTED : EventType::WIFI_DISCONNECTED, nullptr});
        if (!initialized && connected) {
            configTime(DeviceState::instance().timezoneOffset.get(), 0, NetworkConfig::NTP_SERVER);
            WebSocketManager::instance().init(NetworkConfig::BACKEND_WS_URL, NetworkConfig::DEVICE_ID, "device");
            // OTA erst JETZT starten - ArduinoOTA braucht verbundenes WiFi,
            // sonst bindet der Server auf ein Interface ohne IP (Port 3232 bleibt zu)
            ota.setup();
            initialized = true;
            // Firmware-Check deferred in update() (HTTP im Event-Callback = Stack-Overflow)
            pendingUpdateCheck = true;
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
