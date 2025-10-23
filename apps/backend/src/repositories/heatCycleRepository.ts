import type { D1Database } from '@cloudflare/workers-types'
import type { HeatCycleRow } from '@heizbox/types'

export class HeatCycleRepository {
	constructor(private db: D1Database) {}

	async findByTimeRange(start: number, end: number): Promise<HeatCycleRow[]> {
		const { results } = await this.db
			.prepare(
				// Explizite Spalten statt SELECT *
				'SELECT id, created_at, duration, cycle FROM heat_cycles ' +
					'WHERE created_at >= ?1 AND created_at < ?2 ' +
					'ORDER BY created_at ASC LIMIT 1000',
			)
			.bind(start, end)
			.all<HeatCycleRow>()

		return results || []
	}

	async findAll(): Promise<HeatCycleRow[]> {
		const { results } = await this.db
			.prepare('SELECT id, created_at, duration, cycle FROM heat_cycles ORDER BY created_at DESC')
			.all<HeatCycleRow>()

		return results || []
	}

	async findRecent(limit: number = 100): Promise<HeatCycleRow[]> {
		const safeLimit = Math.min(limit, 1000)

		const { results } = await this.db
			.prepare('SELECT id, created_at, duration, cycle FROM heat_cycles ' + 'ORDER BY created_at DESC LIMIT ?1')
			.bind(safeLimit)
			.all<HeatCycleRow>()

		return results || []
	}

	async findDuplicates(duration: number, cycle: number, timeThresholdMs: number): Promise<number> {
		const thresholdTime = Date.now() - timeThresholdMs
		const { count } = await this.db
			.prepare('SELECT COUNT(*) as count FROM heat_cycles WHERE duration = ?1 AND cycle = ?2 AND created_at > ?3')
			.bind(duration, cycle, Math.floor(thresholdTime / 1000))
			.first<{ count: number }>()

		return count || 0
	}

	async create(id: string, duration: number, cycle: number): Promise<void> {
		await this.db
			.prepare('INSERT INTO heat_cycles (id, duration, cycle) VALUES (?1, ?2, ?3)')
			.bind(id, duration, cycle)
			.run()
	}

	async findRecentForSession(timeLimitSeconds: number): Promise<HeatCycleRow[]> {
		const timeThreshold = Math.floor(Date.now() / 1000) - timeLimitSeconds
		const { results } = await this.db
			.prepare(
				'SELECT id, created_at, duration, cycle FROM heat_cycles ' +
					'WHERE created_at >= ?1 ORDER BY created_at ASC LIMIT 500',
			)
			.bind(timeThreshold)
			.all<HeatCycleRow>()

		return results || []
	}
}
