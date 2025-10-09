import { Hono } from 'hono';
import { getDb, insertSession } from '../lib/db';

export const handleCreateRoute = async (c: Hono.Context) => {
  const duration = c.req.query('duration');
  if (!duration) {
    return c.json({ error: 'Duration is required' }, 400);
  }

  const parsedDuration = parseInt(duration, 10);
  if (isNaN(parsedDuration)) {
    return c.json({ error: 'Duration must be a valid number' }, 400);
  }

  const db = getDb(c.env);
  const created_at = new Date().toISOString();

  try {
    await insertSession(db, created_at, parsedDuration);
    return c.json({ message: `Create route hit with duration: ${duration}. Session saved.` });
  } catch (error) {
    console.error('Error inserting session:', error);
    return c.json({ error: 'Failed to save session' }, 500);
  }
};
