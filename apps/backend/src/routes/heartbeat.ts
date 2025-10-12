import { Hono } from 'hono';

const heartbeat = new Hono<{ Bindings: Env }>();

heartbeat.post('/:deviceId', async (c) => {
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

export default heartbeat;
