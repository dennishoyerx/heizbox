import type { D1Database } from '@cloudflare/workers-types'
import { HeatCycleService } from './heatCycleService.js'
import { groupSessions, calculateConsumption } from '../utils/session.js'
import { getBerlinTimeRange } from '../utils/time.js'

export class SessionService {
	private heatCycleService: HeatCycleService

	constructor(db: D1Database) {
		this.heatCycleService = new HeatCycleService(db)
	}

	async getCurrentSessionData() {
		const { start, end } = getBerlinTimeRange()
		const results = await this.heatCycleService.getRecentSession(7200) // 2 hours
		const today = await this.heatCycleService.getHeatCyclesInRange(start, end)

		if (!results || results.length === 0) {
			return {
				clicks: 0,
				caps: 0,
				lastClick: null,
				heat_cycles: [],
				consumption: 0,
				consumptionTotal: 0,
			}
		}

		const clicks = results.length
		const caps = results.filter((x) => x.cycle === 1).length
		const lastClick = results[results.length - 1].created_at
		const heat_cycles = groupSessions(results)
		const consumption = calculateConsumption(caps)
		const consumptionTotal = calculateConsumption(today.filter((x) => x.cycle === 1).length)

		return { clicks, caps, lastClick, heat_cycles, consumption, consumptionTotal }
	}
}
