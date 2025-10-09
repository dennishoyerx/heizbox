import { Hono } from 'hono';
import { cors } from 'hono/cors';
import { handleGetSessions, handleGetJson, handleCreateSession, handleStaticAssets } from './handlers';
import { handleWebSocket } from './handlers/websocket';
import { handleHeartbeat } from './routes/heartbeat';

const app = new Hono<{ Bindings: Env }>();

// ... existing code ...

app.get('/api/sessions', cors({
  origin: ['https://heizbox-frontend.pages.dev', 'http://localhost:5173', 'http://127.0.0.1:5173'],
  allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests'],
  allowMethods: ['POST', 'GET', 'OPTIONS'],
  exposeHeaders: ['Content-Length'],
  maxAge: 600,
  credentials: true,
}), handleGetSessions);
app.get('/api/json', handleGetJson);
app.get('/api/create', handleCreateSession);
app.get('/api/add', handleCreateSession); // Legacy endpoint
app.get('/ws/status', handleWebSocket);

// New route for heartbeat
app.post('/api/heartbeat/:deviceId', async (c) => {
  const deviceId = c.req.param('deviceId');
  const id = c.env.DEVICE_STATUS.idFromName(deviceId);
  const stub = c.env.DEVICE_STATUS.get(id);

  // Construct a new URL to pass to the Durable Object
  const url = new URL(c.req.url);
  url.pathname = `/process-device-message`; // Route to the DO's message processing endpoint

  // Forward the request to the Durable Object
  return stub.fetch(new Request(url.toString(), {
    method: 'POST',
    headers: c.req.headers,
    body: c.req.body,
  }));
});

// Route to interact with DeviceStatus Durable Object
app.all('/api/device-status/:deviceId/*', async (c) => {
  const deviceId = c.req.param('deviceId');
  const id = c.env.DEVICE_STATUS.idFromName(deviceId);
  const stub = c.env.DEVICE_STATUS.get(id);

  // Construct a new URL to pass to the Durable Object, removing the /api/device-status/:deviceId prefix
  const url = new URL(c.req.url);
  url.pathname = url.pathname.replace(`/api/device-status/${deviceId}`, '');

  // Forward the request to the Durable Object
  return stub.fetch(new Request(url.toString(), c.req));
});

// Static asset handler
app.get('/*', handleStaticAssets);

export default app;
export { DeviceStatus } from './durable-objects/DeviceStatus';
