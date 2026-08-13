import type { ServerWebSocket } from "bun";
import type { SessionData } from "@heizbox/types";
import { SessionService } from "./services/sessionService.js";
import { HeatCycleService } from "./services/heatCycleService.js";
import type { DB } from "./db.js";

interface WsMeta {
  type: "frontend" | "device";
  deviceId: string;
}

/**
 * Ersetzt das Cloudflare Durable Object "DeviceStatus" auf dem lokalen Server.
 * Hält Status + WebSocket-Subscriber in Memory (kein Serverless, kein Storage nötig).
 */
export class DeviceStatusHub {
  isOn = false;
  isHeating = false;
  lastSeen = 0;
  currentSessionClicks = 0;
  currentSessionLastClick = 0;
  currentSessionStart = 0;

  private subscribers = new Map<ServerWebSocket<WsMeta>, WsMeta>();
  private sessionDataCache: { data: Omit<SessionData, "type">; timestamp: number } | null = null;
  private readonly SESSION_CACHE_TTL = 5000;
  private readonly OFFLINE_THRESHOLD = 90_000;

  constructor(private db: DB) {}

  // ---- WebSocket lifecycle (von Bun.serve aufgerufen) ----
  handleOpen(ws: ServerWebSocket<WsMeta>) {
    const meta = ws.data;
    this.subscribers.set(ws, meta);
    this.sendInitialStatus(ws);
    this.sendHeartbeat(ws);
    if (meta.type === "device") {
      void this.sendSessionData(ws);
    }
  }

  handleClose(ws: ServerWebSocket<WsMeta>) {
    this.subscribers.delete(ws);
  }

  handleMessage(ws: ServerWebSocket<WsMeta>, raw: string | Buffer) {
    let message: any;
    try {
      message = JSON.parse(raw.toString());
    } catch (err) {
      console.error("MSG PARSE ERROR:", err);
      return;
    }
    void this.processDeviceMessage(ws, message);
  }

  // ---- HTTP-Stub-Routen (wie DO fetch()) ----
  async fetch(request: Request): Promise<Response> {
    const url = new URL(request.url);
    switch (url.pathname) {
      case "/status":
        return Response.json({ isOn: this.isOn, isHeating: this.isHeating });
      case "/session-data":
        return Response.json({
          clicks: this.currentSessionClicks,
          lastClick: this.currentSessionLastClick,
          sessionStart: this.currentSessionStart,
        });
      case "/update-status": {
        const { isOn, isHeating } = (await request.json()) as { isOn?: boolean; isHeating?: boolean };
        let changed = false;
        if (isOn !== undefined && isOn !== this.isOn) { this.isOn = isOn; changed = true; }
        if (isHeating !== undefined && isHeating !== this.isHeating) { this.isHeating = isHeating; changed = true; }
        if (changed) this.publish({ type: "statusUpdate", isOn: this.isOn, isHeating: this.isHeating });
        return new Response("OK");
      }
      case "/send-message": {
        const message = (await request.json()) as Record<string, unknown>;
        this.publish(message);
        return new Response("OK");
      }
      default:
        return new Response("Not found", { status: 404 });
    }
  }

  // ---- Heartbeat (Route heartbeat.ts ruft das) ----
  async handleHeartbeat(): Promise<void> {
    const now = Date.now();
    this.lastSeen = now;
    if (!this.isOn) {
      this.isOn = true;
      this.publish({ type: "statusUpdate", isOn: this.isOn, isHeating: this.isHeating });
    }
  }

  // ---- Nachrichten vom Device verarbeiten ----
  private async processDeviceMessage(ws: ServerWebSocket<WsMeta>, message: any): Promise<void> {
    const deviceId = ws.data.deviceId || "unknown-device";

    if (message.t !== undefined) {
      await this.storeLog(message, deviceId);
      return;
    }
    if (message.type === "statusUpdate") {
      let changed = false;
      if (typeof message.isOn === "boolean" && this.isOn !== message.isOn) { this.isOn = message.isOn; changed = true; }
      if (typeof message.isHeating === "boolean" && this.isHeating !== message.isHeating) { this.isHeating = message.isHeating; changed = true; }
      if (changed) this.publish(message);
      ws.send(JSON.stringify({ success: true }));
      return;
    }
    if (message.type === "heartbeat") {
      this.lastSeen = Date.now();
      if (!this.isOn) {
        this.isOn = true;
        this.publish({ type: "statusUpdate", isOn: this.isOn, isHeating: this.isHeating });
      }
      ws.send(JSON.stringify({ success: true }));
      return;
    }
    if (message.type === "heatCycleCompleted" && typeof message.duration === "number") {
      const service = new HeatCycleService(this.db);
      const success = await service.createHeatCycle(message.duration, message.cycle || 1);
      if (success) {
        this.sessionDataCache = null;
        const newSessionData = await this._getLatestSessionData();
        this.publish({ type: "sessionData", ...newSessionData });
        ws.send(JSON.stringify({ success: true }));
      } else {
        ws.send(JSON.stringify({ success: false, reason: "db_error" }));
      }
      return;
    }
    if (message.type === "stashUpdated") {
      this.publish(message);
      ws.send(JSON.stringify({ success: true }));
      return;
    }
    if (message.type === "tempReading") {
      // RAW IR-Temp Logging
      await this.storeTempReading(ws.data.deviceId || "HeizboxESP32", message);
      ws.send(JSON.stringify({ success: true }));
      return;
    }
    ws.send(JSON.stringify({ success: true }));
  }

