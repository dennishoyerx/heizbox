import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';
import type { HeatCycleRow } from '@heizbox/types';

describe('Session API [/api/session]', () => {

    it('should return current session data', async () => {
        const mockHeatCycles: HeatCycleRow[] = [
            { id: '1', created_at: Math.floor(Date.now() / 1000) - 60, duration: 10, cycle: 1 },
            { id: '2', created_at: Math.floor(Date.now() / 1000) - 30, duration: 10, cycle: 2 },
            { id: '3', created_at: Math.floor(Date.now() / 1000), duration: 10, cycle: 1 },
        ];

        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: mockHeatCycles }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/session', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json.clicks).toBe(3);
        expect(json.caps).toBe(2);
        expect(json.lastClick).toBe(mockHeatCycles[2].created_at);
        expect(json.heat_cycles).toHaveLength(1); // All cycles are in one session
        expect(json.consumption).toBe('0.10');
    });

    it('should return empty session data when no recent heat cycles exist', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: [] }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/session', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json.clicks).toBe(0);
        expect(json.caps).toBe(0);
        expect(json.lastClick).toBeNull();
        expect(json.heat_cycles).toEqual([]);
        expect(json.consumption).toBe('0.00');
    });

    it('should handle database errors gracefully', async () => {
        const mockDb = {
            prepare: vi.fn(() => {
                throw new Error('DB Error');
            }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/session', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(500);
        expect(json.error).toBe('Failed to retrieve session data');
    });
});