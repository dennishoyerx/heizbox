import { Hono } from "hono";
import { HTTPException } from "hono/http-exception";
import { corsMiddleware } from "./middleware/cors.js";
import { ValidationError } from "./utils/validation.js";
import type { Context } from "hono";

import statisticsRoute from "./routes/statistics.js";
import heatCyclesRoute from "./routes/heatCycles.js";
import jsonRoute from "./routes/json.js";
import heartbeatRoute from "./routes/heartbeat.js";
import deviceStatusRoute from "./routes/deviceStatus.js";
import websocketRoute from "./routes/websocket.js";
import sessionRoute from "./routes/session.js";
import stashRoute from "./routes/stash.js";
import stashWithdrawalsRoute from "./routes/stashWithdrawals.js";
import logsRoute from "./routes/logs.js";
import tempReadingsRoute from "./routes/tempReadings.js";

// Lokaler Error-Handler (ohne Sentry - nur für den VPS-Betrieb)
export const errorHandler = (err: Error, c: Context) => {
  if (err instanceof ValidationError) {
    return c.json({ error: err.message }, 400);
  }
  if (err instanceof HTTPException) {
    return err.getResponse();
  }
  console.error("Unhandled error:", err);
  return c.json({ error: "Internal server error" }, 500);
};

export function createApp(env: Env) {
  const app = new Hono<{ Bindings: Env }>();

  app.onError(errorHandler);
  app.use(corsMiddleware);

  app.route("/api/heat_cycles", heatCyclesRoute);
  app.route("/api/json", jsonRoute);
  app.route("/api/statistics", statisticsRoute);
  app.route("/api/heartbeat", heartbeatRoute);
  app.route("/api/device-status", deviceStatusRoute);
  app.route("/ws", websocketRoute);
  app.route("/api/session", sessionRoute);
  app.route("/api/stash", stashRoute);
  app.route("/api/stash_withdrawals", stashWithdrawalsRoute);
  app.route("/api/logs", logsRoute);
  app.route("/api/temp_readings", tempReadingsRoute);

  app.get("/health", (c) => c.json({ ok: true, service: "heizbox-backend" }));

  return app;
}
