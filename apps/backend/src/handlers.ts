import type { SessionRow } from '@heizbox/types';
import { getBerlinTimeRange, groupSessions, calculateConsumption, getMimeType } from './utils';
import type { Context } from 'hono';

export const handleGetSessions = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { start, end } = getBerlinTimeRange();
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration, cycle FROM sessions WHERE created_at >= ?1 AND created_at < ?2 ORDER BY created_at ASC"
    ).bind(start, end).all<SessionRow>();

    if (!results) {
      return c.json({ sessions: [], totalConsumption: "0.00" });
    }

    const sessions = groupSessions(results);
    const totalConsumption = calculateConsumption(results.length);
    return c.json({ sessions, totalConsumption });
  } catch (e: unknown) {
    console.error("Error in handleGetSessions:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve sessions", details: error.message }, 500);
  }
};

export const handleGetJson = async (c: Context<{ Bindings: Env }>) => {
  try {
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration, cycle FROM sessions ORDER BY id DESC"
    ).all<SessionRow>();

    return c.json(results);
  } catch (e: unknown) {
    console.error("Error in handleGetJson:", e);
    const error = e as Error;
    return c.json({ err: "Failed to retrieve all sessions", details: error.message }, 500);
  }
};

export const handleCreateSession = async (c: Context<{ Bindings: Env }>) => {
  try {
    const durationStr = c.req.query("duration");
    const cycleStr = c.req.query("cycle");

    if (!durationStr || !cycleStr) {
      return c.text("Missing duration or cycle", 400);
    }

    const duration = parseFloat(durationStr);
    const cycle = parseInt(cycleStr, 10);

    if (isNaN(duration) || duration <= 0 || isNaN(cycle) || cycle <= 0) {
      return c.text("Invalid duration or cycle", 400);
    }

    // Debounce: Check for recent identical entries
    const thirtySecondsAgo = new Date(Date.now() - 30000).toISOString();
    const { count } = await c.env.db.prepare(
      "SELECT COUNT(*) as count FROM sessions WHERE duration = ?1 AND cycle = ?2 AND created_at > ?3"
    ).bind(duration, cycle, thirtySecondsAgo).first<{ count: number }>();

    if (count > 0) {
      return c.text("Duplicate within 30s, not inserted", 200);
    }

    const now = new Date().toISOString();
    await c.env.db.prepare(
      "INSERT INTO sessions (duration, cycle, created_at) VALUES (?1, ?2, ?3)"
    ).bind(duration, cycle, now).run();

    return c.text("OK");
  } catch (e: unknown) {
    console.error("Error in handleCreateSession:", e);
    const error = e as Error;
    return c.json({ err: "Failed to create session", details: error.message }, 500);
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
