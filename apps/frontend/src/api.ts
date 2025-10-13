import type { ApiResponse, SessionApiResponse } from '@heizbox/types';
import type { StatisticsData } from './types';

const API_BASE_URL = import.meta.env.VITE_PUBLIC_API_URL;

export const fetchHeatCycles = async (): Promise<ApiResponse> => {
  const response = await fetch(`${API_BASE_URL}/api/heat_cycles`);
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return response.json();
};

export const fetchStatistics = async (range: string): Promise<StatisticsData> => {
  const response = await fetch(`${API_BASE_URL}/api/statistics?range=${range}`);
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return response.json();
};

export const fetchSession = async (): Promise<SessionApiResponse> => {
  const response = await fetch(`${API_BASE_URL}/api/session`);
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return response.json();
};
