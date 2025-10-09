import { D1Database } from '@cloudflare/workers-types';

export async function createSession(db: D1Database, duration: number, cycle = 1): Promise<boolean> {
  try {
    console.log(`createSession called with duration: ${duration}, cycle: ${cycle}`);
    if (isNaN(duration) || duration <= 0 || isNaN(cycle) || cycle <= 0) {
      console.error("Invalid duration or cycle provided to createSession");
      return false;
    }

    // Debounce: Check for recent identical entries
    const thirtySecondsAgo = new Date(Date.now() - 30000).toISOString();
    const { count } = await db.prepare(
      "SELECT COUNT(*) as count FROM sessions WHERE duration = ?1 AND cycle = ?2 AND created_at > ?3"
    ).bind(duration, cycle, thirtySecondsAgo).first<{ count: number }>();

    if (count > 0) {
      console.log("Duplicate session within 30s, not inserted.");
      return false;
    }

    const now = new Date().toISOString();
    console.log(`Attempting to insert session: duration=${duration}, cycle=${cycle}, created_at=${now}`);
    await db.prepare(
      "INSERT INTO sessions (duration, cycle, created_at) VALUES (?1, ?2, ?3)"
    ).bind(duration, cycle, now).run();

    console.log(`Session created: duration=${duration}, cycle=${cycle}`);
    return true;
  } catch (e: unknown) {
    console.error("Error in createSession:", e);
    return false;
  }
}
