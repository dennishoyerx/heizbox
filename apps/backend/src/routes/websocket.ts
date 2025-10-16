import { Hono } from 'hono';
import { validateDeviceId, validateConnectionType } from '../utils/validation';

const websocket = new Hono<{ Bindings: Env }>();

websocket.get('/', async (c) => {
  const deviceId = validateDeviceId(c.req.query('deviceId'));
  const type = validateConnectionType(c.req.query('type'));

  const durableObjectId = c.env.DEVICE_STATUS.idFromName(deviceId);
  const durableObjectStub = c.env.DEVICE_STATUS.get(durableObjectId);

  const url = new URL(c.req.url);
  url.pathname = '/';

  const request = new Request(url.toString(), c.req.raw);

  return durableObjectStub.fetch(request);
});

export default websocket;