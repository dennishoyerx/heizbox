import { Hono } from "hono";
import * as Sentry from "@sentry/cloudflare";
import { config } from "./config/index.js";
import { corsMiddleware } from "./middleware/cors.js";
import { errorHandler } from "./middleware/error.js";

// Routes
import statisticsRoute from "./routes/statistics.js";
import heatCyclesRoute from "./routes/heatCycles.js";
import jsonRoute from "./routes/json.js";
import heartbeatRoute from "./routes/heartbeat.js";
import deviceStatusRoute from "./routes/deviceStatus.js";
import websocketRoute from "./routes/websocket.js";
import sessionRoute from "./routes/session.js";

const app = new Hono<{ Bindings: Env }>();

// Error handling
app.onError(errorHandler);

// CORS middleware
app.use(corsMiddleware);

// Routes
app.route("/api/heat_cycles", heatCyclesRoute);
app.route("/api/json", jsonRoute);
app.route("/api/statistics", statisticsRoute);
app.route("/api/heartbeat", heartbeatRoute);
app.route("/api/device-status", deviceStatusRoute);
app.route("/ws", websocketRoute);
app.route("/api/session", sessionRoute);

export default Sentry.withSentry(
  (env: Env) => ({
    dsn: config.sentry.dsn,
    release: env.CF_VERSION_METADATA?.id,
    sendDefaultPii: true,
  }),
  app,
);

export { DeviceStatus } from "./durable-objects/DeviceStatus.js";
