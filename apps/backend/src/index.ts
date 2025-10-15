import { Hono } from "hono";
import { cors } from "hono/cors";
import * as Sentry from "@sentry/cloudflare";
import { HTTPException } from "hono/http-exception";

import statisticsRoute from "./routes/statistics";
import heatCyclesRoute from "./routes/heatCycles";
import jsonRoute from "./routes/json";
import heartbeatRoute from "./routes/heartbeat";
import deviceStatusRoute from "./routes/deviceStatus";
import websocketRoute from "./routes/websocket";
import sessionRoute from "./routes/session";

const app = new Hono<{ Bindings: Env }>();

app.onError((err, c) => {
  // Report _all_ unhandled errors.
  Sentry.captureException(err);
  if (err instanceof HTTPException) {
    return err.getResponse();
  }
  // Or just report errors which are not instances of HTTPException
  // Sentry.captureException(err);
  console.log(err);
  return c.json({ error: "Internal server error" }, 500);
});

app.use(
  cors({
    origin: (origin) => {
      if (
        origin.endsWith(".hzbx.de") ||
        origin === "https://hzbx.de" ||
        origin.endsWith(".heizbox.pages.dev") ||
        origin === "https://heizbox.pages.dev" ||
        origin === "http://localhost:5173" ||
        origin === "http://127.0.0.1:5173"
      ) {
        return origin;
      }

      return undefined;
    },
    allowHeaders: ["X-Custom-Header", "Upgrade-Insecure-Requests"],
    allowMethods: ["POST", "GET", "OPTIONS"],
    exposeHeaders: ["Content-Length"],
    credentials: true,
  }),
);
app.route("/api/heat_cycles", heatCyclesRoute);
app.route("/api/json", jsonRoute);
app.route("/api/statistics", statisticsRoute);
app.route("/api/heartbeat", heartbeatRoute);
app.route("/api/device-status", deviceStatusRoute);
app.route("/ws", websocketRoute);
app.route("/api/session", sessionRoute);

// Static asset handler
export default Sentry.withSentry(
  (env: Env) => {
    const { id: versionId } = env.CF_VERSION_METADATA;

    return {
      dsn: "https://4d0dd22715fe1480def3e8d645856914@o4510082700345344.ingest.de.sentry.io/4510173369925712",

      release: versionId,

      // Adds request headers and IP for users, for more info visit:
      // https://docs.sentry.io/platforms/javascript/guides/cloudflare/configuration/options/#sendDefaultPii
      sendDefaultPii: true,
    };
  },
  // your existing worker export
  app,
);
export { DeviceStatus } from "./durable-objects/DeviceStatus";
