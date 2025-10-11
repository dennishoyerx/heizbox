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
      query = 'SELECT * FROM heat_cycles WHERE created_at >= ?';
      params = [startDate.getTime()];
      break;
    case 'week':
      startDate = new Date(now.getFullYear(), now.getMonth(), now.getDate() - now.getDay());
      query = 'SELECT * FROM heat_cycles WHERE created_at >= ?';
      params = [startDate.getTime()];
      break;
    case 'month':
      startDate = new Date(now.getFullYear(), now.getMonth(), 1);
      query = 'SELECT * FROM heat_cycles WHERE created_at >= ?';
      params = [startDate.getTime()];
      break;
    default:
      return c.json({ error: 'Invalid range specified. Use day, week, or month.' }, 400);
  }

  console.log(`Statistics route called with range: ${range}`);
  console.log(`Generated startDate: ${startDate.toISOString()} (${startDate.getTime()})`);
  console.log(`Query parameters: ${params}`);

  try {
    const { results } = await db.prepare(query).bind(...params).all();

    // Basic aggregation for now, more sophisticated aggregation can be added later
    const totalHeatCycles = results.length;
    const totalDuration = results.reduce((sum, heatCycle: any) => {
      const durationValue = Number(heatCycle.duration);
      return sum + (isNaN(durationValue) ? 0 : durationValue);
    }, 0);

    return c.json({ range, totalHeatCycles, totalDuration, heatCycles: results });
  } catch (error) {
    console.error('Error fetching statistics:', error);
    return c.json({ error: 'Failed to fetch statistics' }, 500);
  }
};
