import { Hono } from 'hono';
import { cors } from 'hono/cors';

import statisticsRoute from './routes/statistics';
import heatCyclesRoute from './routes/heatCycles';
import jsonRoute from './routes/json';
import heartbeatRoute from './routes/heartbeat';
import deviceStatusRoute from './routes/deviceStatus';
import websocketRoute from './routes/websocket';
import sessionRoute from './routes/session';

const app = new Hono<{ Bindings: Env }>();

app.use(cors({
  origin: (origin) => {
    if (origin.endsWith('.heizbox.pages.dev') || origin === 'https://heizbox.pages.dev' || origin === 'http://localhost:5173' || origin === 'http://127.0.0.1:5173') {
      return origin;
    }
    return undefined;
  },
  allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests'],
  allowMethods: ['POST', 'GET', 'OPTIONS'],
  exposeHeaders: ['Content-Length'],
  credentials: true,
}));

app.route('/api/heat_cycles', heatCyclesRoute);
app.route('/api/json', jsonRoute);
app.route('/api/statistics', statisticsRoute);
app.route('/api/heartbeat', heartbeatRoute);
app.route('/api/device-status', deviceStatusRoute);
app.route('/ws', websocketRoute);
app.route('/api/session', sessionRoute);

// Static asset handler
export default app;
export { DeviceStatus } from './durable-objects/DeviceStatus';