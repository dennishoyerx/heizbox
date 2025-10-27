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
import stashRoute from "./routes/stash.js";
import stashWithdrawalsRoute from "./routes/stashWithdrawals.js";

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
app.route("/api/stash", stashRoute);
app.route("/api/stash_withdrawals", stashWithdrawalsRoute);

export default Sentry.withSentry(
  (env: Env) => ({
    dsn: config.sentry.dsn,
    release: env.CF_VERSION_METADATA?.id,
    sendDefaultPii: true,

    // NEU: Sampling für Performance-Optimierung
    tracesSampleRate: 0.1, // 10% der Requests tracen
    beforeSend(event, hint) {
      // Filtere nicht-kritische Errors
      const error = hint.originalException;
      if (error instanceof Error) {
        // ValidationErrors nicht an Sentry senden
        if (error.name === 'ValidationError') {
          return null;
        }
      }
      return event;
    },
  }),
  app,
);

export { DeviceStatus } from "./durable-objects/DeviceStatus.js";
