import { Hono } from 'hono';

export const handleSessionsRoute = (c: Hono.Context) => {
  return c.text('Sessions route hit');
};
