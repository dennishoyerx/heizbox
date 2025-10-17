import { cors } from 'hono/cors';
import { isOriginAllowed } from '../utils/cors.js';

export const corsMiddleware = cors({
  origin: (origin) => {
    if (isOriginAllowed(origin)) {
      return origin;
    }
    return undefined;
  },
  allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests', 'Content-Type'],
  allowMethods: ['POST', 'GET', 'OPTIONS', 'DELETE', 'PUT'],
  exposeHeaders: ['Content-Length'],
  credentials: true,
});
