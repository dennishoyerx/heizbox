import type { Context } from 'hono';
import type { WebSocketMessage } from '@heizbox/types';

export const handleWebSocket = async (c: Context<{ Bindings: Env }>) => {
  const upgradeHeader = c.req.header('Upgrade');
  if (upgradeHeader !== 'websocket') {
    return c.text('Expected WebSocket upgrade', 426);
  }

  const deviceId = c.req.query('deviceId');
  if (!deviceId) {
    return c.text('Missing deviceId query parameter', 400);
  }

  const connectionType = c.req.query('type'); // 'device' or 'frontend'

  // Get the Durable Object stub for this device
  const id = c.env.DEVICE_STATUS.idFromName(deviceId);
  const stub = c.env.DEVICE_STATUS.get(id);

  if (connectionType === 'frontend') {
    // If it's a frontend connection, forward the WebSocket upgrade request to the Durable Object
    return stub.fetch(c.req.raw);
  } else { // Assume 'device' if type is not specified or is 'device'
    const webSocketPair = new WebSocketPair();
    const [client, server] = Object.values(webSocketPair);

    server.accept();

    // Fetch initial session data from Durable Object and send to device
    const sessionDataResponse = await stub.fetch(new Request('http://dummy-host/session-data'));
    const sessionData = await sessionDataResponse.json();
    server.send(JSON.stringify({ type: 'sessionData', ...sessionData }));

    // Update device status to 'on' when connected
    await stub.fetch(new Request('http://dummy-host/status', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ isOn: true, isHeating: false }), // Include isHeating
    }));

    server.addEventListener('message', async (event) => {
      try {
        const message: WebSocketMessage = JSON.parse(event.data as string);
        console.log('Received WebSocket message from device:', deviceId, message);

    // Forward message to the Durable Object for processing
    await stub.fetch(new Request('http://dummy-host/process-device-message', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(message),
    }));

        } catch (error) {
          console.error('Error processing WebSocket message from device:', deviceId, error);
          server.send(JSON.stringify({ error: 'Invalid message format' }));
        }
      });
    server.addEventListener('close', async () => {
        console.log('WebSocket closed for device (handler):', deviceId);
        // Update device status to 'off' when disconnected
        try {
          await stub.fetch(new Request('http://dummy-host/status', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ isOn: false, isHeating: false }), // Include isHeating
          }));
          console.log('Device status updated to offline for device:', deviceId);
        } catch (error) {
          console.error('Error updating device status to offline for device:', deviceId, error);
        }
      });

    server.addEventListener('error', (error) => {
      console.error('WebSocket error for device:', deviceId, error);
    });

    return new Response(null, { status: 101, webSocket: client });
  }
};

