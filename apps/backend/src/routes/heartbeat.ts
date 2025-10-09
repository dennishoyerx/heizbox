import { Hono } from 'hono';

export const handleHeartbeat = async (c: Hono.Context) => {
  const deviceId = c.req.param('deviceId');
  // The actual logic to interact with the Durable Object will be in index.ts
  return c.json({ message: `Heartbeat received for device: ${deviceId}` });
};