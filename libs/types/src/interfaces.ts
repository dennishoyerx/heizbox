export interface HeatCycle {
  id: string;
  startTime: Date;
  endTime: Date;
  duration: number;
  cycle: number;
}

export interface Device {
  id: string;
  name: string;
  location: string;
}

export interface HeatCycleRow {
  id: string;
  created_at: number;
  duration: number;
  cycle: number;
}

export interface WebSocketMessage {
  type: string;
  [key: string]: any;
}

export interface HeatCycleData {
  clicks: number;
  lastClick: number;
  sessionStart: number;
}

export type HeatCycleGroup = HeatCycleRow[];
