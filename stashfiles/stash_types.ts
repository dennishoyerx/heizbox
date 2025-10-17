// File: libs/types/src/stash.ts
// Stash-Tracking Type Definitions

export interface StashItemRow {
  id: string;
  item_name: string;
  quantity_start: number;
  quantity_current: number;
  added_at: number;
  device_id: string | null;
  notes: string | null;
}

export interface StashWithdrawalRow {
  id: string;
  item_id: string;
  quantity: number;
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
  quantity_start: number;
  device_id?: string;
  notes?: string;
}

export interface WithdrawStashItemRequest {
  quantity: number;
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
  type: 'stashUpdated';
  item?: StashItemRow;
  withdrawal?: StashWithdrawalRow;
}

// Export für libs/types/src/index.ts
export type {
  StashItemRow,
  StashWithdrawalRow,
  StashItemWithHistory,
  CreateStashItemRequest,
  WithdrawStashItemRequest,
  StashStatsResponse,
  StashApiResponse,
  StashUpdatedMessage,
};