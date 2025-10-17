// File: apps/backend/src/repositories/stashRepository.ts
import type { D1Database } from "@cloudflare/workers-types";
import type {
  StashItemRow,
  StashWithdrawalRow,
  StashItemWithHistory,
} from "@heizbox/types";

export class StashRepository {
  constructor(private db: D1Database) {}

  async findAllItems(): Promise<StashItemRow[]> {
    const { results } = await this.db
      .prepare(
        "SELECT id, name, initial_amount, COALESCE(current_amount, 0) as current_amount, created_at, updated_at FROM stash_items ORDER BY created_at DESC",
      )
      .all<StashItemRow>();
    return results || [];
  }

  async findItemById(id: string): Promise<StashItemRow | null> {
    const result = await this.db
      .prepare(
        "SELECT id, name, initial_amount, COALESCE(current_amount, 0) as current_amount, created_at, updated_at FROM stash_items WHERE id = ?1",
      )
      .bind(id)
      .first<StashItemRow>();
    return result || null;
  }

  async findItemsWithHistory(): Promise<StashItemWithHistory[]> {
    const { results } = await this.db
      .prepare(
        `
        SELECT
          s.id,
          s.name,
          s.initial_amount,
          s.current_amount,
          s.created_at,
          s.updated_at,
          COALESCE(SUM(w.amount), 0) as total_withdrawn,
          COUNT(w.id) as withdrawal_count,
          MAX(w.withdrawn_at) as last_withdrawal
        FROM stash_items s
        LEFT JOIN stash_withdrawals w ON s.id = w.stash_item_id
        GROUP BY s.id
        ORDER BY s.created_at DESC
      `,
      )
      .all<StashItemWithHistory>();
    return results || [];
  }

  async createItem(
    id: string,
    name: string,
    initial_amount: number,
  ): Promise<void> {
    await this.db
      .prepare(
        "INSERT INTO stash_items (id, name, initial_amount, current_amount) VALUES (?1, ?2, ?3, ?3)",
      )
      .bind(id, name, initial_amount)
      .run();
  }

  async updateItemQuantity(id: string, new_quantity: number): Promise<void> {
    await this.db
      .prepare("UPDATE stash_items SET current_amount = ?1 WHERE id = ?2")
      .bind(new_quantity, id)
      .run();
  }

  async deleteItem(id: string): Promise<void> {
    await this.db
      .prepare("DELETE FROM stash_items WHERE id = ?1")
      .bind(id)
      .run();
  }

  async createWithdrawal(
    id: string,
    stash_item_id: string,
    amount: number,
  ): Promise<void> {
    await this.db
      .prepare(
        "INSERT INTO stash_withdrawals (id, stash_item_id, amount) VALUES (?1, ?2, ?3)",
      )
      .bind(id, stash_item_id, amount)
      .run();
  }

  async findWithdrawalsByTimeRange(
    start: number,
    end: number,
  ): Promise<StashWithdrawalRow[]> {
    const { results } = await this.db
      .prepare(
        "SELECT id, stash_item_id, amount, withdrawn_at FROM stash_withdrawals WHERE withdrawn_at >= ?1 AND withdrawn_at < ?2 ORDER BY withdrawn_at DESC",
      )
      .bind(start, end)
      .all<StashWithdrawalRow>();
    return results || [];
  }

  async findWithdrawalsByItem(
    stash_item_id: string,
  ): Promise<StashWithdrawalRow[]> {
    const { results } = await this.db
      .prepare(
        "SELECT id, stash_item_id, amount, withdrawn_at FROM stash_withdrawals WHERE stash_item_id = ?1 ORDER BY withdrawn_at DESC",
      )
      .bind(stash_item_id)
      .all<StashWithdrawalRow>();
    return results || [];
  }

  async getDailyWithdrawalStats(
    start: number,
    end: number,
  ): Promise<
    Array<{ date: string; total_amount: number; withdrawal_count: number }>
  > {
    const { results } = await this.db
      .prepare(
        `
        SELECT
          date(withdrawn_at, 'unixepoch') as date,
          SUM(amount) as total_amount,
          COUNT(*) as withdrawal_count
        FROM stash_withdrawals
        WHERE withdrawn_at >= ?1 AND withdrawn_at < ?2
        GROUP BY date(withdrawn_at, 'unixepoch')
        ORDER BY date ASC
      `,
      )
      .bind(start, end)
      .all<{
        date: string;
        total_amount: number;
        withdrawal_count: number;
      }>();
    return results || [];
  }
}
