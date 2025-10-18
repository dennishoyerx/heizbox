import { describe, it, expect, vi } from 'vitest';
import app from '../../index.js';
import type { HeatCycleRow } from '@heizbox/types';

describe('JSON API [/api/json]', () => {

    it('should return all heat cycles as JSON', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({
                results: [
                    { id: '1', created_at: 1609459200, duration: 10, cycle: 1 },
                    { id: '2', created_at: 1609459500, duration: 10, cycle: 2 },
                ] as HeatCycleRow[],
            }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/json', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json).toHaveLength(2);
        expect(json[0].id).toBe('1');
        expect(mockDb.prepare).toHaveBeenCalledWith("SELECT id, created_at, duration, cycle FROM heat_cycles ORDER BY id DESC");
    });

    it('should return an empty array if no heat cycles are in the database', async () => {
        const mockDb = {
            prepare: vi.fn().mockReturnThis(),
            all: vi.fn().mockResolvedValue({ results: [] }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/json', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(200);
        expect(json).toEqual([]);
    });

    it('should handle database errors gracefully', async () => {
        const mockDb = {
            prepare: vi.fn(() => {
                throw new Error('DB Error');
            }),
        };

        const env = { db: mockDb } as any;
        const res = await app.request('/api/json', { method: 'GET' }, env);
        const json = await res.json();

        expect(res.status).toBe(500);
        expect(json.err).toBe('Failed to retrieve all heat cycles');
    });
});