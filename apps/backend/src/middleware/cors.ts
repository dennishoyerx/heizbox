import { cors } from 'hono/cors';
import { isOriginAllowed } from '../utils/cors';

export const corsMiddleware = cors({
  origin: (origin) => {
    if (isOriginAllowed(origin)) {
      return origin;
    }
    return undefined;
  },
  allowHeaders: ['X-Custom-Header', 'Upgrade-Insecure-Requests', 'Content-Type'],
  allowMethods: ['POST', 'GET', 'OPTIONS'],
  exposeHeaders: ['Content-Length'],
  credentials: true,
});
