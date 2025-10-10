import { generateUuid } from '../utils';

export const getDb = (env: Env) => {
  return env.db;
};

export const insertHeatCycle = async (db: D1Database, created_at: string, duration: number) => {
  const id = generateUuid();
  await db.prepare("INSERT INTO heat_cycles (id, created_at, duration) VALUES (?1, ?2, ?3)")
    .bind(id, created_at, duration)
    .run();
};

interface Env {
  db: D1Database;
}
