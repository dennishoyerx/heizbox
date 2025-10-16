export interface CreateHeatCycleParams {
  duration: number;
  cycle?: number;
}

export interface HeatCycleQueryParams {
  start: number;
  end: number;
}

export interface WebSocketConnectionParams {
  deviceId: string;
  type: 'device' | 'frontend';
}
