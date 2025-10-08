import { Hono } from 'hono'
import type { SessionRow } from '../types';

// --- Type definitions ---
type Env = { db: D1Database; __STATIC_CONTENT: KVNamespace; __STATIC_CONTENT_MANIFEST: string; };

// --- Hono App Setup ---
const app = new Hono<{ Bindings: Env }>();

// --- Helper Functions ---

// --- Helper Functions ---
const formatDateForDB = (date: Date): string => {
  return date.toISOString();
};

const formatDateForDisplay = (date: Date): string => {
  const berlinDate = new Date(date.toLocaleString("sv-SE", { timeZone: "Europe/Berlin" }));
  return `${String(berlinDate.getDate()).padStart(2, '0')}.${String(berlinDate.getMonth() + 1).padStart(2, '0')} ${String(berlinDate.getHours()).padStart(2, '0')}:${String(berlinDate.getMinutes()).padStart(2, '0')}`;
};

const getBerlinTimeRange = () => {
  const now = new Date();

  // Calculate current time in Berlin timezone
  const nowBerlin = new Date(now.toLocaleString("en-US", { timeZone: "Europe/Berlin" }));

  const startBerlin = new Date(nowBerlin);
  startBerlin.setHours(9, 0, 0, 0);

  const endBerlin = new Date(startBerlin);
  endBerlin.setDate(startBerlin.getDate() + 1);

  // Convert Berlin times to UTC for comparison and database storage
  const startUtc = new Date(startBerlin.toLocaleString("en-US", { timeZone: "UTC" }));
  const endUtc = new Date(endBerlin.toLocaleString("en-US", { timeZone: "UTC" }));

  // Adjust for the 09:00 reset rule
  if (nowBerlin.getHours() < 9) {
    startUtc.setDate(startUtc.getDate() - 1);
    endUtc.setDate(endUtc.getDate() - 1);
  }

  return {
    start: formatDateForDB(startUtc),
    end: formatDateForDB(endUtc),
  };
};

const groupSessions = (rows: SessionRow[]): SessionRow[][] => {
  const sessions: SessionRow[][] = [];
  let currentGroup: SessionRow[] = [];

  for (const row of rows) {
    const date = new Date(row.created_at);

    if (currentGroup.length === 0) {
      currentGroup.push(row);
    } else {
      const lastDate = new Date(currentGroup[currentGroup.length - 1].created_at);
      if (date.getTime() - lastDate.getTime() >= 3600000) { // 1 hour gap
        sessions.push(currentGroup);
        currentGroup = [row];
      } else {
        currentGroup.push(row);
      }
    }
  }

  if (currentGroup.length) sessions.push(currentGroup);
  return sessions.reverse();
};

const calculateConsumption = (count: number): string => (0.05 * Math.ceil(count / 2)).toFixed(2);

app.get('/api/sessions', async (c) => {
  try {
    const { start, end } = getBerlinTimeRange();
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration FROM sessions WHERE created_at >= ?1 AND created_at < ?2 ORDER BY created_at ASC"
    ).bind(start, end).all<SessionRow>();

    if (!results) {
      return c.json({ sessions: [], totalConsumption: "0.00" });
    }

    const sessions = groupSessions(results);
    const totalConsumption = calculateConsumption(results.length);
    return c.json({ sessions, totalConsumption });
  } catch (e: any) {
    return c.json({ err: e.message, name: e.name, cause: e.cause }, 500);
  }
});

app.get('/api/json', async (c) => {
  try {
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at, duration FROM sessions ORDER BY id DESC"
    ).all<SessionRow>();

    return c.json(results, {
      headers: { "Content-Type": "application/json" },
    });
  } catch (e: any) {
    return c.json({ err: e.message, name: e.name, cause: e.cause }, 500);
  }
});

app.get('/api/create', async (c) => {
  console.log('Received /api/create request:', c.req.url, 'Duration:', c.req.query("duration"));
  try {
    const durationStr = c.req.query("duration");
    if (!durationStr) return c.text("Missing duration", 400);

    const duration = parseFloat(durationStr);
    if (isNaN(duration)) {
      return c.text("Invalid duration", 400);
    }

    const now = new Date().toISOString();
    const nowMs = Date.now();

    // Check for duplicates within 30 seconds
    const { results } = await c.env.db.prepare(
      "SELECT id, created_at FROM sessions WHERE duration = ?1 ORDER BY created_at DESC LIMIT 1"
    ).bind(duration).all<SessionRow>();

    if (results.length > 0) {
      const lastTime = new Date(results[0].created_at).getTime();
      if (nowMs - lastTime < 30000) {
        return c.text("Duplicate within 30s, not inserted", 200);
      }
    }

    // Insert new session
    await c.env.db.prepare(
      "INSERT INTO sessions (duration, created_at) VALUES (?1, ?2)"
    ).bind(duration, now).run();

    return c.text("OK");
  } catch (e: any) {
    return c.json({ err: e.message, name: e.name, cause: e.cause }, 500);
  }
});

// Custom static asset handler for all paths (/*)
app.get('/*', async (c) => {
  try {
    const url = new URL(c.req.url);
    let path = url.pathname;

    if (path === '/') {
      path = 'index.html';
    } else {
      path = path.substring(1); // Remove leading slash
    }

    const manifestString = await c.env.__STATIC_CONTENT.get('__STATIC_CONTENT_MANIFEST');
    if (!manifestString) {
      return c.json({ err: '__STATIC_CONTENT_MANIFEST not found in KV', name: 'NotFoundError' }, 500);
    }
    const manifest = JSON.parse(manifestString);
    const assetPath = manifest[path] || path; // Use hashed path from manifest if available, otherwise original path

    const asset = await c.env.__STATIC_CONTENT.get(assetPath, { type: 'arrayBuffer' });

    if (asset) {
      const mimeType = getMimeType(assetPath);
      return new Response(asset, { headers: { 'Content-Type': mimeType } });
    }

    return c.notFound();
  } catch (e: any) {
    return c.json({ err: e.message, name: e.name, cause: e.cause }, 500);
  }
});

// Helper function to determine MIME type
const getMimeType = (path: string): string => {
  if (path.endsWith('.html')) return 'text/html';
  if (path.endsWith('.js')) return 'application/javascript';
  if (path.endsWith('.css')) return 'text/css';
  if (path.endsWith('.svg')) return 'image/svg+xml';
  if (path.endsWith('.png')) return 'image/png';
  if (path.endsWith('.jpg') || path.endsWith('.jpeg')) return 'image/jpeg';
  if (path.endsWith('.json')) return 'application/json';
  return 'application/octet-stream';
};


export default app
