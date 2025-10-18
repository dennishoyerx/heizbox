import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';

describe('WebSocket API [/ws]', () => {

    it('should forward a valid WebSocket upgrade request to the Durable Object', async () => {
        const mockFetch = vi.fn(() => Promise.resolve(new Response(null, { status: 200 })));
        const mockStub = { fetch: mockFetch };
        const mockDeviceStatus = {
            idFromName: vi.fn(() => 'mock-id'),
            get: vi.fn(() => mockStub),
        };

        const env = { DEVICE_STATUS: mockDeviceStatus } as any;
        const deviceId = 'test-device';
        const type = 'frontend';

        const res = await app.request(`/ws?deviceId=${deviceId}&type=${type}`, {
            headers: { 'Upgrade': 'websocket' }
        }, env);

        expect(res.status).toBe(200);
        expect(mockDeviceStatus.idFromName).toHaveBeenCalledWith(deviceId);
        expect(mockDeviceStatus.get).toHaveBeenCalledWith('mock-id');
        
        expect(mockFetch).toHaveBeenCalledTimes(1);
        const forwardedRequest = mockFetch.mock.calls[0][0] as Request;
        const url = new URL(forwardedRequest.url);
        expect(url.pathname).toBe('/');
        expect(forwardedRequest.headers.get('Upgrade')).toBe('websocket');
    });

    it('should return 400 if deviceId is missing', async () => {
        const env = { DEVICE_STATUS: {} } as any;
        const res = await app.request('/ws?type=frontend', {
            headers: { 'Upgrade': 'websocket' }
        }, env);
        
        expect(res.status).toBe(400);
        const json = await res.json();
        expect(json.error).toBe('deviceId is required');
    });

    it('should default connection type to "device" if not specified', async () => {
        const mockFetch = vi.fn(() => Promise.resolve(new Response(null, { status: 200 })));
        const mockStub = { fetch: mockFetch };
        const mockDeviceStatus = {
            idFromName: vi.fn(() => 'mock-id'),
            get: vi.fn(() => mockStub),
        };

        const env = { DEVICE_STATUS: mockDeviceStatus } as any;
        const deviceId = 'test-device';

        await app.request(`/ws?deviceId=${deviceId}`, {
            headers: { 'Upgrade': 'websocket' }
        }, env);

        const forwardedRequest = mockFetch.mock.calls[0][0] as Request;
        const url = new URL(forwardedRequest.url);
        expect(url.searchParams.get('deviceId')).toBe(deviceId);
        expect(url.searchParams.get('type')).toBe(null);
    });
});
