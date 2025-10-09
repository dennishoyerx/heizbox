import { Hono } from 'hono';
import { handleGetSessions, handleGetJson, handleCreateSession, handleStaticAssets } from './handlers';
import { handleWebSocket } from './handlers/websocket';

const app = new Hono<{ Bindings: Env }>();

// API Routes
app.get('/api/sessions', handleGetSessions);
app.get('/api/json', handleGetJson);
app.get('/api/create', handleCreateSession);
app.get('/api/add', handleCreateSession); // Legacy endpoint
app.get('/ws/status', handleWebSocket);

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
