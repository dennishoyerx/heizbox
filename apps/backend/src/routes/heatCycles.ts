import { Hono } from 'hono'
import type { Context } from 'hono'
import { HeatCycleService } from '../services/heatCycleService.js'
import { getBerlinTimeRange } from '../utils/time.js'
import { groupSessions, calculateConsumption } from '../utils/session.js'

const heatCycles = new Hono<{ Bindings: Env }>()

const handleGetHeatCycles = async (c: Context<{ Bindings: Env }>) => {
	try {
		const { start, end } = getBerlinTimeRange()
		console.log(
			`Querying heat_cycles from ${new Date(start * 1000).toISOString()} to ${new Date(end * 1000).toISOString()}`,
		)

		const service = new HeatCycleService(c.env.db)
		const results = await service.getHeatCyclesInRange(start, end)

		if (!results || results.length === 0) {
			return c.json({ heatCycles: [], totalConsumption: '0.00' })
		}

		const heatCycles = groupSessions(results)
		const totalConsumption = calculateConsumption(results.filter((x) => x.cycle === 1).length)

		return c.json({ heatCycles, totalConsumption })
	} catch (error) {
		console.error('Error in handleGetHeatCycles:', error)
		const err = error as Error
		return c.json({ error: 'Failed to retrieve heat cycles', details: err.message }, 500)
	}
}

const handleCreateHeatCycle = async (c: Context<{ Bindings: Env }>) => {
	try {
		const durationStr = c.req.query('duration')
		const cycleStr = c.req.query('cycle')
		const deviceId = c.req.query('deviceId') // Extract deviceId

		if (!durationStr) {
			return c.text('Missing duration', 400)
		}
		if (!deviceId) {
			return c.text('Missing deviceId', 400)
		}

		const duration = parseFloat(durationStr)
		const cycle = cycleStr ? parseInt(cycleStr, 10) : 1

		const service = new HeatCycleService(c.env.db)
		const success = await service.createHeatCycle(duration, cycle)

		if (success) {
			// Get the Durable Object stub
			const durableObjectId = c.env.DEVICE_STATUS.idFromName(deviceId)
			const durableObjectStub = c.env.DEVICE_STATUS.get(durableObjectId)

			// Send WebSocket message via the Durable Object
			await durableObjectStub.fetch(
				new Request('http://do/send-message', {
					method: 'POST',
					headers: { 'Content-Type': 'application/json' },
					body: JSON.stringify({ type: 'heatCycleCreated' }),
				}),
			)
		}

		return success ? c.text('OK') : c.text('Failed to create heat cycle', 500)
	} catch (error) {
		console.error('Error in handleCreateHeatCycle:', error)
		const err = error as Error
		return c.json({ error: 'Failed to create heat cycle', details: err.message }, 500)
	}
}

const handleGetAllHeatCycles = async (c: Context<{ Bindings: Env }>) => {
	try {
		const service = new HeatCycleService(c.env.db)
		const results = await service.getAllHeatCycles()
		return c.json(results)
	} catch (error) {
		console.error('Error in handleGetAllHeatCycles:', error)
		const err = error as Error
		return c.json({ error: 'Failed to retrieve all heat cycles', details: err.message }, 500)
	}
}

heatCycles.get('/', handleGetHeatCycles)
heatCycles.get('/create', handleCreateHeatCycle)
heatCycles.get('/add', handleCreateHeatCycle)
heatCycles.get('/all_heat_cycles', handleGetAllHeatCycles)

export default heatCycles
