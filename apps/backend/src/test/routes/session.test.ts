import { describe, it, expect, vi } from 'vitest'
import app from '../../index.js'
import type { HeatCycleRow } from '@heizbox/types'

describe('Session API [/api/session]', () => {
	it('should return current session data', async () => {
		const now = Math.floor(Date.now() / 1000)
		const mockRecentHeatCycles: HeatCycleRow[] = [
			{ id: '2', created_at: now - 30, duration: 10, cycle: 2 },
			{ id: '3', created_at: now, duration: 10, cycle: 1 },
		]

		// Today's cycles (includes recent)
		const mockTodayHeatCycles: HeatCycleRow[] = [{ id: '1', created_at: now - 3600, duration: 10, cycle: 1 }, ...mockRecentHeatCycles]

		// Yesterday's cycles
		const mockYesterdayHeatCycles: HeatCycleRow[] = [
			{ id: '4', created_at: now - 90000, duration: 10, cycle: 1 },
			{ id: '5', created_at: now - 90030, duration: 10, cycle: 1 },
		]

		const mockDb = {
			prepare: vi.fn().mockReturnThis(),
			bind: vi.fn().mockReturnThis(),
			all: vi
				.fn()
				.mockResolvedValueOnce({ results: mockRecentHeatCycles }) // getRecentSession
				.mockResolvedValueOnce({ results: mockTodayHeatCycles }) // getHeatCyclesInRange (today)
				.mockResolvedValueOnce({ results: mockYesterdayHeatCycles }), // getHeatCyclesInRange (yesterday)
		}

		const env = { db: mockDb } as any
		const res = await app.request('/api/session', { method: 'GET' }, env)
		const json = await res.json()

		expect(res.status).toBe(200)
		expect(json.clicks).toBe(2) // From mockRecentHeatCycles
		expect(json.caps).toBe(1) // From mockRecentHeatCycles
		expect(json.lastClick).toBe(mockRecentHeatCycles[1].created_at)
		expect(json.heat_cycles).toHaveLength(1)
		expect(json.consumption).toBe(0.05) // 1 cap in recent
		expect(json.consumptionTotal).toBe(0.1) // 2 caps in today
		expect(json.consumptionYesterday).toBe(0.1) // 2 caps in yesterday
	})

	it('should return empty session data when no recent heat cycles exist', async () => {
		const mockDb = {
			prepare: vi.fn().mockReturnThis(),
			bind: vi.fn().mockReturnThis(),
			all: vi.fn().mockResolvedValue({ results: [] }),
		}

		const env = { db: mockDb } as any
		const res = await app.request('/api/session', { method: 'GET' }, env)
		const json = await res.json()

		expect(res.status).toBe(200)
		expect(json.clicks).toBe(0)
		expect(json.caps).toBe(0)
		expect(json.lastClick).toBeNull()
		expect(json.heat_cycles).toEqual([])
		expect(json.consumption).toBe(0)
		expect(json.consumptionTotal).toBe(0)
		expect(json.consumptionYesterday).toBe(0)
	})

	it('should handle database errors gracefully', async () => {
		const mockDb = {
			prepare: vi.fn(() => {
				throw new Error('DB Error')
			}),
		}

		const env = { db: mockDb } as any
		const res = await app.request('/api/session', { method: 'GET' }, env)
		const json = await res.json()

		expect(res.status).toBe(500)
		expect(json.error).toBe('Failed to retrieve session data')
	})
})