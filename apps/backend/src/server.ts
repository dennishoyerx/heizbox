import { createApp } from "./app.js";
import { createRuntime } from "./env.js";

const { env, hub } = createRuntime();
const app = createApp(env);

const port = Number(process.env.PORT || 3001);

const server = Bun.serve({
  port,
  fetch(req, srv) {
    const url = new URL(req.url);

    // WebSocket-Upgrade für /ws (wie DO vorher)
    if (url.pathname === "/ws" && req.headers.get("Upgrade") === "websocket") {
      const deviceId = url.searchParams.get("deviceId") || "HeizboxESP32";
      const type = url.searchParams.get("type") === "device" ? "device" : "frontend";
      const ok = srv.upgrade(req, { data: { deviceId, type } });
      if (ok) return;
      return new Response("Upgrade failed", { status: 400 });
    }

    return app.fetch(req, env);
  },
  websocket: {
    open(ws) {
      hub.handleOpen(ws);
    },
    message(ws, message) {
      hub.handleMessage(ws, message);
    },
    close(ws) {
      hub.handleClose(ws);
    },
  },
});

console.log(`Heizbox Backend läuft auf http://localhost:${port}`);
console.log(`Health: http://localhost:${port}/health`);
