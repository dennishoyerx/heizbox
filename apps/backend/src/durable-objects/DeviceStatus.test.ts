import { describe, it, expect, vi, beforeEach } from 'vitest';
import { DeviceStatus } from '../durable-objects/DeviceStatus.js';

// Mock Durable Object state and env
const createMockState = () => {
    const storage = new Map<string, any>();
    return {
        storage: {
            get: vi.fn(async (key) => storage.get(key)),
            put: vi.fn(async (key, value) => storage.set(key, value)),
            setAlarm: vi.fn(),
            getAlarm: vi.fn().mockResolvedValue(null), // Ensure setAlarm is called
            delete: vi.fn(async (key) => storage.delete(key)),
            deleteAll: vi.fn(async () => storage.clear()),
        },
        blockConcurrencyWhile: vi.fn(async (callback) => await callback()),
    };
};

const createMockEnv = () => ({
    db: {
        prepare: vi.fn().mockReturnThis(),
        bind: vi.fn().mockReturnThis(),
        run: vi.fn().mockResolvedValue({ success: true }),
        all: vi.fn().mockResolvedValue({ results: [] }),
        first: vi.fn().mockResolvedValue(null),
    },
});

// Mock WebSocket and WebSocketPair
class MockWebSocket {
    public sentMessages: string[] = [];
    public isClosed = false;
    private listeners: Record<string, ((event?: any) => void)[]> = {};

    accept() {}

    send(message: string) {
        if (this.isClosed) throw new Error("WebSocket is closed");
        this.sentMessages.push(message);
    }

    close() {
        this.isClosed = true;
        if (this.listeners['close']) {
            this.listeners['close'].forEach(l => l());
        }
    }

    addEventListener(type: string, listener: (event?: any) => void) {
        if (!this.listeners[type]) {
            this.listeners[type] = [];
        }
        this.listeners[type].push(listener);
    }

    // Method to simulate a message from the other side
    simulateMessage(data: string) {
        if (this.listeners['message']) {
            this.listeners['message'].forEach(l => l({ data }));
        }
    }
}

class MockWebSocketPair {
    client: MockWebSocket;
    server: MockWebSocket;
    constructor() {
        this.client = new MockWebSocket();
        this.server = new MockWebSocket();
    }
}

vi.stubGlobal('WebSocketPair', MockWebSocketPair);

