export interface SessionData {
  created_at: string;
  duration: number;
}

export interface StatisticsData {
  range: string;
  totalHeatCycles: number;
  totalDuration: number;
  heatCycles: HeatCycleData[];
}
