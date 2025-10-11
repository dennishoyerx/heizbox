import { D1Database } from '@cloudflare/workers-types';
import { generateUuid } from '../utils';

export async function createHeatCycle(db: D1Database, duration: number, cycle = 1): Promise<boolean> {
  try {
    console.log(`createHeatCycle called with duration: ${duration}, cycle: ${cycle}`);
    if (isNaN(duration) || duration <= 0 || isNaN(cycle) || cycle <= 0) {
      console.error("Invalid duration or cycle provided to createHeatCycle");
      return false;
    }

    // Debounce: Check for recent identical entries
    const thirtySecondsAgo = Date.now() - 30000; // Unix timestamp in milliseconds
    const { count } = await db.prepare(
      "SELECT COUNT(*) as count FROM heat_cycles WHERE duration = ?1 AND cycle = ?2 AND created_at > ?3"
    ).bind(duration, cycle, thirtySecondsAgo).first<{ count: number }>();

    if (count > 0) {
      console.log("Duplicate heat cycle within 30s, not inserted.");
      return false;
    }

    const id = generateUuid();
    console.log(`Attempting to insert heat cycle: id=${id}, duration=${duration}, cycle=${cycle}`);
    await db.prepare(
      "INSERT INTO heat_cycles (id, duration, cycle) VALUES (?1, ?2, ?3)"
    ).bind(id, duration, cycle).run();

    console.log(`Heat cycle created: id=${id}, duration=${duration}, cycle=${cycle}`);
    return true;
  } catch (e: unknown) {
    console.error("Error in createHeatCycle:", e);
    return false;
  }
}
