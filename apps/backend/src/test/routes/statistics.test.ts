import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';
import type { HeatCycleRow } from '@heizbox/types';

describe('Statistics API [/api/statistics]', () => {

    const mockHeatCycles: HeatCycleRow[] = [
        { id: '1', created_at: Math.floor(Date.now() / 1000) - 3600, duration: 10, cycle: 1 },
        { id: '2', created_at: Math.floor(Date.now() / 1000) - 7200, duration: 12, cycle: 1 },
    ];

    it('should return statistics for the default range (month)', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: mockHeatCycles }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/statistics', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json.range).toBe('month');
        expect(json.totalHeatCycles).toBe(2);
        expect(json.totalDuration).toBe(22);
        expect(json.heatCycles).toHaveLength(2);
    });

    it('should return statistics for the "day" range', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: [mockHeatCycles[0]] }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/statistics?range=day', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json.range).toBe('day');
        expect(json.totalHeatCycles).toBe(1);
        expect(json.totalDuration).toBe(10);
    });

    it('should return statistics for the "week" range', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: mockHeatCycles }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/statistics?range=week', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json.range).toBe('week');
        expect(json.totalHeatCycles).toBe(2);
    });

    it('should return 400 for an invalid range', async () => {
        const env = { db: {} } as any;
        const res = await app.request('/api/statistics?range=invalid', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(400);
        expect(json.error).toContain('Invalid range');
    });

    it('should return empty statistics if no data is found', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: [] }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/statistics?range=day', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json.totalHeatCycles).toBe(0);
        expect(json.totalDuration).toBe(0);
        expect(json.heatCycles).toEqual([]);
    });

    it('should handle database errors gracefully', async () => {
        const mockDb = {
            prepare: vi.fn(() => {
                throw new Error('DB Error');
            }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/statistics', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(500);
        expect(json.error).toBe('Failed to fetch statistics');
    });
});