import { Hono } from 'hono';
import type { HeatCycleRow } from '@heizbox/types';
import { groupSessions, calculateConsumption } from '../utils';
import type { Context } from 'hono';

const session = new Hono<{ Bindings: Env }>();

const handleGetSession = async (c: Context<{ Bindings: Env }>) => {
  try {
    const twoHoursAgo = Date.now() - (2 * 60 * 60 * 1000);

    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration, cycle FROM heat_cycles WHERE created_at >= ?1 ORDER BY created_at ASC"
    ).bind(twoHoursAgo).all<HeatCycleRow>();

    if (!results || results.length === 0) {
      return c.json({
        clicks: 0,
        lastClick: null,
        heat_cycles: [],
        totalConsumption: "0.00"
      });
    }

    const clicks = results.length;
    const lastClick = results[results.length - 1].created_at;
    const heat_cycles = groupSessions(results);
    const totalConsumption = calculateConsumption(clicks);

    return c.json({ clicks, lastClick, heat_cycles, totalConsumption });
  } catch (e: unknown) {
    console.error("Error in handleGetSession:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve session data", details: error.message }, 500);
  }
};

session.get('/', handleGetSession);

export default session;
