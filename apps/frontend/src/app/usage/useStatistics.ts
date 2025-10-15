import { useQueries } from '@tanstack/react-query';
import { fetchStatistics } from '../../api';

export const useStatistics = () => {
  const ranges = ['day', 'week', 'month'];

  const results = useQueries({
    queries: ranges.map(range => ({
      queryKey: ['statistics', range],
      queryFn: () => fetchStatistics(range),
    })),
  });

  const isLoading = results.some(result => result.isLoading);
  const isError = results.some(result => result.isError);
  const error = results.find(result => result.error)?.error as Error | null;

  const [dailyStats, weeklyStats, monthlyStats] = results.map(result => result.data);

  return {
    dailyStats,
    weeklyStats,
    monthlyStats,
    isLoading,
    isError,
    error,
  };
};
