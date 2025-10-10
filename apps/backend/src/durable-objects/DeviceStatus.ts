import { Hono } from 'hono';
import { createHeatCycle } from '../lib/session';
import type { SessionData } from '@heizbox/types';

export class DeviceStatus {
  state: DurableObjectState;
  app: Hono;
  env: Env; // Add env member variable
  isOn = false; // Default status
  isHeating = false; // New: Default heating status
  lastSeen = 0; // Timestamp of last heartbeat
  currentSessionClicks = 0; // New: Clicks in current heating session
  currentSessionLastClick = 0; // New: Timestamp of last click in current heating session
  currentSessionStart = 0; // New: Timestamp of current heating session start
  subscribers: Set<WebSocket> = new Set(); // Store connected WebSocket clients (subscribers)

  private readonly OFFLINE_THRESHOLD = 90 * 1000; // 90 seconds

  constructor(state: DurableObjectState, env: Env) { // Accept env in constructor
    this.state = state;
    this.app = new Hono();
    this.env = env; // Store env

    // Initialize the status from storage when the Durable Object is created
    this.state.blockConcurrencyWhile(async () => {
      const storedIsOn = await this.state.storage.get<boolean>('isOn');
      this.isOn = storedIsOn !== undefined ? storedIsOn : false;

      const storedIsHeating = await this.state.storage.get<boolean>('isHeating');
      this.isHeating = storedIsHeating !== undefined ? storedIsHeating : false;

      const storedLastSeen = await this.state.storage.get<number>('lastSeen');
      this.lastSeen = storedLastSeen !== undefined ? storedLastSeen : 0;

      const storedSessionClicks = await this.state.storage.get<number>('currentSessionClicks');
      this.currentSessionClicks = storedSessionClicks !== undefined ? storedSessionClicks : 0;

      const storedSessionLastClick = await this.state.storage.get<number>('currentSessionLastClick');
      this.currentSessionLastClick = storedSessionLastClick !== undefined ? storedSessionLastClick : 0;

      const storedSessionStart = await this.state.storage.get<number>('currentSessionStart');
      this.currentSessionStart = storedSessionStart !== undefined ? storedSessionStart : 0;

      // Set an alarm to periodically check for offline devices
      const currentAlarm = await this.state.storage.getAlarm();
      if (currentAlarm === null) {
        await this.state.storage.setAlarm(Date.now() + this.OFFLINE_THRESHOLD);
      }
    });

    // Define routes for the Durable Object
    this.app.get('/status', (c) => {
      return c.json({ isOn: this.isOn, isHeating: this.isHeating });
    });

    this.app.get('/session-data', (c) => {
      return c.json({
        clicks: this.currentSessionClicks,
        lastClick: this.currentSessionLastClick,
        sessionStart: this.currentSessionStart,
      });
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
      console.log('DeviceStatus: Received message for publish:', message);

      if (message.type === 'statusUpdate' && typeof message.isOn === 'boolean' && typeof message.isHeating === 'boolean') {
        let statusChanged = false;
        if (this.isOn !== message.isOn) {
          this.isOn = message.isOn;
          await this.state.storage.put('isOn', this.isOn);
          statusChanged = true;
        }
        if (this.isHeating !== message.isHeating) {
          this.isHeating = message.isHeating;
          await this.state.storage.put('isHeating', this.isHeating);
          statusChanged = true;
        }
        if (statusChanged) {
          console.log('DeviceStatus: State updated from WebSocket message.', { isOn: this.isOn, isHeating: this.isHeating });
        }
      } else if (message.type === 'heatCycleCompleted' && typeof message.duration === 'number') {
        console.log('DeviceStatus: Processing heatCycleCompleted message.', message);
        await createHeatCycle(this.env.db, message.duration, message.cycle || 1); // Use this.env.db
      }

      this.publish(message);
      return c.json({ success: true });
    });
  }

  // Method to send current session data to a specific WebSocket
  sendSessionData(ws: WebSocket) {
    const sessionData: SessionData & { type: string } = {
      type: 'sessionData',
      clicks: this.currentSessionClicks,
      lastClick: this.currentSessionLastClick,
      sessionStart: this.currentSessionStart,
    };
    try {
      ws.send(JSON.stringify(sessionData));
    } catch (err) {
      console.error('Error sending session data to WebSocket:', err);
      this.subscribers.delete(ws); // Remove broken connection
    }
  }

