import type {
	HeatCyclesApiResponse,
	SessionApiResponse,
	StatisticsApiResponse,
	StashItemWithHistory,
	StashApiResponse,
	CreateStashItemRequest,
	WithdrawStashItemRequest,
} from '@heizbox/types'

const API_BASE_URL = import.meta.env.VITE_PUBLIC_API_URL

// Ein benutzerdefinierter Error für API-Fehler
export class ApiError extends Error {
	constructor(
		message: string,
		public status: number,
	) {
		super(message)
		this.name = 'ApiError'
	}
}

// Wrapper für fetch
async function apiFetch<T>(url: string, options?: RequestInit): Promise<T> {
	const response = await fetch(`${API_BASE_URL}${url}`, options)

	if (!response.ok) {
		const errorBody = await response.text()
		console.error('API Error Response:', errorBody)
		throw new ApiError(`HTTP error! status: ${response.status}`, response.status)
	}
	return response.json() as Promise<T>
}

// Aufrufe sind jetzt vollständig typsicher
export const fetchHeatCycles = () => apiFetch<HeatCyclesApiResponse>('/api/heat_cycles')

export const fetchStatistics = (range: string) => apiFetch<StatisticsApiResponse>(`/api/statistics?range=${range}`)

export const fetchSession = () => apiFetch<SessionApiResponse>('/api/session')

export const fetchRecentWithdrawals = () =>
	apiFetch<{ name: string; amount: number; withdrawn_at: string }[]>('/api/stash_withdrawals/recent')

export const fetchStashItems = () => apiFetch<StashApiResponse>('/api/stash')

export const createStashItem = (data: CreateStashItemRequest) =>
	apiFetch<StashItemWithHistory>('/api/stash', {
		method: 'POST',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify(data),
	})

export const withdrawStashItem = (itemId: string, data: WithdrawStashItemRequest) =>
	apiFetch<StashItemWithHistory>(`/api/stash/${itemId}/withdraw`, {
		method: 'PUT',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify(data),
	})

export const deleteStashItem = (itemId: string) =>
	apiFetch<void>(`/api/stash/${itemId}`, {
		method: 'DELETE',
	})

export const fetchLogs = (limit?: number) => {
	const url = limit ? `/api/logs?limit=${limit}` : '/api/logs'
	return apiFetch<{ timestamp: number; log_type: string; message: string }[]>(url)
}
