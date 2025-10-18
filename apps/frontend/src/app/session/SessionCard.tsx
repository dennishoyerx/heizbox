import { useState } from 'react'
import type { HeatCycleRows, HeatCycleRow } from '@heizbox/types'
import { formatTimestampForTimeDisplay, calculateConsumption } from '@heizbox/utils'
import { Card, Flex, Text, Heading, Badge } from '@radix-ui/themes'
import { SessionDetailModal } from './SessionDetailModal'

export const SessionCard = ({
	heatCycles,
	index,
	totalHeatCycles,
}: {
	heatCycles: HeatCycleRows
	index: number
	totalHeatCycles: number
}) => {
	const [isModalOpen, setIsModalOpen] = useState(false)
	const count = heatCycles.length
	const caps = heatCycles.filter((hc: HeatCycleRow) => hc.cycle === 1).length

	const heatCycleConsumption = calculateConsumption(caps)
	const consumptionValue = parseFloat(heatCycleConsumption)

	let consumptionColor: 'red' | 'yellow' | 'green' = 'red'
	if (consumptionValue <= 0.2) {
		consumptionColor = 'green'
	} else if (consumptionValue <= 0.3) {
		consumptionColor = 'yellow'
	}

	// Calculate time range
	const startTimestamp = heatCycles[0].created_at
	const endTimestamp = heatCycles[heatCycles.length - 1].created_at
	const timeRangeString = `${formatTimestampForTimeDisplay(startTimestamp)} - ${formatTimestampForTimeDisplay(endTimestamp)}`

	return (
		<>
			<Card onClick={() => setIsModalOpen(true)} className='cursor-pointer'>
				<Flex justify='between' align='center'>
					<Flex align='baseline' gap='3' wrap='wrap' gapY='0'>
						<Heading size='3'>Session {totalHeatCycles - index}</Heading>
						<Text size='2' color='gray'>
							{timeRangeString}
						</Text>
					</Flex>
					<Flex align='center' gap='3'>
						<Badge color='iris'>
							<Flex align='center' direction='column'>
								<div>
									<span className='font-numeric'>{caps}</span> Caps
								</div>
								<div>
									<span className='font-numeric'>{count}</span> Klicks
								</div>
							</Flex>
						</Badge>
						<Badge color={consumptionColor} className='font-numeric'>
							{heatCycleConsumption}g
						</Badge>
					</Flex>
				</Flex>
			</Card>
			<SessionDetailModal isOpen={isModalOpen} onClose={() => setIsModalOpen(false)} heatCycle={heatCycles} />
		</>
	)
}