  // Method to publish messages to all connected subscribers
  publish(message: any) {
    console.log('DeviceStatus: Publishing message to subscribers:', message);
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
    console.log('DeviceStatus fetch called');
    const upgradeHeader = request.headers.get('Upgrade');
    if (upgradeHeader === 'websocket') {
      console.log('DeviceStatus: Upgrade header is websocket');
      const webSocketPair = new WebSocketPair();
      const [client, server] = Object.values(webSocketPair);

      server.accept();
      console.log('DeviceStatus: WebSocket server accepted');
      this.subscribers.add(server);

      // If it's a device connection, send current session data
      const url = new URL(request.url);
      const connectionType = url.searchParams.get('type');
      if (connectionType === 'device') {
        this.sendSessionData(server);
      }

      server.addEventListener('message', async (event) => {
        try {
          const message = JSON.parse(event.data as string);
          console.log('Received message from subscriber:', message);
          // For frontend subscribers, just publish the message
          this.publish(message);
        } catch (error) {
          console.error('Error in DeviceStatus WebSocket message handler:', error);
        }
      });

      server.addEventListener('close', () => {
        try {
          this.subscribers.delete(server);
          console.log('Subscriber WebSocket closed');
        } catch (error) {
          console.error('Error in DeviceStatus WebSocket close handler:', error);
        }
      });

      server.addEventListener('error', (err) => {
        try {
          this.subscribers.delete(server);
          console.error('Subscriber WebSocket error:', err);
        } catch (error) {
          console.error('Error in DeviceStatus WebSocket error handler:', error);
        }
      });

      return new Response(null, { status: 101, webSocket: client });
    }

    // Handle HTTP requests for status updates and device messages
    if (request.url.endsWith('/process-device-message')) {
      return this.processDeviceMessage(request);
    }

    // If not a WebSocket upgrade or device message, handle as a regular HTTP request
    return this.app.fetch(request);
  }

  // New method to process messages coming from devices
  async processDeviceMessage(request: Request): Promise<Response> {
    const rawBody = await request.text();
    console.log('DeviceStatus: Raw device message body:', rawBody);
    const message = JSON.parse(rawBody);
    console.log('DeviceStatus: Processing device message:', message);
    if (message.type === 'statusUpdate') {
      let statusChanged = false;

      if (typeof message.isOn === 'boolean' && this.isOn !== message.isOn) {
        this.isOn = message.isOn;
        await this.state.storage.put('isOn', this.isOn);
        statusChanged = true;
      }

      if (typeof message.isHeating === 'boolean' && this.isHeating !== message.isHeating) {
        this.isHeating = message.isHeating;
        await this.state.storage.put('isHeating', this.isHeating);
        statusChanged = true;
      }

      if (statusChanged) {
        console.log('DeviceStatus: State updated from device message.', { isOn: this.isOn, isHeating: this.isHeating });
        this.publish(message); // Publish the status update to all subscribers
      }
    } else if (message.type === 'heartbeat') {
      this.lastSeen = Date.now();
      await this.state.storage.put('lastSeen', this.lastSeen);
      console.log('DeviceStatus: Heartbeat received. lastSeen updated to', this.lastSeen);
      // Optionally, publish a status update if the device was previously considered offline
      if (!this.isOn) {
        this.isOn = true;
        await this.state.storage.put('isOn', this.isOn);
        this.publish({ type: 'statusUpdate', isOn: this.isOn, isHeating: this.isHeating });
      }
    } else if (message.type === 'heatCycleCompleted' && typeof message.duration === 'number') {
      console.log('DeviceStatus: Processing heatCycleCompleted message.', message);
      const success = await createHeatCycle(this.env.db, message.duration, message.cycle || 1);
      if (success) {
        this.publish({ type: 'sessionCreated' }); // Notify subscribers that a new session was created
      }
      this.publish(message); // Publish the heat cycle completed message to all subscribers
    } else if (message.type === 'sessionUpdate' && typeof message.clicks === 'number' && typeof message.lastClick === 'number' && typeof message.sessionStart === 'number') {
      let sessionDataChanged = false;
      if (this.currentSessionClicks !== message.clicks) {
        this.currentSessionClicks = message.clicks;
        await this.state.storage.put('currentSessionClicks', this.currentSessionClicks);
        sessionDataChanged = true;
      }
      if (this.currentSessionLastClick !== message.lastClick) {
        this.currentSessionLastClick = message.lastClick;
        await this.state.storage.put('currentSessionLastClick', this.currentSessionLastClick);
        sessionDataChanged = true;
      }
      if (this.currentSessionStart !== message.sessionStart) {
        this.currentSessionStart = message.sessionStart;
        await this.state.storage.put('currentSessionStart', this.currentSessionStart);
        sessionDataChanged = true;
      }
      if (sessionDataChanged) {
        console.log('DeviceStatus: Session data updated from device message.', { clicks: this.currentSessionClicks, lastClick: this.currentSessionLastClick, sessionStart: this.currentSessionStart });
        this.publish({ type: 'sessionData', clicks: this.currentSessionClicks, lastClick: this.currentSessionLastClick, sessionStart: this.currentSessionStart }); // Publish the session data update to all subscribers
      }
    }
    return new Response(JSON.stringify({ success: true }), { headers: { 'Content-Type': 'application/json' } });
  }

  async alarm() {
    console.log('DeviceStatus: Alarm triggered.');
    const now = Date.now();
    if (this.isOn && (now - this.lastSeen > this.OFFLINE_THRESHOLD)) {
      console.log('DeviceStatus: Device is offline. Setting isOn to false.');
      this.isOn = false;
      await this.state.storage.put('isOn', this.isOn);
      this.publish({ type: 'statusUpdate', isOn: this.isOn, isHeating: this.isHeating });
    }
    // Reschedule the alarm
    await this.state.storage.setAlarm(now + this.OFFLINE_THRESHOLD);
  }
}
