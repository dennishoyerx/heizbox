import { describe, it, expect, vi, beforeEach } from 'vitest';
import app from '../../index.js';
import type { StashItemRow, StashItemWithHistory, StashWithdrawalRow } from '@heizbox/types';

describe('Stash API [/api/stash]', () => {

    let mockDb: any;
    let mockDeviceStatus: any;
    let env: any;

    beforeEach(() => {
        mockDb = {
            prepare: vi.fn().mockReturnThis(),
            bind: vi.fn().mockReturnThis(),
            run: vi.fn().mockResolvedValue({}),
            all: vi.fn().mockResolvedValue({ results: [] }),
            first: vi.fn().mockResolvedValue(null),
        };
        mockDeviceStatus = {
            idFromName: vi.fn().mockReturnThis(),
            get: vi.fn().mockReturnThis(),
            fetch: vi.fn().mockResolvedValue(new Response(JSON.stringify({ success: true }))),
        };
        env = { db: mockDb, DEVICE_STATUS: mockDeviceStatus };
    });

    describe('GET /', () => {
        it('should return all stash items with history', async () => {
            const mockItems: StashItemWithHistory[] = [
                { id: '1', name: 'Item 1', initial_amount: 10, current_amount: 5, created_at: '', updated_at: '', total_withdrawn: 5, withdrawal_count: 1, last_withdrawal: '' },
            ];
            mockDb.all.mockResolvedValue({ results: mockItems });

            const res = await app.request('/api/stash', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.items).toHaveLength(1);
            expect(json.total_current).toBe(5);
            expect(json.total_withdrawn).toBe(5);
        });
    });

    describe('POST /', () => {
        it('should create a new stash item', async () => {
            const newItem = { name: 'New Item', initial_amount: 20 };
            const createdItem: StashItemRow = { id: 'gen-uuid', ...newItem, current_amount: 20, created_at: '', updated_at: '' };
            mockDb.first.mockResolvedValue(createdItem);

            const res = await app.request('/api/stash', { method: 'POST', body: JSON.stringify(newItem) }, env);
            const json = await res.json();

            expect(res.status).toBe(201);
            expect(json.name).toBe('New Item');
            expect(mockDb.run).toHaveBeenCalled();
            expect(mockDeviceStatus.fetch).toHaveBeenCalled();
        });

        it('should return 400 for invalid data', async () => {
            const res = await app.request('/api/stash', { method: 'POST', body: JSON.stringify({ name: 'bad' }) }, env);
            expect(res.status).toBe(400);
        });
    });

    describe('PUT /:id/withdraw', () => {
        it('should withdraw from a stash item', async () => {
            const item: StashItemRow = { id: '1', name: 'Item 1', initial_amount: 10, current_amount: 5, created_at: '', updated_at: '' };
            mockDb.first.mockResolvedValue(item);

            const res = await app.request('/api/stash/1/withdraw', { method: 'PUT', body: JSON.stringify({ amount: 2 }) }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.item.current_amount).toBe(3);
            expect(mockDb.run).toHaveBeenCalledTimes(2); // createWithdrawal and updateItemQuantity
        });

        it('should return 400 for invalid amount', async () => {
            const res = await app.request('/api/stash/1/withdraw', { method: 'PUT', body: JSON.stringify({ amount: -1 }) }, env);
            expect(res.status).toBe(400);
        });

        it('should return error for insufficient quantity', async () => {
            const item: StashItemRow = { id: '1', name: 'Item 1', initial_amount: 10, current_amount: 5, created_at: '', updated_at: '' };
            mockDb.first.mockResolvedValue(item);
            const res = await app.request('/api/stash/1/withdraw', { method: 'PUT', body: JSON.stringify({ amount: 10 }) }, env);
            const json = await res.json();
            expect(res.status).toBe(500); // Should be a 4xx error, but service throws generic Error
            expect(json.details).toContain('Insufficient quantity');
        });
    });

    describe('DELETE /:id', () => {
        it('should delete a stash item', async () => {
            const item: StashItemRow = { id: '1', name: 'Item 1', initial_amount: 10, current_amount: 5, created_at: '', updated_at: '' };
            mockDb.first.mockResolvedValue(item);

            const res = await app.request('/api/stash/1', { method: 'DELETE' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.success).toBe(true);
            expect(mockDb.run).toHaveBeenCalled();
        });
    });

    describe('GET /stats', () => {
        it('should return stash statistics', async () => {
            mockDb.all.mockResolvedValueOnce({ results: [] }).mockResolvedValueOnce({ results: [] }); // items and daily withdrawals

            const res = await app.request('/api/stash/stats', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json.range).toBe('30d');
            expect(json).toHaveProperty('total_items');
        });
    });

    describe('GET /:id/withdrawals', () => {
        it('should return withdrawal history for an item', async () => {
            const withdrawals: StashWithdrawalRow[] = [
                { id: 'w1', stash_item_id: '1', amount: 2, withdrawn_at: '' },
            ];
            mockDb.all.mockResolvedValue({ results: withdrawals });

            const res = await app.request('/api/stash/1/withdrawals', { method: 'GET' }, env);
            const json = await res.json();

            expect(res.status).toBe(200);
            expect(json).toHaveLength(1);
            expect(json[0].id).toBe('w1');
        });
    });
});