import { execSync } from "child_process";
import fs from "fs";

const dbName = "heizbox";
const dumpFile = "./heizbox-backup.sql";
const localDb = "./.wrangler/state/v3/d1/heizbox.db";

const tables = [
  "heat_cycles",
  "stash_items",
  "stash_withdrawals",
  "d1_migrations",
];

function run(cmd) {
  console.log(">", cmd);
  execSync(cmd, { stdio: "inherit", shell: true });
}

function fileContainsAllTables(filePath, requiredTables) {
  const sql = fs.readFileSync(filePath, "utf-8");
  return requiredTables.every((t) =>
    new RegExp(`CREATE TABLE\\s+${t}\\b`, "i").test(sql),
  );
}

// 1. Remote export
run(`pnpm exec wrangler d1 export ${dbName} --remote --output=${dumpFile}`);

// 2. Validierung
if (!fileContainsAllTables(dumpFile, tables)) {
  console.error("❌ Export unvollständig oder Tabellen fehlen!");
  console.log("Bitte manuell prüfen:", dumpFile);
  process.exit(1);
}

// 3. Alte Tabellen droppen via sqlite3
for (const t of tables) {
  run(`sqlite3 ${localDb} "DROP TABLE IF EXISTS ${t};"`);
}

// 4. Import des Dumps via sqlite3
run(`sqlite3 ${localDb} ".read ${dumpFile}"`);

// 5. Cleanup
fs.unlinkSync(dumpFile);
console.log("✅ DB-Sync erfolgreich abgeschlossen");
