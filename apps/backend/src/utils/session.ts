import type { HeatCycleRow } from '@heizbox/types'
import { config } from '../config/index.js'

export const groupSessions = (rows: HeatCycleRow[]): HeatCycleRow[][] => {
	if (!rows || rows.length === 0) return []

	const intervalMs = config.session.groupIntervalMinutes * 60 * 1000
	const heatCycles: HeatCycleRow[][] = []
	let currentGroup: HeatCycleRow[] = [rows[0]]

	for (let i = 1; i < rows.length; i++) {
		const currentRow = rows[i]
		const previousRow = rows[i - 1]
		const currentDate = new Date(Number(currentRow.created_at) * 1000)
		const previousDate = new Date(Number(previousRow.created_at) * 1000)

		if (currentDate.getTime() - previousDate.getTime() >= intervalMs) {
			heatCycles.push(currentGroup)
			currentGroup = [currentRow]
		} else {
			currentGroup.push(currentRow)
		}
	}

	heatCycles.push(currentGroup)
	return heatCycles.reverse()
}

export const calculateConsumption = (count: number): number =>
	Number((config.session.consumptionPerCycle * Math.ceil(count)).toFixed(2))
