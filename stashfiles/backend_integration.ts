// ============================================
// File: apps/backend/src/index.ts (Erweiterung)
// ============================================
// Import hinzufügen:
// import stashRoute from "./routes/stash.js";

// Route registrieren (nach anderen Routes):
// app.route("/api/stash", stashRoute);

// ============================================
// File: apps/backend/src/durable-objects/DeviceStatus.ts
// In der processDeviceMessage Methode hinzufügen:
// ============================================
/*
} else if (message.type === 'stashUpdated') {
  console.log('DeviceStatus: Processing stashUpdated message.', message);
  this.publish(message); // Broadcast to all subscribers
}
*/

// ============================================
// Wrangler.toml bleibt unverändert
// D1-Datenbank und Durable Objects sind bereits konfiguriert
// ============================================

// ============================================
// Migration ausführen:
// ============================================
// Im Backend-Verzeichnis:
// wrangler d1 execute heizbox --local --file=migrations/0002_create_stash_tables.sql
// wrangler d1 execute heizbox --remote --file=migrations/0002_create_stash_tables.sql