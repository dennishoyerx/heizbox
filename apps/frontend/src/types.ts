export interface SessionData {
  created_at: number;
  duration: number;
}

export interface StatisticsData {
  range: string;
  totalHeatCycles: number;
  totalDuration: number;
  heatCycles: HeatCycleData[];
}
