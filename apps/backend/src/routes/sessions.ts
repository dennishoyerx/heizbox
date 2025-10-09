import { Hono } from 'hono';

export const handleSessionsRoute = (c: Hono.Context) => {
  return c.json({ message: 'Sessions route hit' });
};
