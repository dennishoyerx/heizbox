import { useQuery, useQueryClient } from '@tanstack/react-query';
import { fetchHeatCycles } from '../../api';
import { useWebSocketEvent } from '../WebSocketContext';
import { useCallback } from 'react';

export const useHeatCycles = () => {
  const queryClient = useQueryClient();

  const query = useQuery({
    queryKey: ['heatCycles'], // Eindeutiger Key für Caching
    queryFn: fetchHeatCycles, // Die API-Funktion
  });

  // WebSocket-Event-Handler zum automatischen Neuladen
  const handleCycleUpdate = useCallback(() => {
    console.log('Heat cycle update received, invalidating query...');
    // Invalidiert den Cache und löst einen Refetch aus
    queryClient.invalidateQueries({ queryKey: ['heatCycles'] });
  }, [queryClient]);

  useWebSocketEvent("heatCycleCompleted", handleCycleUpdate);
  useWebSocketEvent("heatCycleCreated", handleCycleUpdate);

  return query;
};
