import { describe, it, expect, vi, beforeEach } from 'vitest';
import app from '../../index.js';
import type { HeatCycleRow } from '@heizbox/types';
import { getBerlinTimeRange } from '../../utils/time.js';

// Mock the time utility
vi.mock('../../utils/time.js', () => ({
    getBerlinTimeRange: vi.fn(),
}));

describe('Heat Cycles API [/api/heat_cycles]', () => {

    beforeEach(() => {
        vi.clearAllMocks();
    });

    describe('GET /', () => {
        it('should return heat cycles and consumption for a given time range', async () => {
            vi.mocked(getBerlinTimeRange).mockReturnValue({ start: 1609459200, end: 1609459800 });

            const mockDb = {
                prepare: vi.fn().mockReturnThis(),
                bind: vi.fn().mockReturnThis(),
                all: vi.fn().mockResolvedValue({
                    results: [
                        { id: '1', created_at: 1609459200, duration: 10, cycle: 1 },
                        { id: '2', created_at: 1609459500, duration: 10, cycle: 2 },
                    ] as HeatCycleRow[],
                }),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.heatCycles).toHaveLength(1);
            expect(json.heatCycles[0]).toHaveLength(2);
            expect(json.totalConsumption).toBe('0.05');
            expect(mockDb.prepare).toHaveBeenCalledWith(expect.stringContaining('SELECT id, created_at, duration, cycle FROM heat_cycles'));
            expect(mockDb.bind).toHaveBeenCalledWith(1609459200, 1609459800);
        });

        it('should return an empty array and 0 consumption if no heat cycles are found', async () => {
            vi.mocked(getBerlinTimeRange).mockReturnValue({ start: 1609459200, end: 1609459800 });

            const mockDb = {
                prepare: vi.fn().mockReturnThis(),
                bind: vi.fn().mockReturnThis(),
                all: vi.fn().mockResolvedValue({ results: [] }),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.heatCycles).toEqual([]);
            expect(json.totalConsumption).toBe('0.00');
        });

        it('should handle database errors gracefully', async () => {
            vi.mocked(getBerlinTimeRange).mockReturnValue({ start: 1609459200, end: 1609459800 });
            
            const mockDb = {
                prepare: vi.fn(() => {
                    throw new Error('DB error');
                }),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(500);
            expect(json.error).toBe('Failed to retrieve heat cycles');
        });
    });

    describe('GET /create', () => {
        it('should create a new heat cycle and return OK', async () => {
            const mockDb = {
                prepare: vi.fn().mockReturnThis(),
                bind: vi.fn().mockReturnThis(),
                first: vi.fn().mockResolvedValue({ count: 0 }),
                run: vi.fn().mockResolvedValue({}),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles/create?duration=10&cycle=1', { method: 'GET' }, env);
            
            expect(res.status).toBe(200);
            expect(await res.text()).toBe('OK');
            expect(mockDb.prepare).toHaveBeenCalledTimes(2); // once for duplicate check, once for insert
            expect(mockDb.run).toHaveBeenCalled();
        });

        it('should use cycle=1 if not provided', async () => {
            const mockDb = {
                prepare: vi.fn().mockReturnThis(),
                bind: vi.fn().mockReturnThis(),
                first: vi.fn().mockResolvedValue({ count: 0 }),
                run: vi.fn().mockResolvedValue({}),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles/create?duration=10', { method: 'GET' }, env);
            
            expect(res.status).toBe(200);
            expect(await res.text()).toBe('OK');
            expect(mockDb.bind).toHaveBeenCalledWith(expect.any(String), 10, 1);
        });

        it('should return 400 if duration is missing', async () => {
            const env = { db: {} } as any;
            const res = await app.request('/api/heat_cycles/create', { method: 'GET' }, env);
            expect(res.status).toBe(400);
            expect(await res.text()).toBe('Missing duration');
        });

        it('should return 500 if creating a duplicate heat cycle', async () => {
            const mockDb = {
                prepare: vi.fn().mockReturnThis(),
                bind: vi.fn().mockReturnThis(),
                first: vi.fn().mockResolvedValue({ count: 1 }), // Simulate duplicate found
                run: vi.fn(),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles/create?duration=10&cycle=1', { method: 'GET' }, env);
            
            expect(res.status).toBe(500);
            expect(await res.text()).toBe('Failed to create heat cycle');
            expect(mockDb.run).not.toHaveBeenCalled();
        });
    });

    describe('GET /add', () => {
        it('should behave the same as /create', async () => {
            const mockDb = {
                prepare: vi.fn().mockReturnThis(),
                bind: vi.fn().mockReturnThis(),
                first: vi.fn().mockResolvedValue({ count: 0 }),
                run: vi.fn().mockResolvedValue({}),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles/add?duration=15', { method: 'GET' }, env);
            expect(res.status).toBe(200);
            expect(await res.text()).toBe('OK');
        });
    });

    describe('GET /all_heat_cycles', () => {
        it('should return all heat cycles from the database', async () => {
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
            const res = await app.request('/api/heat_cycles/all_heat_cycles', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json).toHaveLength(2);
            expect(mockDb.prepare).toHaveBeenCalledWith(expect.stringContaining('SELECT id, created_at, duration, cycle FROM heat_cycles'));
        });

        it('should handle database errors gracefully', async () => {
            const mockDb = {
                prepare: vi.fn(() => {
                    throw new Error('DB error');
                }),
            };

            const env = { db: mockDb } as any;
            const res = await app.request('/api/heat_cycles/all_heat_cycles', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(500);
            expect(json.error).toBe('Failed to retrieve all heat cycles');
        });
    });
});