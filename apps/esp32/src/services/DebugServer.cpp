#include "services/DebugServer.h"
#include "utils/Logger.h"
#include "driver/net/WebSocketManager.h"
#include "core/DeviceState.h"
#include "Config.h"

#include <WiFi.h>
#include <Update.h>
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
setInterval(refresh, 2000);
refresh();
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
    json += "\"backend\":\"" + String(NetworkConfig::BACKEND_WS_URL) + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void DebugServer::handleApiLog() {
    uint32_t since = server.arg("since").toInt();
    String json = logRingJson(since);
    server.send(200, "application/json", json);
}

void DebugServer::handleApiOtaDone() {
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
        logPrint("ota", "OTA Start: %s (%u bytes)", upload.filename.c_str(), upload.totalSize);
        if (!Update.begin(upload.totalSize)) {
            logPrint("ota", "Update.begin failed: %s", Update.errorString());
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            logPrint("ota", "Update.write failed: %s", Update.errorString());
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (!Update.end(true)) {
            logPrint("ota", "Update.end failed: %s", Update.errorString());
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        logPrint("ota", "OTA aborted");
    }
}

void DebugServer::handleNotFound() {
    server.send(404, "text/plain", "not found");
}
