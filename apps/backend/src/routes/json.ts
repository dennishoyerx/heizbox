import { Hono } from 'hono';
import type { HeatCycleRow } from '@heizbox/types';
import type { Context } from 'hono';

const json = new Hono<{ Bindings: Env }>();

const handleGetJson = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration, cycle FROM heat_cycles ORDER BY id DESC"
    ).all<HeatCycleRow>();

    return c.json(results);
  } catch (e: unknown) {
    console.error("Error in handleGetJson:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve all heat cycles", details: error.message }, 500);
  }
};

json.get('/', handleGetJson);

export default json;
