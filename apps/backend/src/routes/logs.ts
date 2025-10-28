import { Hono } from 'hono';
import { HTTPException } from 'hono/http-exception';

const logs = new Hono();

interface LogEntry {
  timestamp: number;
  log_type: 'info' | 'error';
  message: string;
}

logs.get('/', async (c) => {
  const limit = c.req.query('limit') ? parseInt(c.req.query('limit') as string, 10) : 10;

  try {
    const { results } = await c.env.db.prepare(
      'SELECT timestamp, log_type, message FROM logs ORDER BY timestamp DESC LIMIT ?'
    ).bind(limit).all<LogEntry>();

    return c.json(results.map(log => ({ ...log, timestamp: log.timestamp * 1000 })));
  } catch (e: any) {
    console.error({ message: e.message, cause: e.cause });
    throw new HTTPException(500, { message: 'Failed to fetch logs' });
  }
});

export default logs;
