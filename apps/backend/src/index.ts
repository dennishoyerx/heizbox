import { Hono } from 'hono';
import { cors } from 'hono/cors';

import statisticsRoute from './routes/statistics';
import heatCyclesRoute from './routes/heatCycles';
import jsonRoute from './routes/json';
import heartbeatRoute from './routes/heartbeat';
import deviceStatusRoute from './routes/deviceStatus';
import websocketRoute from './routes/websocket';

const app = new Hono<{ Bindings: Env }>();

app.use(cors({
  origin: ['https://heizbox.pages.dev', 'https://heizbox-frontend.pages.dev', 'http://localhost:5173', 'http://127.0.0.1:5173'],
  allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests'],
  allowMethods: ['POST', 'GET', 'OPTIONS'],
  exposeHeaders: ['Content-Length'],
  credentials: true,
}));

app.route('/api', heatCyclesRoute);
app.route('/api/json', jsonRoute);
app.route('/api/statistics', statisticsRoute);
app.route('/api/heartbeat', heartbeatRoute);
app.route('/api/device-status', deviceStatusRoute);
app.route('/ws', websocketRoute);

// Static asset handler
export default app;
export { DeviceStatus } from './durable-objects/DeviceStatus';