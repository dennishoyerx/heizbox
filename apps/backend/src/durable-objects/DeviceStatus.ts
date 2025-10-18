import { Hono } from 'hono'
import { SessionService } from '../services/sessionService.js' // Import SessionService
import type { SessionData } from '@heizbox/types'
import { HeatCycleService } from '../services/heatCycleService.js'

/**
 * Einfacher In-Memory-Cache für kürzlich gesehene Heat-Cycle-IDs
 * Verhindert doppelte DB-Queries zur Duplikaterkennung
 */
class RecentCycleCache {
  private cache: Set<string> = new Set();
  private readonly maxSize = 100;

  has(duration: number, cycle: number): boolean {
    const key = `${duration}-${cycle}`;
    return this.cache.has(key);
  }

  add(duration: number, cycle: number): void {
    const key = `${duration}-${cycle}`;
    this.cache.add(key);

    // FIFO: Entferne ältesten Eintrag bei Überlauf
    if (this.cache.size > this.maxSize) {
      const firstKey = this.cache.values().next().value;
      this.cache.delete(firstKey);
    }
  }
}

export class DeviceStatus {
	state: DurableObjectState
	app: Hono
	env: Env // Add env member variable
	isOn = false // Default status
	isHeating = false // New: Default heating status
	lastSeen = 0 // Timestamp of last heartbeat
	currentSessionClicks = 0 // New: Clicks in current heating session
	currentSessionLastClick = 0 // New: Timestamp of last click in current heating session
	currentSessionStart = 0 // New: Timestamp of current heating session start
	  subscribers: Map<WebSocket, { type: string | null }> = new Map() // Store connected WebSocket clients and their type
	
	  private recentCycleCache = new RecentCycleCache();
	  private readonly OFFLINE_THRESHOLD = 90 * 1000 // 90 seconds
	
	  // Cache für Session-Daten (TTL 5 Sekunden)
	  private sessionDataCache: {
	    data: Omit<SessionData, 'type'>;
	    timestamp: number;
	  } | null = null;
	  private readonly SESSION_CACHE_TTL = 5000; // 5 Sekunden
	constructor(state: DurableObjectState, env: Env) {
		// Accept env in constructor
		this.state = state
		this.app = new Hono()
		this.env = env // Store env

		this.initialize()

		// Define routes for the Durable Object
		this.app.get('/status', (c) => {
			return c.json({ isOn: this.isOn, isHeating: this.isHeating })
		})

		this.app.get('/session-data', (c) => {
			return c.json({
				clicks: this.currentSessionClicks,
				lastClick: this.currentSessionLastClick,
				sessionStart: this.currentSessionStart,
			})
		})

		this.app.post('/status', async (c) => {
			const { isOn, isHeating } = await c.req.json()
			let statusChanged = false

			if (typeof isOn === 'boolean' && this.isOn !== isOn) {
				this.isOn = isOn
				await this.state.storage.put('isOn', this.isOn)
				statusChanged = true
			}

			if (typeof isHeating === 'boolean' && this.isHeating !== isHeating) {
				this.isHeating = isHeating
				await this.state.storage.put('isHeating', this.isHeating)
				statusChanged = true
			}

			if (statusChanged) {
				this.publish({
					type: 'statusUpdate',
					isOn: this.isOn,
					isHeating: this.isHeating,
				}) // Publish status update
			}
			return c.json({
				success: true,
				isOn: this.isOn,
				isHeating: this.isHeating,
			})
		})

		this.app.post('/publish', async (c) => {
			const message = await c.req.json()
			console.log('DeviceStatus: Received message for publish:', message)

			if (
				message.type === 'statusUpdate' &&
				typeof message.isOn === 'boolean' &&
				typeof message.isHeating === 'boolean'
			) {
				let statusChanged = false
				if (this.isOn !== message.isOn) {
					this.isOn = message.isOn
					await this.state.storage.put('isOn', this.isOn)
					statusChanged = true
				}
				if (this.isHeating !== message.isHeating) {
					this.isHeating = message.isHeating
					await this.state.storage.put('isHeating', this.isHeating)
					statusChanged = true
				}
				if (statusChanged) {
					console.log('DeviceStatus: State updated from WebSocket message.', {
						isOn: this.isOn,
						isHeating: this.isHeating,
					})
				}
			} else if (message.type === 'heatCycleCompleted' && typeof message.duration === 'number') {
				console.log('DeviceStatus: Processing heatCycleCompleted message.', message)
				const heatCycleService = new HeatCycleService(this.env.db)
				await heatCycleService.createHeatCycle(message.duration, message.cycle || 1) // Use this.env.db
			}

			this.publish(message)
			return c.json({ success: true })
		})
	}

