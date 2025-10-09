// apps/backend/src/durable-objects/DeviceStatus.ts
import { Hono } from 'hono';

export class DeviceStatus {
  state: DurableObjectState;
  app: Hono;
  isOn: boolean = false; // Default status
  isHeating: boolean = false; // New: Default heating status
  subscribers: Set<WebSocket> = new Set(); // Store connected WebSocket clients (subscribers)

  constructor(state: DurableObjectState) {
    this.state = state;
    this.app = new Hono();

    // Initialize the status from storage when the Durable Object is created
    this.state.blockConcurrencyWhile(async () => {
      const storedIsOn = await this.state.storage.get<boolean>('isOn');
      this.isOn = storedIsOn !== undefined ? storedIsOn : false;

      const storedIsHeating = await this.state.storage.get<boolean>('isHeating');
      this.isHeating = storedIsHeating !== undefined ? storedIsHeating : false;
    });

    // Define routes for the Durable Object
    this.app.get('/status', (c) => {
      return c.json({ isOn: this.isOn, isHeating: this.isHeating });
    });

    this.app.post('/status', async (c) => {
      const { isOn, isHeating } = await c.req.json();
      let statusChanged = false;

      if (typeof isOn === 'boolean' && this.isOn !== isOn) {
        this.isOn = isOn;
        await this.state.storage.put('isOn', this.isOn);
        statusChanged = true;
      }

      if (typeof isHeating === 'boolean' && this.isHeating !== isHeating) {
        this.isHeating = isHeating;
        await this.state.storage.put('isHeating', this.isHeating);
        statusChanged = true;
      }

      if (statusChanged) {
        this.publish({ type: 'statusUpdate', isOn: this.isOn, isHeating: this.isHeating }); // Publish status update
      }
      return c.json({ success: true, isOn: this.isOn, isHeating: this.isHeating });
    });

    this.app.post('/publish', async (c) => {
      const message = await c.req.json();
      this.publish(message);
      return c.json({ success: true });
    });
  }

  // Method to publish messages to all connected subscribers
  publish(message: any) {
    const messageString = JSON.stringify(message);
    this.subscribers.forEach((ws) => {
      try {
        ws.send(messageString);
      } catch (err) {
        console.error('Error sending message to subscriber:', err);
        this.subscribers.delete(ws); // Remove broken connection
      }
    });
  }

  async fetch(request: Request): Promise<Response> {
    const upgradeHeader = request.headers.get('Upgrade');
    if (upgradeHeader === 'websocket') {
      const webSocketPair = new WebSocketPair();
      const [client, server] = Object.values(webSocketPair);

      server.accept();
      this.subscribers.add(server);

      server.addEventListener('message', async (event) => {
        // Handle messages from subscribers if needed (e.g., for topic subscriptions)
        console.log('Received message from subscriber:', event.data);
      });

      server.addEventListener('close', () => {
        this.subscribers.delete(server);
        console.log('Subscriber WebSocket closed');
      });

      server.addEventListener('error', (err) => {
        this.subscribers.delete(server);
        console.error('Subscriber WebSocket error:', err);
      });

      return new Response(null, { status: 101, webSocket: client });
    }

    // If not a WebSocket upgrade, handle as a regular HTTP request
    return this.app.fetch(request);
  }
}