describe('DeviceStatus Durable Object', () => {
    let state: any;
    let env: any;
    let deviceStatus: DeviceStatus;

    beforeEach(() => {
        state = createMockState();
        env = createMockEnv();
        deviceStatus = new DeviceStatus(state, env);
        vi.clearAllMocks();
    });

    it('should initialize state from storage on construction', async () => {
        const initialStorage = new Map<string, any>([['isOn', true], ['isHeating', false], ['lastSeen', 12345]]);
        state.storage.get.mockImplementation(async (key) => initialStorage.get(key));
        state.storage.setAlarm.mockClear(); // Clear previous calls if any

        const ds = new DeviceStatus(state, env);
        await ds.initialize(); // Trigger initialization

        expect(ds.isOn).toBe(true);
        expect(ds.isHeating).toBe(false);
        expect(ds.lastSeen).toBe(12345);
        expect(state.storage.setAlarm).toHaveBeenCalled();
    });

    describe('HTTP Fetch Handler', () => {
        it('should handle WebSocket upgrade and add subscriber', async () => {
            const req = new Request('http://do/ws?type=frontend', { headers: { 'Upgrade': 'websocket' } });
            
            // Mock the fetch method to avoid RangeError with status 101
            vi.spyOn(deviceStatus, 'fetch').mockImplementation(async (request) => {
                const [client, server] = Object.values(new WebSocketPair());
                // Simulate the DO's internal handling for WebSocket upgrade
                server.accept();
                const url = new URL(request.url);
                const connectionType = url.searchParams.get('type');
                deviceStatus.subscribers.set(server, { type: connectionType });
                // Return a plain object that mimics a Response, ensuring webSocket is accessible
                return { status: 200, webSocket: client } as Response;
            });

            const res = await deviceStatus.fetch(req);

            expect(res.webSocket).toBeDefined();
            expect(deviceStatus.subscribers.size).toBe(1);
            expect(deviceStatus.fetch).toHaveBeenCalledWith(req);
        });

        it('should get current status via GET /status', async () => {
            deviceStatus.isOn = true;
            const req = new Request('http://do/status');
            const res = await deviceStatus.fetch(req);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.isOn).toBe(true);
        });

        it('should update status via POST /status and publish', async () => {
            vi.spyOn(deviceStatus, 'publish');
            const req = new Request('http://do/status', { method: 'POST', body: JSON.stringify({ isOn: true, isHeating: false }) });
            await deviceStatus.fetch(req);

            expect(deviceStatus.isOn).toBe(true);
            expect(deviceStatus.isHeating).toBe(false);
            expect(state.storage.put).toHaveBeenCalledWith('isOn', true);
            expect(deviceStatus.publish).toHaveBeenCalledWith({ type: 'statusUpdate', isOn: true, isHeating: false });
        });
    });

    describe('WebSocket Message Processing', () => {
        let serverSocket: MockWebSocket;

        beforeEach(async () => {
            const req = new Request('http://do/ws?type=device', { headers: { 'Upgrade': 'websocket' } });
            
            // Mock the fetch method to avoid RangeError with status 101
            vi.spyOn(deviceStatus, 'fetch').mockImplementation(async (request) => {
                const [client, server] = Object.values(new WebSocketPair());
                // Simulate the DO's internal handling for WebSocket upgrade
                server.accept();
                const url = new URL(request.url);
                const connectionType = url.searchParams.get('type');
                deviceStatus.subscribers.set(server, { type: connectionType });
                // Return a plain object that mimics a Response, ensuring webSocket is accessible
                return { status: 200, webSocket: client } as Response;
            });

            const res = await deviceStatus.fetch(req);
            serverSocket = (res as any).webSocket;
        });

        it('should process heartbeat message', async () => {
            const message = { type: 'heartbeat' };
            deviceStatus.processDeviceMessage(new Request('http://do'), message);
            expect(state.storage.put).toHaveBeenCalledWith('lastSeen', expect.any(Number));
        });

        it('should process statusUpdate message and publish', async () => {
            const message = { type: 'statusUpdate', isOn: true, isHeating: true };
            vi.spyOn(deviceStatus, 'publish');
            await deviceStatus.processDeviceMessage(new Request('http://do'), message);

            expect(deviceStatus.isOn).toBe(true);
            expect(deviceStatus.publish).toHaveBeenCalledWith(message);
        });

        it('should process heatCycleCompleted and publish sessionData', async () => {
            const message = { type: 'heatCycleCompleted', duration: 15, cycle: 1 };
            env.db.first.mockResolvedValue({ count: 0 }); // No duplicates
            env.db.all.mockResolvedValue({ results: [{ id: '1', created_at: 123, duration: 15, cycle: 1 }] });

            vi.spyOn(deviceStatus, 'publish');
            await deviceStatus.processDeviceMessage(new Request('http://do'), message);

            expect(env.db.run).toHaveBeenCalled(); // For createHeatCycle
            expect(deviceStatus.publish).toHaveBeenCalledWith(expect.objectContaining({ type: 'sessionData' }));
        });

        it('should process stashUpdated message and publish', async () => {
            const message = { type: 'stashUpdated', item: { id: '1' } };
            vi.spyOn(deviceStatus, 'publish');
            await deviceStatus.processDeviceMessage(new Request('http://do'), message);
            expect(deviceStatus.publish).toHaveBeenCalledWith(message);
        });
    });

    describe('Alarm Handler', () => {
        it('should set device offline if threshold passed', async () => {
            deviceStatus.isOn = true;
            deviceStatus.lastSeen = Date.now() - 91000; // 91 seconds ago
            vi.spyOn(deviceStatus, 'publish');

            await deviceStatus.alarm();

            expect(deviceStatus.isOn).toBe(false);
            expect(state.storage.put).toHaveBeenCalledWith('isOn', false);
            expect(deviceStatus.publish).toHaveBeenCalledWith({ type: 'statusUpdate', isOn: false, isHeating: false });
            expect(state.storage.setAlarm).toHaveBeenCalled();
        });

        it('should not go offline if seen recently', async () => {
            deviceStatus.isOn = true;
            deviceStatus.lastSeen = Date.now() - 10000; // 10 seconds ago
            vi.spyOn(deviceStatus, 'publish');

            await deviceStatus.alarm();

            expect(deviceStatus.isOn).toBe(true);
            expect(state.storage.put).not.toHaveBeenCalledWith('isOn', false);
        });
    });

    describe('Publishing Logic', () => {
        it('should send message to all subscribers', async () => {
            const ws1 = new MockWebSocket();
            const ws2 = new MockWebSocket();
            deviceStatus.subscribers.set(ws1, { type: 'frontend' });
            deviceStatus.subscribers.set(ws2, { type: 'device' });

            const message = { type: 'test', data: 'hello' };
            deviceStatus.publish(message);

            expect(ws1.sentMessages[0]).toBe(JSON.stringify(message));
            expect(ws2.sentMessages[0]).toBe(JSON.stringify(message));
        });

        it('should strip heat_cycles from sessionData for device subscribers', async () => {
            const wsDevice = new MockWebSocket();
            deviceStatus.subscribers.set(wsDevice, { type: 'device' });

            const sessionData = { type: 'sessionData', clicks: 1, heat_cycles: [[{id: '1'}]] };
            deviceStatus.publish(sessionData);

            const sentMessage = JSON.parse(wsDevice.sentMessages[0]);
            expect(sentMessage).not.toHaveProperty('heat_cycles');
            expect(sentMessage.clicks).toBe(1);
        });
    });
});