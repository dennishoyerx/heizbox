import { Database } from "bun:sqlite";
import { mkdirSync } from "fs";
import { dirname } from "path";

// DB-Pfad: env oder default ./data/heizbox.db
const dbPath = process.env.DB_PATH || "./data/heizbox.db";
mkdirSync(dirname(dbPath), { recursive: true });

export const sqlite = new Database(dbPath);
sqlite.exec("PRAGMA journal_mode = WAL;");
sqlite.exec("PRAGMA foreign_keys = ON;");

// ---- Schema (D1-kompatibel + temp_readings) ----
sqlite.exec(`
CREATE TABLE IF NOT EXISTS heat_cycles (
  id TEXT PRIMARY KEY,
  duration INTEGER,
  cycle INTEGER,
  created_at INTEGER DEFAULT (unixepoch())
);
CREATE TABLE IF NOT EXISTS stash_items (
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    initial_amount REAL NOT NULL,
    current_amount REAL NOT NULL,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL,
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL
);
CREATE TABLE IF NOT EXISTS stash_withdrawals (
    id TEXT PRIMARY KEY NOT NULL,
    stash_item_id TEXT NOT NULL,
    amount REAL NOT NULL,
    withdrawn_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL,
    FOREIGN KEY (stash_item_id) REFERENCES stash_items(id) ON DELETE CASCADE
);
CREATE TABLE IF NOT EXISTS logs (
  id TEXT PRIMARY KEY,
  device_id TEXT NOT NULL,
  log_type TEXT,
  message TEXT NOT NULL,
  timestamp INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS temp_readings (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  device_id TEXT NOT NULL DEFAULT 'HeizboxESP32',
  timestamp INTEGER NOT NULL,
  temp_raw REAL,
  temp_calibrated REAL,
  is_heating INTEGER DEFAULT 0
);
CREATE INDEX IF NOT EXISTS idx_temp_readings_ts ON temp_readings(timestamp);
`);

// ---- D1-kompatibles Statement (prepare/bind/all/first/run) ----
class D1Statement {
  private stmt: ReturnType<Database["prepare"]>;
  private args: unknown[] = [];
  constructor(stmt: ReturnType<Database["prepare"]>) {
    this.stmt = stmt;
  }
  bind(...args: unknown[]) {
    this.args = args;
    return this;
  }
  async all<T = Record<string, unknown>>(): Promise<{ results: T[] }> {
    const rows = this.stmt.all(...(this.args as never[])) as T[];
    return { results: rows };
  }
  async first<T = Record<string, unknown>>(): Promise<T | null> {
    const row = this.stmt.get(...(this.args as never[])) as T | undefined;
    return row ?? null;
  }
  async run(): Promise<{ success: boolean }> {
    this.stmt.run(...(this.args as never[]));
    return { success: true };
  }
}

export const db = {
  prepare(sql: string) {
    return new D1Statement(sqlite.prepare(sql));
  },
};

export type DB = typeof db;
