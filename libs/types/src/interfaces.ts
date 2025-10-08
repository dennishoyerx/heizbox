export interface Session {
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

export interface SessionRow {
  id: string;
  created_at: string;
  duration: number;
  cycle: number;
}
