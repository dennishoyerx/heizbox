import type { ApiResponse } from '@heizbox/types';

const API_BASE_URL = import.meta.env.VITE_PUBLIC_API_URL;

export const fetchSessions = async (): Promise<ApiResponse> => {
  const response = await fetch(`${API_BASE_URL}/api/sessions`);
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return response.json();
};

export const fetchStatistics = async (range: string): Promise<any> => {
  const response = await fetch(`${API_BASE_URL}/api/statistics?range=${range}`);
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return response.json();
};
