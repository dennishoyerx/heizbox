import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';

describe('Heartbeat API [/api/heartbeat]', () => {

    it('should forward heartbeat request to the correct Durable Object stub', async () => {
        let capturedBody: any;
        const mockFetch = vi.fn(async (request: Request) => {
            if (request.method === 'POST') {
                capturedBody = await request.json();
            }
            return new Response('OK');
        });
        const mockStub = { fetch: mockFetch };
        const mockDeviceStatus = {
            idFromName: vi.fn(() => 'mock-id'),
            get: vi.fn(() => mockStub),
        };

        const env = { DEVICE_STATUS: mockDeviceStatus } as any;
        const deviceId = 'test-device';
        const requestBody = { type: 'heartbeat' };

        const res = await app.request(`/api/heartbeat/${deviceId}`, {
            method: 'POST',
            body: JSON.stringify(requestBody),
            headers: { 'Content-Type': 'application/json' }
        }, env);

        expect(res.status).toBe(200);
        expect(await res.text()).toBe('OK');

        expect(mockDeviceStatus.idFromName).toHaveBeenCalledWith(deviceId);
        expect(mockDeviceStatus.get).toHaveBeenCalledWith('mock-id');
        
        expect(mockFetch).toHaveBeenCalledTimes(1);
        const forwardedRequest = mockFetch.mock.calls[0][0] as Request;
        expect(forwardedRequest.url).toContain('/process-device-message');
        expect(forwardedRequest.method).toBe('POST');
        expect(capturedBody).toEqual(requestBody);
    });
});