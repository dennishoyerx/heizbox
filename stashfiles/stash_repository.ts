// File: apps/backend/src/repositories/stashRepository.ts
import type { D1Database } from '@cloudflare/workers-types';
import type {
  StashItemRow,
  StashWithdrawalRow,
  StashItemWithHistory,
} from '@heizbox/types';

export class StashRepository {
  constructor(private db: D1Database) {}

  async findAllItems(): Promise<StashItemRow[]> {
    const { results } = await this.db
      .prepare(
        'SELECT id, item_name, quantity_start, quantity_current, added_at, device_id, notes FROM stash_items ORDER BY added_at DESC'
      )
      .all<StashItemRow>();
    return results || [];
  }

  async findItemById(id: string): Promise<StashItemRow | null> {
    const result = await this.db
      .prepare(
        'SELECT id, item_name, quantity_start, quantity_current, added_at, device_id, notes FROM stash_items WHERE id = ?1'
      )
      .bind(id)
      .first<StashItemRow>();
    return result || null;
  }

  async findItemsWithHistory(): Promise<StashItemWithHistory[]> {
    const { results } = await this.db
      .prepare(`
        SELECT 
          s.id,
          s.item_name,
          s.quantity_start,
          s.quantity_current,
          s.added_at,
          s.device_id,
          s.notes,
          COALESCE(SUM(w.quantity), 0) as total_withdrawn,
          COUNT(w.id) as withdrawal_count,
          MAX(w.taken_at) as last_withdrawal
        FROM stash_items s
        LEFT JOIN stash_withdrawals w ON s.id = w.item_id
        GROUP BY s.id
        ORDER BY s.added_at DESC
      `)
      .all<StashItemWithHistory>();
    return results || [];
  }

  async createItem(
    id: string,
    item_name: string,
    quantity_start: number,
    device_id: string | null,
    notes: string | null
  ): Promise<void> {
    await this.db
      .prepare(
        'INSERT INTO stash_items (id, item_name, quantity_start, quantity_current, device_id, notes) VALUES (?1, ?2, ?3, ?3, ?4, ?5)'
      )
      .bind(id, item_name, quantity_start, device_id, notes)
      .run();
  }

  async updateItemQuantity(id: string, new_quantity: number): Promise<void> {
    await this.db
      .prepare('UPDATE stash_items SET quantity_current = ?1 WHERE id = ?2')
      .bind(new_quantity, id)
      .run();
  }

  async deleteItem(id: string): Promise<void> {
    await this.db
      .prepare('DELETE FROM stash_items WHERE id = ?1')
      .bind(id)
      .run();
  }

  async createWithdrawal(
    id: string,
    item_id: string,
    quantity: number,
    device_id: string | null,
    notes: string | null
  ): Promise<void> {
    await this.db
      .prepare(
        'INSERT INTO stash_withdrawals (id, item_id, quantity, device_id, notes) VALUES (?1, ?2, ?3, ?4, ?5)'
      )
      .bind(id, item_id, quantity, device_id, notes)
      .run();
  }

  async findWithdrawalsByTimeRange(
    start: number,
    end: number
  ): Promise<StashWithdrawalRow[]> {
    const { results } = await this.db
      .prepare(
        'SELECT id, item_id, quantity, taken_at, device_id, notes FROM stash_withdrawals WHERE taken_at >= ?1 AND taken_at < ?2 ORDER BY taken_at DESC'
      )
      .bind(start, end)
      .all<StashWithdrawalRow>();
    return results || [];
  }

  async findWithdrawalsByItem(item_id: string): Promise<StashWithdrawalRow[]> {
    const { results } = await this.db
      .prepare(
        'SELECT id, item_id, quantity, taken_at, device_id, notes FROM stash_withdrawals WHERE item_id = ?1 ORDER BY taken_at DESC'
      )
      .bind(item_id)
      .all<StashWithdrawalRow>();
    return results || [];
  }

  async getDailyWithdrawalStats(
    start: number,
    end: number
  ): Promise<Array<{ date: string; total_quantity: number; withdrawal_count: number }>> {
    const { results } = await this.db
      .prepare(`
        SELECT 
          date(taken_at, 'unixepoch') as date,
          SUM(quantity) as total_quantity,
          COUNT(*) as withdrawal_count
        FROM stash_withdrawals
        WHERE taken_at >= ?1 AND taken_at < ?2
        GROUP BY date(taken_at, 'unixepoch')
        ORDER BY date ASC
      `)
      .bind(start, end)
      .all<{ date: string; total_quantity: number; withdrawal_count: number }>();
    return results || [];
  }
}