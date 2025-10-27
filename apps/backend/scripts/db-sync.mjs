import { execSync } from 'child_process'
import fs from 'fs'

const dbName = 'heizbox'
const dumpFile = './heizbox-backup.sql'
const localDb = './.wrangler/state/v3/d1/heizbox.db'

const tables = ['heat_cycles', 'stash_items', 'stash_withdrawals', 'd1_migrations', 'logs']

function fixDump() {
	const dump = fs.readFileSync(dumpFile, 'utf-8')

	// 1. main. entfernen
	const cleanedDump = dump.replace(/main\./g, '')

	// 2. CREATE TABLE + INSERT INTO Statements extrahieren
	const createStmts = cleanedDump.match(/CREATE TABLE [\s\S]+?;/gi) || []
	const insertStmts = cleanedDump.match(/INSERT INTO [\s\S]+?;/gi) || []

	// 3. FK-basierte Sortierung der CREATE TABLEs
	// Einfacher Ansatz: Tabellen ohne FK zuerst, Tabellen mit FK später
	const tableDeps = createStmts.map((stmt) => {
		const matchName = stmt.match(/CREATE TABLE\s+(\S+)\s*\(/i)
		const name = matchName ? matchName[1] : null
		const fkMatches = [...stmt.matchAll(/REFERENCES\s+(\S+)/gi)].map((m) => m[1])
		return { name, stmt, deps: fkMatches }
	})

	// Topological Sort (einfacher Ansatz für kleine Anzahl Tabellen)
	const sortedCreate = []
	const visited = new Set()

	function visit(t) {
		if (visited.has(t.name)) return
		for (const dep of t.deps) {
			const depTable = tableDeps.find((x) => x.name === dep)
			if (depTable) visit(depTable)
		}
		sortedCreate.push(t.stmt)
		visited.add(t.name)
	}

	tableDeps.forEach((t) => visit(t))

	// 4. INSERTs nach Tabellen sortieren (FK-Ziele zuerst)
	const sortedInsert = insertStmts.sort((a, b) => {
		for (const t of tables) {
			if (a.includes(t)) return -1
			if (b.includes(t)) return 1
		}
		return 0
	})

	// 5. Dump zusammenbauen mit PRAGMA foreign_keys=OFF für sicheren Import
	const fixedDump = [
		'PRAGMA foreign_keys=OFF;',
		'BEGIN TRANSACTION;',
		...sortedCreate,
		...sortedInsert,
		'COMMIT;',
		'PRAGMA foreign_keys=ON;',
	].join('\n\n')

	fs.writeFileSync(dumpFile, fixedDump)
	console.log('✅ Dump fix fertig!')
}

function run(cmd) {
	console.log('>', cmd)
	execSync(cmd, { stdio: 'inherit', shell: true })
}

function fileContainsAllTables(filePath, requiredTables) {
	const sql = fs.readFileSync(filePath, 'utf-8')
	return requiredTables.every((t) => new RegExp(`CREATE TABLE\\s+${t}\\b`, 'i').test(sql))
}

// 1. Remote export
run(`pnpm exec wrangler d1 export ${dbName} --remote --output=${dumpFile}`)

// 2. Validierung
if (!fileContainsAllTables(dumpFile, tables)) {
	console.error('❌ Export unvollständig oder Tabellen fehlen!')
	console.log('Bitte manuell prüfen:', dumpFile)
	process.exit(1)
}

fixDump()

// 3. Alte Tabellen droppen via Wrangler
for (const t of tables) {
	run(`pnpm exec wrangler d1 execute ${dbName} --local --command="DROP TABLE IF EXISTS ${t};"`)
}

// 4. Import des Dumps
run(`pnpm exec wrangler d1 execute ${dbName} --local --file=${dumpFile}`)

// 5. Cleanup
fs.unlinkSync(dumpFile)
console.log('✅ DB-Sync erfolgreich abgeschlossen')
