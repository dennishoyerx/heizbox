export type SessionRow = {
  id: number;
  created_at: string;
  duration: number;
};

export type SessionGroup = SessionRow[];

export interface ApiResponse {
  sessions: SessionGroup[];
  totalConsumption: string;
}
