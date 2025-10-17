// File: apps/backend/src/services/stashService.ts
import type { D1Database } from "@cloudflare/workers-types";
import { StashRepository } from "../repositories/stashRepository.js";
import { generateUuid } from "../utils/index.js";

export class StashService {
  private repository: StashRepository;

  constructor(db: D1Database) {
    this.repository = new StashRepository(db);
  }

  async createStashItem(
    name: string,
    initial_amount: number,
  ) {
    if (!name || name.trim().length === 0) {
      throw new Error("Item name is required");
    }
    if (initial_amount < 0) {
      throw new Error("Quantity must be non-negative");
    }

    const id = generateUuid();
    await this.repository.createItem(
      id,
      name.trim(),
      initial_amount,
    );

    return this.repository.findItemById(id);
  }

  async getStashItems() {
    return this.repository.findItemsWithHistory();
  }

  async getStashItemById(id: string) {
    return this.repository.findItemById(id);
  }

  async withdrawFromStash(
    item_id: string,
    amount: number,
  ) {
    if (amount <= 0) {
      throw new Error("Withdrawal quantity must be positive");
    }

    const item = await this.repository.findItemById(item_id);
    if (!item) {
      throw new Error("Stash item not found");
    }

    if (item.current_amount < amount) {
      throw new Error(
        `Insufficient quantity. Available: ${item.current_amount}g, requested: ${amount}g`,
      );
    }

    const withdrawal_id = generateUuid();
    await this.repository.createWithdrawal(withdrawal_id, item_id, amount);

    const new_quantity = item.current_amount - amount;
    await this.repository.updateItemQuantity(item_id, new_quantity);

    return {
      item: await this.repository.findItemById(item_id),
      withdrawal: {
        id: withdrawal_id,
        stash_item_id: item_id,
        amount,
        withdrawn_at: Math.floor(Date.now() / 1000),
      },
    };
  }

  async deleteStashItem(id: string) {
    const item = await this.repository.findItemById(id);
    if (!item) {
      throw new Error("Stash item not found");
    }
    await this.repository.deleteItem(id);
    return true;
  }

  async getStashStats(range: string) {
    const now = Math.floor(Date.now() / 1000);
    let start: number;

    switch (range) {
      case "7d":
        start = now - 7 * 24 * 60 * 60;
        break;
      case "30d":
        start = now - 30 * 24 * 60 * 60;
        break;
      case "90d":
        start = now - 90 * 24 * 60 * 60;
        break;
      default:
        start = now - 30 * 24 * 60 * 60;
    }

    const items = await this.repository.findItemsWithHistory();
    const dailyWithdrawals = await this.repository.getDailyWithdrawalStats(
      start,
      now,
    );

    const total_current = items.reduce(
      (sum, item) => sum + item.current_amount,
      0,
    );
    const total_withdrawn = items.reduce(
      (sum, item) => sum + item.total_withdrawn,
      0,
    );

    return {
      range,
      total_items: items.length,
      total_quantity_current: total_current,
      total_quantity_withdrawn: total_withdrawn,
      items,
      daily_withdrawals: dailyWithdrawals,
    };
  }

  async getWithdrawalsByItem(item_id: string) {
    return this.repository.findWithdrawalsByItem(item_id);
  }
}
