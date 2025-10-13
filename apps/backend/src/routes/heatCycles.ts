import { Hono } from 'hono';
import type { HeatCycleRow } from '@heizbox/types';
import { getBerlinTimeRange, groupSessions, calculateConsumption } from '../utils';
import type { Context } from 'hono';
import { createHeatCycle } from '../lib/heatCycleService';

const heatCycles = new Hono<{ Bindings: Env }>();

const handleGetHeatCycles = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { start, end } = getBerlinTimeRange();
    console.log(`Querying heat_cycles from ${new Date(start).toISOString()} (${start}) to ${new Date(end).toISOString()} (${end})`);
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration, cycle FROM heat_cycles WHERE created_at >= ?1 AND created_at < ?2 ORDER BY created_at ASC"
    ).bind(start, end).all<HeatCycleRow>();

    if (!results || results.length === 0) {
      return c.json({ heatCycles: [], totalConsumption: "0.00" });
    }

    const heatCycles = groupSessions(results);
    const totalConsumption = calculateConsumption(results.length);
    return c.json({ heatCycles, totalConsumption });
  } catch (e: unknown) {
    console.error("Error in handleGetHeatCycles:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve heat cycles", details: error.message }, 500);
  }
};

const handleCreateHeatCycle = async (c: Context<{ Bindings: Env }>) => {
  try {
    const durationStr = c.req.query("duration");
    const cycleStr = c.req.query("cycle");

    if (!durationStr) {
      return c.text("Missing duration", 400);
    }

    const duration = parseFloat(durationStr);
    const cycle = cycleStr ? parseInt(cycleStr, 10) : 1; // Default to 1 if not provided

    const success = await createHeatCycle(c.env.db, duration, cycle);

    if (success) {
      return c.text("OK");
    } else {
      return c.text("Failed to create heat cycle", 500);
    }
  } catch (e: unknown) {
    console.error("Error in handleCreateHeatCycle:", e);
    const error = e as Error;
    return c.json({ err: "Failed to create heat cycle", details: error.message }, 500);
  }
};

const handleGetAllHeatCycles = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { results } = await c.env.db.prepare("SELECT id, created_at, duration, cycle FROM heat_cycles ORDER BY created_at DESC").all<HeatCycleRow>();
    return c.json(results);
  } catch (e: unknown) {
    console.error("Error in handleGetAllHeatCycles:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve all heat cycles", details: error.message }, 500);
  }
};

heatCycles.get('/', handleGetHeatCycles);
heatCycles.get('/create', handleCreateHeatCycle);
heatCycles.get('/add', handleCreateHeatCycle); // Legacy endpoint
heatCycles.get('/all_heat_cycles', handleGetAllHeatCycles);

export default heatCycles;
