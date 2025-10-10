export interface SessionData {
  created_at: string;
  duration: number;
}

export interface StatisticsData {
  range: string;
  totalSessions: number;
  totalDuration: number;
  sessions: SessionData[];
}
