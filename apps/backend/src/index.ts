import { Hono } from 'hono';
import * as Sentry from '@sentry/cloudflare';
import { config } from './config';
import { corsMiddleware } from './middleware/cors';
import { errorHandler } from './middleware/error';

// Routes
import statisticsRoute from './routes/statistics';
import heatCyclesRoute from './routes/heatCycles';
import jsonRoute from './routes/json';
import heartbeatRoute from './routes/heartbeat';
import deviceStatusRoute from './routes/deviceStatus';
import websocketRoute from './routes/websocket';
import sessionRoute from './routes/session';

const app = new Hono<{ Bindings: Env }>();

// Error handling
app.onError(errorHandler);

// CORS middleware
app.use(corsMiddleware);

// Routes
app.route('/api/heat_cycles', heatCyclesRoute);
app.route('/api/json', jsonRoute);
app.route('/api/statistics', statisticsRoute);
app.route('/api/heartbeat', heartbeatRoute);
app.route('/api/device-status', deviceStatusRoute);
app.route('/ws', websocketRoute);
app.route('/api/session', sessionRoute);

// Export with Sentry integration
export default Sentry.withSentry(
  (env: Env) => ({
    dsn: config.sentry.dsn,
    release: env.CF_VERSION_METADATA?.id,
    sendDefaultPii: true,
  }),
  app
);

export { DeviceStatus } from './durable-objects/DeviceStatus';