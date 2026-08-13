#include "services/DebugServer.h"
#include "utils/Logger.h"
#include "driver/net/WebSocketManager.h"
#include "core/DeviceState.h"
#include "heater/HeaterState.h"
#include "Config.h"

#include <WiFi.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include "utils/Logger.h"

static const char* PAGE_HTML PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Heizbox Debug</title>
<style>
body{font-family:ui-monospace,Menlo,Consolas,monospace;background:#111;color:#ddd;margin:0;padding:16px}
h1{font-size:18px;color:#4fc3f7;margin:0 0 8px}
#status{display:grid;grid-template-columns:auto 1fr;gap:4px 12px;font-size:13px;background:#1a1a1a;border:1px solid #333;border-radius:8px;padding:10px;margin-bottom:12px}
#status b{color:#fff}
#log{background:#0d0d0d;border:1px solid #333;border-radius:8px;padding:10px;font-size:12px;height:60vh;overflow-y:auto;white-space:pre-wrap;word-break:break-all}
.ln{color:#666;margin-right:6px}
.t-boot{color:#4fc3f7}.t-ws{color:#ffb74d}.t-wifi{color:#81c784}.t-err{color:#e57373}.t-heat{color:#ff8a65}
button{background:#1e88e5;border:none;color:#fff;padding:8px 16px;border-radius:6px;font-size:13px;cursor:pointer;margin-bottom:12px}
button.danger{background:#c62828}
</style>
</head>
<body>
<h1>🔥 Heizbox Debug <span id="ver" style="font-size:12px;color:#888"></span></h1>
<div id="status"><b>Firmware</b><span id="s_fw">-</span><b>WiFi</b><span id="s_wifi">-</span><b>IP</b><span id="s_ip">-</span><b>RSSI</b><span id="s_rssi">-</span><b>WebSocket</b><span id="s_ws">-</span><b>Uptime</b><span id="s_up">-</span><b>Heap</b><span id="s_heap">-</span><b>Backend</b><span id="s_be">-</span></div>
<button onclick="restart()">🔄 Neustart</button>
<div style="margin-bottom:12px">
<button onclick="checkUpdate()">\u2b06 Update checken</button>
</div>
<div id="settings" style="background:#1a1a1a;border:1px solid #333;border-radius:8px;padding:10px;margin-bottom:12px;font-size:13px">
<b>\u2699 Settings</b><br>
<label>Brightness <input type="range" id="set_brightness" min="20" max="100" step="5" oninput="document.getElementById('s_brightness').textContent=this.value"> <span id="s_brightness">-</span></label><br>
<label>Idle <input type="range" id="set_idle" min="20" max="100" step="5" oninput="document.getElementById('s_idle').textContent=this.value"> <span id="s_idle">-</span></label><br>
<button onclick="saveSettings()">\ud83d\udcbe Speichern</button>
</div>
<div id="log">Lade Log...</div>
<script>
let lastTs = 0;
async function refresh(){
  try{
    const r = await fetch('/api/status'); const s = await r.json();
    document.getElementById('s_fw').textContent = s.firmware;
    document.getElementById('s_wifi').textContent = s.wifi;
    document.getElementById('s_ip').textContent = s.ip;
    document.getElementById('s_rssi').textContent = s.rssi + ' dBm';
    document.getElementById('s_ws').textContent = s.ws ? '✅ verbunden' : '❌ getrennt';
    document.getElementById('s_up').textContent = s.uptime;
    document.getElementById('s_heap').textContent = s.heap + ' kB';
    document.getElementById('s_be').textContent = s.backend;
  }catch(e){}
  try{
    const r = await fetch('/api/log?since=' + lastTs); const j = await r.json();
    const el = document.getElementById('log');
    if(j.since !== undefined) lastTs = j.since;
    if(j.lines){ for(const l of j.lines){
      const div = document.createElement('div');
      div.innerHTML = '<span class="ln">' + l.ts + '</span><span class="t-' + (l.t||'log') + '">' + esc(l.m) + '</span>';
      el.appendChild(div);
    }}
    el.scrollTop = el.scrollHeight;
  }catch(e){}
}
function esc(s){return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');}
async function restart(){
  if(!confirm('Heizbox neu starten?')) return;
  await fetch('/api/restart',{method:'POST'});
}
async function checkUpdate(){
  try{
    const r = await fetch('/api/update',{method:'POST'}); const j = await r.json();
    alert(j.ok ? 'Update-Check gestartet' : 'Update-Check: ' + (j.error||'Fehler'));
  }catch(e){ alert('Update-Check fehlgeschlagen'); }
}
async function loadSettings(){
  try{
    const r = await fetch('/api/settings'); const s = await r.json();
    document.getElementById('set_brightness').value = s.brightness;
    document.getElementById('set_idle').value = s.idleBrightness;
    document.getElementById('s_brightness').textContent = s.brightness;
    document.getElementById('s_idle').textContent = s.idleBrightness;
  }catch(e){}
}
async function saveSettings(){
  try{
    const b = document.getElementById('set_brightness').value;
    const i = document.getElementById('set_idle').value;
    const r = await fetch('/api/settings',{method:'POST',body:'brightness='+b+'&idleBrightness='+i,headers:{'Content-Type':'application/x-www-form-urlencoded'}});
    const j = await r.json();
    if(!j.ok) alert('Fehler: '+(j.error||'?'));
    else loadSettings();
  }catch(e){ alert('Speichern fehlgeschlagen'); }
}
setInterval(refresh, 2000);
refresh();
loadSettings();
</script>
</body>
</html>
)rawliteral";

void DebugServer::init() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/api/status", HTTP_GET, [this]() { handleApiStatus(); });
    server.on("/api/log", HTTP_GET, [this]() { handleApiLog(); });
    server.on("/api/restart", HTTP_POST, [this]() {
        server.send(200, "application/json", "{\"ok\":true}");
        delay(100);
        ESP.restart();
    });
    server.on("/api/nettest", HTTP_GET, [this]() {
        handleApiNetTest();
    });
    server.on("/api/settings", HTTP_GET, [this]() { handleApiSettingsGet(); });
    server.on("/api/settings", HTTP_POST, [this]() { handleApiSettingsPost(); });
    server.on("/api/update", HTTP_POST, [this]() { handleApiUpdate(); });
    server.on("/api/ota", HTTP_POST,
        [this]() { handleApiOtaDone(); },
        [this]() { handleApiOtaUpload(); });
    server.onNotFound([this]() { handleNotFound(); });
    server.begin();
    logPrint("boot", "🔧 DebugServer auf Port 80 gestartet");
}

void DebugServer::update() {
    server.handleClient();
}

void DebugServer::handleRoot() {
    server.send_P(200, "text/html", PAGE_HTML);
}

void DebugServer::handleApiStatus() {
    auto& ws = WebSocketManager::instance();
    String json = "{";
    json += "\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",";
    json += "\"wifi\":\"" + String(WiFi.status() == WL_CONNECTED ? "connected" : "disconnected") + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"ws\":" + String(ws.isConnected() ? "true" : "false") + ",";
    json += "\"uptime\":\"" + String(millis() / 1000) + "s\",";
    json += "\"heap\":" + String(ESP.getFreeHeap() / 1024) + ",";
    json += "\"freeSketch\":" + String(ESP.getFreeSketchSpace()) + ",";
    json += "\"backend\":\"" + String(NetworkConfig::BACKEND_WS_URL) + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void DebugServer::handleApiLog() {
    uint32_t since = server.arg("since").toInt();
    String json = logRingJson(since);
    server.send(200, "application/json", json);
}

void DebugServer::handleApiNetTest() {
    String result = "{";
    // 1. DNS
    IPAddress ip;
    bool dnsOk = WiFi.hostByName("box.hzbx.de", ip);
    result += "\"dns\":" + String(dnsOk ? "true" : "false");
    result += ",\"ip\":\"" + (dnsOk ? ip.toString() : String("fail")) + "\"";

    // 2. TCP-Connect
    WiFiClient probe;
    bool tcpOk = probe.connect(ip, 443);
    result += ",\"tcp443\":" + String(tcpOk ? "true" : "false");
    if (tcpOk) probe.stop();

    // 3. TLS-Handshake
    WiFiClientSecure tls;
    tls.setInsecure();
    bool tlsOk = tls.connect("box.hzbx.de", 443);
    result += ",\"tls\":" + String(tlsOk ? "true" : "false");
    if (tlsOk) {
        result += ",\"tlsErr\":\"\"";
        tls.stop();
    } else {
        result += ",\"tlsErr\":\"" + String(tls.lastError(nullptr, 0)) + "\"";
    }

    // 4. HTTP GET auf box.hzbx.de (ohne WS)
    WiFiClientSecure http;
    http.setInsecure();
    if (http.connect("box.hzbx.de", 443)) {
        http.print("GET /health HTTP/1.1\r\nHost: box.hzbx.de\r\nConnection: close\r\n\r\n");
        String resp = http.readStringUntil('\n');
        result += ",\"http\":\"" + resp + "\"";
        http.stop();
    } else {
        result += ",\"http\":\"fail\"";
    }

    result += "}";
    server.send(200, "application/json", result);
}

void DebugServer::handleApiOtaDone() {
    if (otaTooBig_) {
        otaTooBig_ = false;
        logPrint("ota", "OTA abgebrochen: Datei zu gross");
        server.send(413, "application/json", "{\"ok\":false,\"error\":\"firmware too big for partition\"}");
        return;
    }
    if (Update.hasError()) {
        logPrint("ota", "OTA failed: %s", Update.errorString());
        server.send(500, "text/plain", String("OTA failed: ") + Update.errorString());
    } else {
        logPrint("ota", "OTA via DebugServer erfolgreich, reboot");
        server.send(200, "application/json", "{\"ok\":true}");
        delay(500);
        ESP.restart();
    }
}

void DebugServer::handleApiOtaUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        logPrint("ota", "OTA Start: %s", upload.filename.c_str());
        // totalSize ist bei multipart im ESP32-Core oft 0 -> max annehmen, echte Groesse tracken
        if (upload.totalSize > ESP.getFreeSketchSpace()) {
            logPrint("ota", "OTA size %u > freeSketch %u, abort", upload.totalSize, ESP.getFreeSketchSpace());
            Update.abort();
            otaTooBig_ = true;
            return;
        }
        otaReceived_ = 0;
        size_t sz = (upload.totalSize > 0) ? upload.totalSize : ESP.getFreeSketchSpace();
        if (!Update.begin(sz)) {
            logPrint("ota", "Update.begin failed: %s", Update.errorString());
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (otaTooBig_) return;
        otaReceived_ += upload.currentSize;
        if (otaReceived_ > ESP.getFreeSketchSpace()) {
            logPrint("ota", "OTA empfangene Bytes %u > freeSketch %u, abort", otaReceived_, ESP.getFreeSketchSpace());
            Update.abort();
            otaTooBig_ = true;
            return;
        }
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            logPrint("ota", "Update.write failed: %s", Update.errorString());
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (otaTooBig_) { logPrint("ota", "OTA aborted (too big)"); return; }
        if (otaReceived_ == 0) {
            logPrint("ota", "OTA leer, abort");
            Update.abort();
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"empty upload\"}");
            return;
        }
        if (!Update.end(true)) {
            logPrint("ota", "Update.end failed: %s", Update.errorString());
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        logPrint("ota", "OTA aborted");
    }
}

void DebugServer::handleApiSettingsGet() {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();
    String json = "{";
    json += "\"brightness\":" + String(ds.display.brightness.get()) + ",";
    json += "\"idleBrightness\":" + String(ds.display.idleBrightness.get()) + ",";
    json += "\"irSlope\":" + String(hs.irCalSlope.get(), 4) + ",";
    json += "\"irOffset\":" + String(hs.irCalOffset.get(), 2) + ",";
    json += "\"irCorrection\":" + String(hs.irCorrection.get()) + ",";
    json += "\"irMeasuredA\":" + String(hs.irCalMeasuredA.get()) + ",";
    json += "\"irMeasuredB\":" + String(hs.irCalMeasuredB.get()) + ",";
    json += "\"irActualA\":" + String(hs.irCalActualA.get()) + ",";
    json += "\"irActualB\":" + String(hs.irCalActualB.get());
    json += "}";
    server.send(200, "application/json", json);
}

void DebugServer::handleApiSettingsPost() {
    auto& ds = DeviceState::instance();
    auto& hs = HeaterState::instance();
    bool changed = false;
    if (server.hasArg("brightness")) {
        int v = server.arg("brightness").toInt();
        if (v < DisplayConfig::BRIGHTNESS_MIN || v > DisplayConfig::BRIGHTNESS_MAX) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"brightness out of range\"}");
            return;
        }
        ds.display.brightness.set((uint8_t)v);
        changed = true;
        logPrint("api", "brightness -> %d", v);
    }
    if (server.hasArg("idleBrightness")) {
        int v = server.arg("idleBrightness").toInt();
        if (v < DisplayConfig::BRIGHTNESS_MIN || v > DisplayConfig::BRIGHTNESS_MAX) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"idleBrightness out of range\"}");
            return;
        }
        ds.display.idleBrightness.set((uint8_t)v);
        changed = true;
        logPrint("api", "idleBrightness -> %d", v);
    }
    if (server.hasArg("clearCalibration") && server.arg("clearCalibration") == "1") {
        hs.irCalMeasuredA.set(0);
        hs.irCalMeasuredB.set(0);
        hs.irCalActualA.set(150);
        hs.irCalActualB.set(200);
        hs.irCalSlope.set(1.0f);
        hs.irCalOffset.set(0.0f);
        hs.irCorrection.set(0);
        changed = true;
        logPrint("api", "IR calibration cleared");
    }
    if (server.hasArg("irSlope")) {
        float v = server.arg("irSlope").toFloat();
        if (v < -10.0f || v > 10.0f) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irSlope out of range\"}"); return; }
        hs.irCalSlope.set(v); changed = true; logPrint("api", "irSlope -> %.4f", v);
    }
    if (server.hasArg("irOffset")) {
        float v = server.arg("irOffset").toFloat();
        if (v < -200.0f || v > 200.0f) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irOffset out of range\"}"); return; }
        hs.irCalOffset.set(v); changed = true; logPrint("api", "irOffset -> %.2f", v);
    }
    if (server.hasArg("irCorrection")) {
        int v = server.arg("irCorrection").toInt();
        if (v < -50 || v > 50) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irCorrection out of range\"}"); return; }
        hs.irCorrection.set((int16_t)v); changed = true; logPrint("api", "irCorrection -> %d", v);
    }
    if (server.hasArg("irMeasuredA")) {
        int v = server.arg("irMeasuredA").toInt();
        if (v < 0 || v > 1000) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irMeasuredA out of range\"}"); return; }
        hs.irCalMeasuredA.set((uint16_t)v); changed = true; logPrint("api", "irMeasuredA -> %d", v);
    }
    if (server.hasArg("irMeasuredB")) {
        int v = server.arg("irMeasuredB").toInt();
        if (v < 0 || v > 1000) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irMeasuredB out of range\"}"); return; }
        hs.irCalMeasuredB.set((uint16_t)v); changed = true; logPrint("api", "irMeasuredB -> %d", v);
    }
    if (server.hasArg("irActualA")) {
        int v = server.arg("irActualA").toInt();
        if (v < 0 || v > 1000) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irActualA out of range\"}"); return; }
        hs.irCalActualA.set((uint16_t)v); changed = true; logPrint("api", "irActualA -> %d", v);
    }
    if (server.hasArg("irActualB")) {
        int v = server.arg("irActualB").toInt();
        if (v < 0 || v > 1000) { server.send(400, "application/json", "{\"ok\":false,\"error\":\"irActualB out of range\"}"); return; }
        hs.irCalActualB.set((uint16_t)v); changed = true; logPrint("api", "irActualB -> %d", v);
    }
    if (changed) server.send(200, "application/json", "{\"ok\":true}");
    else server.send(400, "application/json", "{\"ok\":false,\"error\":\"no valid args\"}");
}

void DebugServer::handleApiUpdate() {
    if (!updateCb_) {
        server.send(500, "application/json", "{\"ok\":false,\"error\":\"no-update-callback\"}");
        return;
    }
    if (updateCb_()) {
        server.send(200, "application/json", "{\"ok\":true,\"update\":\"checking\"}");
    } else {
        server.send(429, "application/json", "{\"ok\":false,\"error\":\"rate-limited\"}");
    }
}

void DebugServer::handleNotFound() {
    server.send(404, "text/plain", "not found");
}
