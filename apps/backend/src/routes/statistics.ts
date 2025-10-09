import { Hono } from 'hono';
import { getDb } from '../lib/db';

export const handleStatisticsRoute = async (c: Hono.Context) => {
  const db = getDb(c.env);
  const range = c.req.query('range') || 'month'; // Default to month

  let query = '';
  let params: (string | number)[] = [];

  const now = new Date();
  let startDate: Date;

  switch (range) {
    case 'day':
      startDate = new Date(now.getFullYear(), now.getMonth(), now.getDate());
      query = 'SELECT * FROM sessions WHERE created_at >= ?';
      params = [startDate.toISOString()];
      break;
    case 'week':
      startDate = new Date(now.getFullYear(), now.getMonth(), now.getDate() - now.getDay());
      query = 'SELECT * FROM sessions WHERE created_at >= ?';
      params = [startDate.toISOString()];
      break;
    case 'month':
      startDate = new Date(now.getFullYear(), now.getMonth(), 1);
      query = 'SELECT * FROM sessions WHERE created_at >= ?';
      params = [startDate.toISOString()];
      break;
    default:
      return c.json({ error: 'Invalid range specified. Use day, week, or month.' }, 400);
  }

  try {
    const { results } = await db.prepare(query).bind(...params).all();

    // Basic aggregation for now, more sophisticated aggregation can be added later
    console.log('Statistics query results:', results);
    const totalSessions = results.length;
    const totalDuration = results.reduce((sum, session: any) => {
      const durationValue = Number(session.duration);
      return sum + (isNaN(durationValue) ? 0 : durationValue);
    }, 0);

    console.log('Calculated totalDuration:', totalDuration);
    return c.json({ range, totalSessions, totalDuration, sessions: results });
  } catch (error) {
    console.error('Error fetching statistics:', error);
    return c.json({ error: 'Failed to fetch statistics' }, 500);
  }
};
