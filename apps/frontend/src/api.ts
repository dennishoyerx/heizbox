import type { ApiResponse } from '../types';

export const fetchSessions = async (): Promise<ApiResponse> => {
  const response = await fetch('/api/sessions');
  if (!response.ok) {
    throw new Error(`HTTP error! status: ${response.status}`);
  }
  return response.json();
};