	async initialize() {
		// Initialize the status from storage when the Durable Object is created
		return this.state.blockConcurrencyWhile(async () => {
			const storedIsOn = await this.state.storage.get<boolean>('isOn')
			this.isOn = storedIsOn !== undefined ? storedIsOn : false

			const storedIsHeating = await this.state.storage.get<boolean>('isHeating')
			this.isHeating = storedIsHeating !== undefined ? storedIsHeating : false

			const storedLastSeen = await this.state.storage.get<number>('lastSeen')
			this.lastSeen = storedLastSeen !== undefined ? storedLastSeen : 0

			const storedSessionClicks = await this.state.storage.get<number>('currentSessionClicks')
			this.currentSessionClicks = storedSessionClicks !== undefined ? storedSessionClicks : 0

			const storedSessionLastClick = await this.state.storage.get<number>('currentSessionLastClick')
			this.currentSessionLastClick = storedSessionLastClick !== undefined ? storedSessionLastClick : 0

			const storedSessionStart = await this.state.storage.get<number>('currentSessionStart')
			this.currentSessionStart = storedSessionStart !== undefined ? storedSessionStart : 0

			// Set an alarm to periodically check for offline devices
			const currentAlarm = await this.state.storage.getAlarm()
			if (currentAlarm === null) {
				await this.state.storage.setAlarm(Date.now() + this.OFFLINE_THRESHOLD)
			}
		})
	}

	  private async _getLatestSessionData(): Promise<Omit<SessionData, 'type'>> {
	    const now = Date.now();
	
	    // Cache-Hit: Gebe gecachte Daten zurück
	    if (
	      this.sessionDataCache &&
	      (now - this.sessionDataCache.timestamp) < this.SESSION_CACHE_TTL
	    ) {
	      console.log('DeviceStatus: Returning cached session data');
	      return this.sessionDataCache.data;
	    }
	
	    // Cache-Miss: Lade Daten aus DB
	    console.log('DeviceStatus: Fetching fresh session data');
	    const sessionService = new SessionService(this.env.db);
	    const data = await sessionService.getCurrentSessionData();
	
	    // Aktualisiere Cache
	    this.sessionDataCache = { data, timestamp: now };
	    return data;
	  }
	// Method to send current session data to a specific WebSocket
	async sendSessionData(ws: WebSocket) {
		const sessionDataPayload = await this._getLatestSessionData()

		// For devices, we don't want to send the large heat_cycles array
		const { heat_cycles, ...rest } = sessionDataPayload

		const sessionData: SessionData & { type: string } = {
			type: 'sessionData',
			...rest,
		}

		try {
			ws.send(JSON.stringify(sessionData))
		} catch (err) {
			console.error('Error sending session data to WebSocket:', err)
			this.subscribers.delete(ws) // Remove broken connection
		}
	}

	// Method to publish messages to all connected subscribers
  publish(message: any) {
    if (this.subscribers.size === 0) return; // Early exit

    // Pre-serialize base message einmalig
    const baseMessageString = JSON.stringify(message);

    this.subscribers.forEach((meta, ws) => {
      let finalMessage = baseMessageString;

      // Nur bei Bedarf neu serialisieren
      if (message.type === 'sessionData' && meta.type === 'device') {
        const { heat_cycles, ...rest } = message;
        finalMessage = JSON.stringify(rest);
      }

      try {
        ws.send(finalMessage);
      } catch (err) {
        console.error('DeviceStatus: Failed to send message to subscriber:', err);
        this.subscribers.delete(ws);
      }
    });
  }

  /**
   * Optimierte Heartbeat-Verarbeitung ohne HTTP-Overhead
   */
  async handleHeartbeat(): Promise<void> {
    const now = Date.now();
    this.lastSeen = now;
    await this.state.storage.put('lastSeen', now);

    console.log('DeviceStatus: Heartbeat processed, lastSeen:', now);

    // Gerät war offline → sende Status-Update
    if (!this.isOn) {
      this.isOn = true;
      await this.state.storage.put('isOn', true);

      this.publish({
        type: 'statusUpdate',
        isOn: this.isOn,
        isHeating: this.isHeating,
      });

    }

    // NEU: Stelle sicher, dass Alarm läuft
    const currentAlarm = await this.state.storage.getAlarm();
    if (currentAlarm === null) {
      await this.state.storage.setAlarm(now + this.OFFLINE_THRESHOLD);
      console.log('DeviceStatus: Alarm reactivated');
    }
  }

