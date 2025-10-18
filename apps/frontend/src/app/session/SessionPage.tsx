import { SessionCard } from './SessionCard'
import SessionHeader from './SessionHeader'
import { Flex, Text } from '@radix-ui/themes'
import { useWebSocket } from '../WebSocketContext'
import { useHeatCycles } from './useHeatCycles'
import type { HeatCycleRows } from '@heizbox/types'

function SessionPage() {
	const { data, isLoading, isError, error } = useHeatCycles()
	const { deviceIsHeating } = useWebSocket()

	return (
		<Flex direction='column' gap='3' maxWidth='600px' className='mx-auto'>
			<SessionHeader isHeating={deviceIsHeating} consumed={data?.totalConsumption} />
			{isLoading && <Text>Lade Daten...</Text>}
			{isError && <Text color='red'>Fehler beim Laden: {error.message}</Text>}
			{data &&
				(data.heatCycles && data.heatCycles.length > 0 ? (
					<Flex direction='column' gap='3'>
						{data.heatCycles.map((heatCycles: HeatCycleRows, index: number) => (
							<SessionCard
								key={heatCycles[0]?.id || index}
								heatCycles={heatCycles}
								index={index}
								totalHeatCycles={data.heatCycles.length}
							/>
						))}
					</Flex>
				) : (
					!isLoading && <Text>Keine Heat Cycles im ausgewählten Zeitraum gefunden.</Text>
				))}
		</Flex>
	)
}

export default SessionPage
