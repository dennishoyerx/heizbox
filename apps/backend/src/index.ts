import { Hono } from 'hono';
import { cors } from 'hono/cors';
import { handleGetSessions, handleGetJson, handleCreateSession, handleStaticAssets } from './handlers';
import { handleWebSocket } from './handlers/websocket';

const app = new Hono<{ Bindings: Env }>();

app.use(
  '/api/*',
  cors({
    origin: ['https://heizbox-frontend.pages.dev', 'http://localhost:5173', 'http://127.0.0.1:5173'],
    allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests'],
    allowMethods: ['POST', 'GET', 'OPTIONS'],
    exposeHeaders: ['Content-Length'],
    maxAge: 600,
    credentials: true,
  })
);

app.use(
  '/ws/*',
  cors({
    origin: ['https://heizbox-frontend.pages.dev', 'http://localhost:5173', 'http://127.0.0.1:5173'],
    allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests'],
    allowMethods: ['GET', 'OPTIONS'],
    exposeHeaders: ['Content-Length'],
    maxAge: 600,
    credentials: true,
  })
);

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
