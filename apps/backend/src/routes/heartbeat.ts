import { Hono } from 'hono';

const heartbeat = new Hono<{ Bindings: Env }>();

heartbeat.post('/:deviceId', async (c) => {
  const deviceId = c.req.param('deviceId');

  // Hole DO-Stub
  const id = c.env.DEVICE_STATUS.idFromName(deviceId);
  const stub = c.env.DEVICE_STATUS.get(id);

  // Direkter Methodenaufruf statt fetch()
  await stub.handleHeartbeat();

  return c.json({ success: true });
});

export default heartbeat;
