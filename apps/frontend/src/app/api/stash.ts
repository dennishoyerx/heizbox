// File: apps/frontend/src/api-stash.ts
import type {
  StashApiResponse,
  StashStatsResponse,
  CreateStashItemRequest,
  WithdrawStashItemRequest,
  StashItemRow,
  StashWithdrawalRow,
} from '@heizbox/types';

const API_BASE_URL = import.meta.env.VITE_PUBLIC_API_URL;

export class ApiError extends Error {
  constructor(message: string, public status: number) {
    super(message);
    this.name = 'ApiError';
  }
}

async function apiFetch<T>(url: string, options?: RequestInit): Promise<T> {
  const response = await fetch(`${API_BASE_URL}${url}`, options);

  if (!response.ok) {
    const errorBody = await response.text();
    console.error('API Error Response:', errorBody);
    throw new ApiError(`HTTP error! status: ${response.status}`, response.status);
  }
  return response.json() as Promise<T>;
}

// Liste aller Stash-Items mit Statistiken
export const fetchStashItems = () => apiFetch<StashApiResponse>('/api/stash');

// Neues Stash-Item anlegen
export const createStashItem = (data: CreateStashItemRequest) =>
  apiFetch<StashItemRow>('/api/stash', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data),
  });

// Entnahme aus Stash durchführen
export const withdrawStashItem = (itemId: string, data: WithdrawStashItemRequest) =>
  apiFetch<{ item: StashItemRow; withdrawal: StashWithdrawalRow }>(
    `/api/stash/${itemId}/withdraw`,
    {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data),
    }
  );

// Stash-Item löschen
export const deleteStashItem = (itemId: string) =>
  apiFetch<{ success: boolean }>(`/api/stash/${itemId}`, {
    method: 'DELETE',
  });

// Stash-Statistiken abrufen
export const fetchStashStats = (range: string = '30d') =>
  apiFetch<StashStatsResponse>(`/api/stash/stats?range=${range}`);

// Entnahme-Historie für ein Item
export const fetchItemWithdrawals = (itemId: string) =>
  apiFetch<StashWithdrawalRow[]>(`/api/stash/${itemId}/withdrawals`);
