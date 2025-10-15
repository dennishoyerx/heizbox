import type { ApiResponse, SessionApiResponse } from "@heizbox/types";
import type { StatisticsData } from "./types";

const API_BASE_URL = import.meta.env.VITE_PUBLIC_API_URL;

// Ein benutzerdefinierter Error für API-Fehler
export class ApiError extends Error {
  constructor(
    message: string,
    public status: number,
  ) {
    super(message);
    this.name = "ApiError";
  }
}

// Wrapper für fetch
async function apiFetch<T>(url: string, options?: RequestInit): Promise<T> {
  const response = await fetch(`${API_BASE_URL}${url}`, options);
  if (!response.ok) {
    const errorBody = await response.text();
    console.error("API Error Response:", errorBody);
    throw new ApiError(
      `HTTP error! status: ${response.status}`,
      response.status,
    );
  }
  return response.json() as Promise<T>;
}

// Aufrufe werden schlanker
export const fetchHeatCycles = () => apiFetch<ApiResponse>("/api/heat_cycles");
export const fetchStatistics = (range: string) =>
  apiFetch<StatisticsData>(`/api/statistics?range=${range}`);
export const fetchSession = () => apiFetch<SessionApiResponse>("/api/session");