	async fetch(request: Request): Promise<Response> {
		console.log('DeviceStatus fetch called')
		const upgradeHeader = request.headers.get('Upgrade')
		if (upgradeHeader === 'websocket') {
			const webSocketPair = new WebSocketPair()
			const [client, server] = Object.values(webSocketPair)

			server.accept()
			// Add the new subscriber to the Map with its connection type
			const url = new URL(request.url)
			const connectionType = url.searchParams.get('type')
			this.subscribers.set(server, { type: connectionType })

			console.log('WS connected:', connectionType)

			if (connectionType === 'device') {
				// Fire-and-forget async call to send initial data without blocking
				;(async () => {
					try {
						await this.sendSessionData(server)
					} catch (e) {
						console.error('Failed to send initial session data:', e)
					}
				})()
			}

			server.addEventListener('message', async (event) => {
				try {
					const message = JSON.parse(event.data as string)
					console.log('Received message from subscriber:', message)
					if (connectionType === 'device') {
						this.processDeviceMessage(request, message)
					}
				} catch (error) {
					console.error('Error in DeviceStatus WebSocket message handler:', error)
				}
			})

			server.addEventListener('close', () => {
				try {
					this.subscribers.delete(server)
					console.log('Subscriber WebSocket closed')
				} catch (error) {
					console.error('Error in DeviceStatus WebSocket close handler:', error)
				}
			})

			server.addEventListener('error', (err) => {
				try {
					this.subscribers.delete(server)
					console.error('Subscriber WebSocket error:', err)
				} catch (error) {
					console.error('Error in DeviceStatus WebSocket error handler:', error)
				}
			})

			return new Response(null, { status: 101, webSocket: client })
		}

		// Handle HTTP requests for status updates and device messages
		if (request.url.endsWith('/process-device-message')) {
			return this.processDeviceMessage(request)
		}

		// If not a WebSocket upgrade or device message, handle as a regular HTTP request
		return this.app.fetch(request)
	}

	// New method to process messages coming from devices
	async processDeviceMessage(request: Request, message?: any): Promise<Response> {
		// If message is not provided, it's likely an HTTP call, so parse the body.
		if (!message) {
			try {
				message = await request.json()
			} catch (e) {
				return new Response('Invalid JSON in request body', { status: 400 })
			}
		}

		console.log('!!!!!DeviceStatus: Processing device message:', message)
		if (message.type === 'statusUpdate') {
			let statusChanged = false

			if (typeof message.isOn === 'boolean' && this.isOn !== message.isOn) {
				this.isOn = message.isOn
				await this.state.storage.put('isOn', this.isOn)
				statusChanged = true
			}

			if (typeof message.isHeating === 'boolean' && this.isHeating !== message.isHeating) {
				this.isHeating = message.isHeating
				await this.state.storage.put('isHeating', this.isHeating)
				statusChanged = true
			}

			if (statusChanged) {
				console.log('DeviceStatus: State updated from device message.', {
					isOn: this.isOn,
					isHeating: this.isHeating,
				})
				this.publish(message) // Publish the status update to all subscribers
			}
		} else if (message.type === 'heartbeat') {
			this.lastSeen = Date.now()
			await this.state.storage.put('lastSeen', this.lastSeen)
			console.log('DeviceStatus: Heartbeat received. lastSeen updated to', this.lastSeen)
			// Optionally, publish a status update if the device was previously considered offline
			if (!this.isOn) {
				this.isOn = true
				await this.state.storage.put('isOn', this.isOn)
				this.publish({
					type: 'statusUpdate',
					isOn: this.isOn,
					isHeating: this.isHeating,
				})
			}
		    } else if (message.type === 'heatCycleCompleted' && typeof message.duration === 'number') {
		      console.log('DeviceStatus: Processing heatCycleCompleted message.', message);
		
		      // Schneller Cache-Check vor DB-Abfrage
		      if (this.recentCycleCache.has(message.duration, message.cycle || 1)) {
		        console.log('DeviceStatus: Duplicate detected in cache, skipping');
		        return new Response(JSON.stringify({ success: false, reason: 'duplicate' }), {
		          headers: { 'Content-Type': 'application/json' },
		        });
		      }
		
		      const heatCycleService = new HeatCycleService(this.env.db);
		      const success = await heatCycleService.createHeatCycle(
		        message.duration,
		        message.cycle || 1
		      );
		
		      if (success) {
		        // Füge zu Cache hinzu
		        this.recentCycleCache.add(message.duration, message.cycle || 1);
		
		        // Invalidiere Session-Cache
		        this.sessionDataCache = null;
		
		        const newSessionData = await this._getLatestSessionData();
		        this.publish({ type: 'sessionData', ...newSessionData });
		      }
		    } else if (message.type === 'stashUpdated') {
			console.log('DeviceStatus: Processing stashUpdated message.', message)
			this.publish(message) // Broadcast to all subscribers
		}

		return new Response(JSON.stringify({ success: true }), {
			headers: { 'Content-Type': 'application/json' },
		})
	}

	  async alarm() {
	    console.log('DeviceStatus: Alarm triggered.');
	    const now = Date.now();
	    const timeSinceLastSeen = now - this.lastSeen;
	
	    // Gerät ist offline geworden
	    if (this.isOn && timeSinceLastSeen > this.OFFLINE_THRESHOLD) {
	      console.log('DeviceStatus: Device is offline. Setting isOn to false.');
	
	      this.isOn = false;
	      await this.state.storage.put('isOn', this.isOn);
	
	      this.publish({
	        type: 'statusUpdate',
	        isOn: this.isOn,
	        isHeating: this.isHeating,
	      });
	    }
	
	    // Schedule nächsten Alarm nur wenn Gerät online ist
	    // (Reduziert unnötige DO-Wakeups)
	    if (this.isOn) {
	      await this.state.storage.setAlarm(now + this.OFFLINE_THRESHOLD);
	    } else {
	      console.log('DeviceStatus: Device offline, skipping alarm reschedule');
	    }
	  }}
