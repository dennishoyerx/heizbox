import { Hono } from 'hono';

export const handleCreateRoute = (c: Hono.Context) => {
  const duration = c.req.query('duration');
  return c.json({ message: `Create route hit with duration: ${duration}` });
};
