import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';

describe('Heartbeat API [/api/heartbeat]', () => {

    it('should call the Durable Object\'s handleHeartbeat method directly', async () => {
        const mockHandleHeartbeat = vi.fn(async () => {});
        const mockStub = { handleHeartbeat: mockHandleHeartbeat };
        const mockDeviceStatus = {
            idFromName: vi.fn(() => 'mock-id'),
            get: vi.fn(() => mockStub),
        };

        const env = { DEVICE_STATUS: mockDeviceStatus } as any;
        const deviceId = 'test-device';

        const res = await app.request(`/api/heartbeat/${deviceId}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
        }, env);

        expect(res.status).toBe(200);
        expect(await res.json()).toEqual({ success: true });

        expect(mockDeviceStatus.idFromName).toHaveBeenCalledWith(deviceId);
        expect(mockDeviceStatus.get).toHaveBeenCalledWith('mock-id');
        expect(mockHandleHeartbeat).toHaveBeenCalledTimes(1);
    });
});