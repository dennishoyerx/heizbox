import { Hono } from 'hono'
import type { Context } from 'hono'
import { HeatCycleService } from '../services/heatCycleService.js'
import { groupSessions, calculateConsumption } from '../utils/session.js'

const session = new Hono<{ Bindings: Env }>()

const handleGetSession = async (c: Context<{ Bindings: Env }>) => {
	try {
		const service = new HeatCycleService(c.env.db)
		const results = await service.getRecentSession(7200) // 2 hours

		if (!results || results.length === 0) {
			return c.json({
				clicks: 0,
				caps: 0,
				lastClick: null,
				heat_cycles: [],
				consumption: '0.00',
			})
		}

		const clicks = results.length
		const caps = results.filter((x) => x.cycle === 1).length
		const lastClick = results[results.length - 1].created_at
		const heat_cycles = groupSessions(results)
		const totalConsumption = calculateConsumption(caps)

		return c.json({ clicks, lastClick, heat_cycles, totalConsumption })
	} catch (error) {
		console.error('Error in handleGetSession:', error)
		const err = error as Error
		return c.json({ error: 'Failed to retrieve session data', details: err.message }, 500)
	}
}

session.get('/', handleGetSession)

export default session
