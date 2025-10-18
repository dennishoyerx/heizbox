import { useQuery, useQueryClient } from '@tanstack/react-query'
import { fetchSession } from '../../api'
import { useWebSocketEvent } from '../WebSocketContext'
import { useCallback } from 'react'

export const useSession = () => {
	const queryClient = useQueryClient()

	const query = useQuery({
		queryKey: ['runningSession'], // Eindeutiger Key für Caching
		queryFn: fetchSession, // Die API-Funktion
	})

	// WebSocket-Event-Handler zum automatischen Neuladen
	const handleCycleUpdate = useCallback(() => {
		console.log('Heat cycle update received, invalidating query...')
		// Invalidiert den Cache und löst einen Refetch aus
		queryClient.invalidateQueries({ queryKey: ['runningSession'] })
	}, [queryClient])

	useWebSocketEvent('heatCycleCompleted', handleCycleUpdate)
	useWebSocketEvent('heatCycleCreated', handleCycleUpdate)
	return query
}
