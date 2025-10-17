export interface Device {
  isOn: boolean;
  isHeating: boolean;
}

export interface HeatCycle {
  created_at: number;
  duration: number;
  cycle: 1 | 2 | 3 | 4;
}

export interface Session {
  startedAt: number;
  endedAt: number;
  caps: number;
  consumption: number;
  heatCycles: HeatCycleRow[];
}

export interface RunningSession {
  startedAt: number;
  lastAt: number;
  caps: number;
  clicks: number;
  consumption: number;
  heatCycles: HeatCycleRow[];
}

/**
 * Ein einzelner Heizzyklus, wie er von der Datenbank kommt.
 */
export interface HeatCycleRow extends HeatCycle {
  id: string;
}

/**
 * Eine Gruppe von zusammengehörigen Heizzyklen, die eine "Session" bilden.
 */
export type HeatCycleRows = HeatCycleRow[];

/**
 * Die Antwort der API für die Heat-Cycle-Liste.
 */
export interface HeatCyclesApiResponse {
  heatCycles: HeatCycleRows[];
  totalConsumption: string;
  totalHeatCycles: number;
}

/**
 * Die Antwort der API für die aggregierten Statistiken.
 */
export interface StatisticsApiResponse {
  range: "day" | "week" | "month";
  totalHeatCycles: number;
  totalDuration: number;
  heatCycles: Pick<HeatCycleRow, "created_at" | "duration" | "cycle">[];
}

/**
 * Die Antwort der API für die Session-Daten.
 */
export interface SessionApiResponse {
  totalDuration: number;
  clickCount: number;
  lastClickAt: string | null;
}

// --- WebSocket Payloads --- //

export interface DeviceStatusPayload {
  isOn: boolean;
  isHeating: boolean;
}

export interface HeatCyclePayload extends HeatCycleRow {}

/**
 * Eine diskriminierte Union für alle möglichen Events, die vom Server gesendet werden.
 */
export type ServerWebSocketMessage =
  | { type: "statusUpdate"; payload: Partial<DeviceStatusPayload> }
  | { type: "heatCycleCreated"; payload: HeatCyclePayload }
  | { type: "heatCycleCompleted"; payload: HeatCyclePayload }
  | { type: "sessionCreated" }
  | { type: "sessionData"; payload: SessionData }
  | { type: "error"; payload: { message: string } };

/**
 * Eine diskriminierte Union für alle möglichen Events, die vom Client gesendet werden.
 */
export type ClientWebSocketMessage = { type: "getDeviceStatus" };

// Stash-Tracking Type Definitions

export interface StashItemRow {
  id: string;
  item_name: string;
  initial_amount: number;
  current_amount: number;
  added_at: number;
  device_id: string | null;
  notes: string | null;
}

export interface StashWithdrawalRow {
  id: string;
  item_id: string;
  amount: number;
  taken_at: number;
  device_id: string | null;
  notes: string | null;
}

export interface StashItemWithHistory extends StashItemRow {
  total_withdrawn: number;
  withdrawal_count: number;
  last_withdrawal: number | null;
}

export interface CreateStashItemRequest {
  item_name: string;
  initial_amount: number;
  device_id?: string;
  notes?: string;
}

export interface WithdrawStashItemRequest {
  amount: number;
  notes?: string;
}

export interface StashStatsResponse {
  range: string;
  total_items: number;
  total_quantity_current: number;
  total_quantity_withdrawn: number;
  items: StashItemWithHistory[];
  daily_withdrawals?: Array<{
    date: string;
    total_quantity: number;
    withdrawal_count: number;
  }>;
}

export interface StashApiResponse {
  items: StashItemWithHistory[];
  total_current: number;
  total_withdrawn: number;
}

// WebSocket Messages
export interface StashUpdatedMessage {
  type: "stashUpdated";
  item?: StashItemRow;
  withdrawal?: StashWithdrawalRow;
}
