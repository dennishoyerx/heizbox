CREATE TABLE IF NOT EXISTS heat_cycles (
  id TEXT PRIMARY KEY,
  duration INTEGER,
  cycle INTEGER,
  created_at INTEGER DEFAULT (
    unixepoch()
  )
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
