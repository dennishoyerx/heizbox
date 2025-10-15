/**
 * Ein einzelner Heizzyklus, wie er von der Datenbank kommt.
 */
export interface HeatCycleRow {
  id: string;
  created_at: number; // Unix-Timestamp (Sekunden)
  duration: number; // in Sekunden
  cycle: number;
}

/**
 * Eine Gruppe von zusammengehörigen Heizzyklen, die eine "Session" bilden.
 */
export type HeatCycleGroup = HeatCycleRow[];

/**
 * Die Antwort der API für die Heat-Cycle-Liste.
 */
export interface HeatCyclesApiResponse {
  heatCycles: HeatCycleGroup[];
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
  heatCycles: Pick<HeatCycleRow, "created_at" | "duration">[];
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
  | { type: "deviceStatusChanged"; payload: DeviceStatusPayload }
  | { type: "heatCycleCreated"; payload: HeatCyclePayload }
  | { type: "heatCycleCompleted"; payload: HeatCyclePayload }
  | { type: "error"; payload: { message: string } };

/**
 * Eine diskriminierte Union für alle möglichen Events, die vom Client gesendet werden.
 */
export type ClientWebSocketMessage = { type: "getDeviceStatus" };
