import { useEffect, useState, useCallback } from 'react';
import { SessionGroup, ApiResponse } from '../../types';
import api from '../api';

export function useSessions() {
  const [sessions, setSessions] = useState<SessionGroup[]>([]);
  const [totalConsumption, setTotalConsumption] = useState<string>('0');
  const [loading, setLoading] = useState<boolean>(true);
  const [error, setError] = useState<Error | null>(null);

  const fetchSessions = useCallback(async () => {
    try {
      setLoading(true);
        setError(null);
      const data: ApiResponse = await api.sessions.list();
        setSessions(data.sessions);
      setTotalConsumption(data.totalConsumption);
    } catch (err) {
      if (err instanceof Error) {
        setError(err);
      } else {
        setError(new Error('An unknown error occurred'));
      }
      console.error(err);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchSessions();
  }, [fetchSessions]);

  return { sessions, totalConsumption, loading, error, refetch: fetchSessions };
}