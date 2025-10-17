-- Create stash_items table
CREATE TABLE IF NOT EXISTS stash_items (
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    initial_amount REAL NOT NULL,
    current_amount REAL NOT NULL,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL,
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL
);

-- Create stash_withdrawals table
CREATE TABLE IF NOT EXISTS stash_withdrawals (
    id TEXT PRIMARY KEY NOT NULL,
    stash_item_id TEXT NOT NULL,
    amount REAL NOT NULL,
    withdrawn_at TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL,
    FOREIGN KEY (stash_item_id) REFERENCES stash_items(id) ON DELETE CASCADE
);

-- Optimized indices for performance
CREATE INDEX IF NOT EXISTS idx_stash_items_name ON stash_items(name);
CREATE INDEX IF NOT EXISTS idx_stash_withdrawals_stash_item_id ON stash_withdrawals(stash_item_id);
