import type { D1Database } from '@cloudflare/workers-types';
import { HeatCycleRepository } from '../repositories/heatCycleRepository';
import { validateHeatCycle } from '../utils/validation';
import { generateUuid } from '../utils';

export class HeatCycleService {
  private repository: HeatCycleRepository;

  constructor(db: D1Database) {
    this.repository = new HeatCycleRepository(db);
  }

  async createHeatCycle(duration: number, cycle: number = 1): Promise<boolean> {
    try {
      validateHeatCycle(duration, cycle);

      // Debounce: Check for recent identical entries
      const duplicateCount = await this.repository.findDuplicates(
        duration,
        cycle,
        30000 // 30 seconds
      );

      if (duplicateCount > 0) {
        console.log('Duplicate heat cycle within 30s, not inserted.');
        return false;
      }

      const id = generateUuid();
      await this.repository.create(id, duration, cycle);

      console.log(`Heat cycle created: id=${id}, duration=${duration}, cycle=${cycle}`);
      return true;
    } catch (error) {
      console.error('Error in createHeatCycle:', error);
      return false;
    }
  }

  async getHeatCyclesInRange(start: number, end: number) {
    return this.repository.findByTimeRange(start, end);
  }

  async getAllHeatCycles() {
    return this.repository.findAll();
  }

  async getRecentSession(timeLimitSeconds: number = 7200) {
    return this.repository.findRecentForSession(timeLimitSeconds);
  }
}
