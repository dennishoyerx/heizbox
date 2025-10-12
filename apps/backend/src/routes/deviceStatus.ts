import { Hono } from 'hono';

const deviceStatus = new Hono<{ Bindings: Env }>();

deviceStatus.all('/:deviceId/*', async (c) => {
  const deviceId = c.req.param('deviceId');
  const id = c.env.DEVICE_STATUS.idFromName(deviceId);
  const stub = c.env.DEVICE_STATUS.get(id);

  // Construct a new URL to pass to the Durable Object, removing the /api/device-status/:deviceId prefix
  const url = new URL(c.req.url);
  url.pathname = url.pathname.replace(`/api/device-status/${deviceId}`, '');

  // Forward the request to the Durable Object
  return stub.fetch(new Request(url.toString(), c.req));
});

export default deviceStatus;
