#include "services/FirmwareUpdater.h"
#include "core/EventBus.h"
#include "utils/Logger.h"  // logPrint
#include "Config.h"
#include "credentials.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <ArduinoJson.h>

FirmwareUpdater::FirmwareUpdater() {}

int FirmwareUpdater::parseVersionPart(const String& v, int index) {
    int start = 0;
    for (int i = 0; i < index; i++) {
        int dot = v.indexOf('.', start);
        if (dot < 0) return 0;
        start = dot + 1;
    }
    int end = v.indexOf('.', start);
    if (end < 0) end = v.length();
    return v.substring(start, end).toInt();
}

bool FirmwareUpdater::checkVersion() {
    WiFiClientSecure client;
    client.setInsecure(); // Cloudflare TLS - public cert, kein PIN noetig
    HTTPClient http;
    String url = String(API_ENDPOINT) + "/firmware.json";
    http.begin(client, url);
    http.setTimeout(10000);
    http.setConnectTimeout(5000);

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        logPrint("warn", "Firmware check HTTP %d", code);
        http.end();
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getString());
    http.end();
    if (err) {
        logPrint("error", "Firmware JSON parse failed");
        return false;
    }

    String remote = doc["version"] | "";
    if (remote.isEmpty()) return false;

    int rmaj = parseVersionPart(remote, 0);
    int rmin = parseVersionPart(remote, 1);
    int rpat = parseVersionPart(remote, 2);

    String local = String(FIRMWARE_VERSION);
    int lmaj = parseVersionPart(local, 0);
    int lmin = parseVersionPart(local, 1);
    int lpat = parseVersionPart(local, 2);

    bool newer = (rmaj > lmaj) ||
                 (rmaj == lmaj && rmin > lmin) ||
                 (rmaj == lmaj && rmin == lmin && rpat > lpat);

    if (!newer) {
        logPrint("log", "Firmware up to date (%s)", local.c_str());
        return false;
    }

    logPrint("log", "New firmware available: %s (local %s)", remote.c_str(), local.c_str());

    String binUrl = String(API_ENDPOINT) + doc["url"].as<String>();
    size_t size = doc["size"] | (size_t)0;

    if (size == 0) {
        // Falls size fehlt: HEAD-Request für Content-Length
        WiFiClientSecure headClient;
        headClient.setInsecure();
        HTTPClient head;
        head.begin(headClient, binUrl);
        head.setTimeout(10000);
        head.sendRequest("HEAD");
        size = head.getSize();
        head.end();
    }

    return downloadAndFlash(binUrl.c_str(), size);
}

bool FirmwareUpdater::downloadAndFlash(const char* url, size_t size) {
    updating = true;
    EventBus::instance().publish(Event{EventType::OTA_UPDATE_STARTED, nullptr});

    WiFiClientSecure client;
    client.setInsecure(); // Cloudflare TLS - public cert, kein PIN noetig
    HTTPClient http;
    http.begin(client, url);
    http.setTimeout(30000);
    http.setConnectTimeout(5000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.useHTTP10(true); // kein Chunked Encoding beim Streaming

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        logPrint("error", "Firmware download failed HTTP %d", code);
        EventBus::instance().publish(Event{EventType::OTA_UPDATE_FAILED, nullptr});
        updating = false;
        http.end();
        return false;
    }

    if (size == 0) size = http.getSize();
    if (size == 0) {
        logPrint("error", "Unknown firmware size, aborting");
        EventBus::instance().publish(Event{EventType::OTA_UPDATE_FAILED, nullptr});
        updating = false;
        http.end();
        return false;
    }

    if (!Update.begin(size)) {
        logPrint("error", "Update.begin failed: %s", Update.errorString());
        EventBus::instance().publish(Event{EventType::OTA_UPDATE_FAILED, nullptr});
        updating = false;
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    if (written != size) {
        logPrint("error", "Update write incomplete: %u/%u", written, size);
    }

    if (Update.end()) {
        logPrint("log", "OTA done (%u bytes), rebooting...", written);
        EventBus::instance().publish(Event{EventType::OTA_UPDATE_FINISHED, nullptr});
        http.end();
        delay(500);
        ESP.restart();
        return true;
    } else {
        logPrint("error", "Update.end failed: %s", Update.errorString());
        EventBus::instance().publish(Event{EventType::OTA_UPDATE_FAILED, nullptr});
    }

    updating = false;
    http.end();
    return false;
}

bool FirmwareUpdater::checkNow(bool force) {
    if (updating) return false;
    uint32_t now = millis();
    // Rate-Limit: max 1 Check pro Minute (WS-Reconnects koennen haeufig sein)
    if (!force && now - lastCheck < MIN_CHECK_GAP_MS) {
        logPrint("log", "Update check rate-limited");
        return false;
    }
    lastCheck = now;
    logPrint("log", "Checking firmware...");
    checkVersion();
    return true;
}

void FirmwareUpdater::update() {
    if (updating) return;
    uint32_t now = millis();
    if (now - lastCheck >= CHECK_INTERVAL_MS) {
        lastCheck = now;
        checkVersion();
    }
}
