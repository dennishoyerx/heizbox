export const getDb = (env: Env) => {
  return env.db;
};

export const insertSession = async (db: D1Database, created_at: string, duration: number) => {
  await db.prepare("INSERT INTO sessions (created_at, duration) VALUES (?, ?)")
    .bind(created_at, duration)
    .run();
};

interface Env {
  db: D1Database;
}
