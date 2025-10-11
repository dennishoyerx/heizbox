import type { HeatCycleRow } from '@heizbox/types';
import { getBerlinTimeRange, groupSessions, calculateConsumption, getMimeType } from './utils';
import type { Context } from 'hono';
import { createHeatCycle } from './lib/session';

export const handleGetHeatCycles = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { start, end } = getBerlinTimeRange();
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration, cycle FROM heat_cycles WHERE created_at >= ?1 AND created_at < ?2 ORDER BY created_at ASC"
    ).bind(start, end).all<HeatCycleRow>();

    if (!results) {
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

export const handleGetJson = async (c: Context<{ Bindings: Env }>) => {
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

export const handleCreateHeatCycle = async (c: Context<{ Bindings: Env }>) => {
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

export const handleStaticAssets = async (c: Context<{ Bindings: Env }>) => {
  try {
    const url = new URL(c.req.url);
    const path = url.pathname === '/' ? 'index.html' : url.pathname.substring(1);

    const manifestString = await c.env.__STATIC_CONTENT.get('__STATIC_CONTENT_MANIFEST');
    if (!manifestString) {
      console.error("__STATIC_CONTENT_MANIFEST not found in KV");
      return c.text("Internal Server Error: Manifest not found", 500);
    }

    const manifest = JSON.parse(manifestString);
    const assetKey = manifest[path] || path;

    const asset = await c.env.__STATIC_CONTENT.get(assetKey, { type: 'arrayBuffer' });

    if (!asset) {
      return c.notFound();
    }

    const mimeType = getMimeType(path);
    return new Response(asset, {
      headers: {
        'Content-Type': mimeType,
        'Cache-Control': 'public, max-age=31536000', // Cache for 1 year
      },
    });
  } catch (e: unknown) {
    console.error("Error in handleStaticAssets:", e);
    return c.text("Internal Server Error", 500);
  }
};

export const handleGetAllHeatCycles = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { results } = await c.env.db.prepare("SELECT id, created_at, duration, cycle FROM heat_cycles ORDER BY created_at DESC").all<HeatCycleRow>();
    return c.json(results);
  } catch (e: unknown) {
    console.error("Error in handleGetAllHeatCycles:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve all heat cycles", details: error.message }, 500);
  }
};