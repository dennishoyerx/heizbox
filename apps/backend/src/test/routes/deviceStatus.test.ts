import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';

describe('Device Status API [/api/device-status]', () => {

    it('should forward a GET request to the correct Durable Object stub with path stripped', async () => {
        const mockFetch = vi.fn(() => Promise.resolve(new Response('GET OK')));
        const mockStub = { fetch: mockFetch };
        const mockDeviceStatus = {
            idFromName: vi.fn(() => 'mock-id'),
            get: vi.fn(() => mockStub),
        };

        const env = { DEVICE_STATUS: mockDeviceStatus } as any;
        const deviceId = 'test-device';
        const subPath = 'status';

        const res = await app.request(`/api/device-status/${deviceId}/${subPath}`, { method: 'GET' }, env);

        expect(res.status).toBe(200);
        expect(await res.text()).toBe('GET OK');

        expect(mockDeviceStatus.idFromName).toHaveBeenCalledWith(deviceId);
        expect(mockDeviceStatus.get).toHaveBeenCalledWith('mock-id');
        
        expect(mockFetch).toHaveBeenCalledTimes(1);
        const forwardedRequest = mockFetch.mock.calls[0][0] as Request;
        const url = new URL(forwardedRequest.url);
        expect(url.pathname).toBe(`/${subPath}`);
        expect(forwardedRequest.method).toBe('GET');
    });

    it('should forward a POST request to the correct Durable Object stub with path and body', async () => {
        let capturedBody: any;
        const mockFetch = vi.fn(async (request: Request) => {
            if (request.method === 'POST') {
                capturedBody = await request.json();
            }
            return new Response('POST OK');
        });
        const mockStub = { fetch: mockFetch };
        const mockDeviceStatus = {
            idFromName: vi.fn(() => 'mock-id'),
            get: vi.fn(() => mockStub),
        };

        const env = { DEVICE_STATUS: mockDeviceStatus } as any;
        const deviceId = 'test-device';
        const subPath = 'status';
        const requestBody = { isOn: true };

        const res = await app.request(`/api/device-status/${deviceId}/${subPath}`, {
            method: 'POST',
            body: JSON.stringify(requestBody),
            headers: { 'Content-Type': 'application/json' }
        }, env);

        expect(res.status).toBe(200);
        expect(await res.text()).toBe('POST OK');
        
        expect(mockFetch).toHaveBeenCalledTimes(1);
        const forwardedRequest = mockFetch.mock.calls[0][0] as Request;
        const url = new URL(forwardedRequest.url);
        expect(url.pathname).toBe(`/${subPath}`);
        expect(forwardedRequest.method).toBe('POST');
        expect(capturedBody).toEqual(requestBody);
    });
});