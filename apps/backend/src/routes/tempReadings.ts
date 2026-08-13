import { Hono } from "hono";

const tempReadings = new Hono<{ Bindings: Env }>();

// POST /api/temp_readings  { temp_raw, temp_calibrated, is_heating, deviceId? }
tempReadings.post("/", async (c) => {
  const body = await c.req.json().catch(() => null);
  if (!body) return c.text("Invalid JSON", 400);

  const { temp_raw, temp_calibrated, is_heating, deviceId } = body as {
    temp_raw?: number;
    temp_calibrated?: number;
    is_heating?: boolean;
    deviceId?: string;
  };

  try {
    await c.env.db
      .prepare(
        "INSERT INTO temp_readings (device_id, timestamp, temp_raw, temp_calibrated, is_heating) VALUES (?, ?, ?, ?, ?)"
      )
      .bind(
        deviceId || "HeizboxESP32",
        Date.now(),
        typeof temp_raw === "number" ? temp_raw : null,
        typeof temp_calibrated === "number" ? temp_calibrated : null,
        is_heating ? 1 : 0
      )
      .run();
    return c.json({ success: true });
  } catch (e: any) {
    console.error("temp_readings insert error:", e);
    return c.json({ error: "Insert failed", details: e.message }, 500);
  }
});

// GET /api/temp_readings?limit=100&deviceId=
tempReadings.get("/", async (c) => {
  const limit = Math.min(parseInt(c.req.query("limit") || "100", 10), 1000);
  const deviceId = c.req.query("deviceId");

  try {
    let sql = "SELECT id, device_id, timestamp, temp_raw, temp_calibrated, is_heating FROM temp_readings";
    const args: unknown[] = [];
    if (deviceId) {
      sql += " WHERE device_id = ?";
      args.push(deviceId);
    }
    sql += " ORDER BY timestamp DESC LIMIT ?";
    args.push(limit);

    const { results } = await c.env.db.prepare(sql).bind(...args).all();
    return c.json(results);
  } catch (e: any) {
    console.error("temp_readings select error:", e);
    return c.json({ error: "Query failed", details: e.message }, 500);
  }
});

export default tempReadings;