  private async storeLog(data: any, deviceId: string): Promise<void> {
    const { t: logType, m: message } = data;
    const id = `${Date.now()}-${Math.random().toString(36).substring(2, 15)}`;
    const timestamp = Date.now();
    if (!message) return;
    try {
      await this.db
        .prepare("INSERT INTO logs (id, device_id, log_type, message, timestamp) VALUES (?, ?, ?, ?, ?)")
        .bind(id, deviceId || "unknown-device", logType, message, timestamp)
        .run();
    } catch (err) {
      console.error("storeLog error:", err);
    }
  }

  private async storeTempReading(deviceId: string, msg: any): Promise<void> {
    try {
      await this.db
        .prepare(
          "INSERT INTO temp_readings (device_id, timestamp, temp_raw, temp_calibrated, is_heating) VALUES (?, ?, ?, ?, ?)"
        )
        .bind(
          deviceId,
          Date.now(),
          typeof msg.temp_raw === "number" ? msg.temp_raw : null,
          typeof msg.temp_calibrated === "number" ? msg.temp_calibrated : null,
          msg.is_heating ? 1 : 0
        )
        .run();
    } catch (err) {
      console.error("storeTempReading error:", err);
    }
  }

  private async _getLatestSessionData(): Promise<Omit<SessionData, "type">> {
    const now = Date.now();
    if (this.sessionDataCache && now - this.sessionDataCache.timestamp < this.SESSION_CACHE_TTL) {
      return this.sessionDataCache.data;
    }
    const service = new SessionService(this.db);
    const data = await service.getCurrentSessionData();
    this.sessionDataCache = { data, timestamp: now };
    return data;
  }

  private async sendSessionData(ws: ServerWebSocket<WsMeta>) {
    try {
      const payload = await this._getLatestSessionData();
      const { heat_cycles, ...rest } = payload;
      ws.send(JSON.stringify({ type: "sessionData", ...rest }));
    } catch (err) {
      console.error("sendSessionData error:", err);
    }
  }

  private sendInitialStatus(ws: ServerWebSocket<WsMeta>) {
    try {
      ws.send(JSON.stringify({ type: "statusUpdate", isOn: this.isOn, isHeating: this.isHeating }));
    } catch { /* ignore */ }
  }

  private sendHeartbeat(ws: ServerWebSocket<WsMeta>) {
    try {
      ws.send(JSON.stringify({ type: "heartbeat" }));
    } catch { /* ignore */ }
  }

  publish(message: any) {
    if (this.subscribers.size === 0) return;
    const base = JSON.stringify(message);
    this.subscribers.forEach((meta, ws) => {
      let final = base;
      if (message.type === "sessionData" && meta.type === "device") {
        const { heat_cycles, ...rest } = message;
        final = JSON.stringify(rest);
      }
      try {
        ws.send(final);
      } catch {
        this.subscribers.delete(ws);
      }
    });
  }
}

// ---- Env-Ersatz: DEVICE_STATUS Namespace + Stub (emuliert DO-API) ----
export class DeviceStatusNamespace {
  constructor(private hub: DeviceStatusHub) {}
  idFromName(_name: string) {
    return { name: _name } as never;
  }
  get(_id: never) {
    return new DeviceStatusStub(this.hub);
  }
}

class DeviceStatusStub {
  constructor(private hub: DeviceStatusHub) {}
  fetch(request: Request) {
    return this.hub.fetch(request);
  }
  handleHeartbeat() {
    return this.hub.handleHeartbeat();
  }
}
