import { Hono } from 'hono';
import { HTTPException } from 'hono/http-exception';

const stashWithdrawalsRoute = new Hono<{ Bindings: Env }>();

stashWithdrawalsRoute.get('/recent', async (c) => {
  try {
    const { results } = await c.env.db.prepare(
      `
      SELECT
        si.name,
        sw.amount,
        sw.withdrawn_at
      FROM stash_withdrawals sw
      JOIN stash_items si ON sw.stash_item_id = si.id
      WHERE sw.withdrawn_at >= strftime('%Y-%m-%d %H:%M:%S', 'now', '-30 days')
      ORDER BY sw.withdrawn_at DESC;
      `
    ).all();
    return c.json(results);
  } catch (error) {
    console.error('Error fetching recent withdrawals:', error);
    throw new HTTPException(500, { message: 'Internal server error' });
  }
});

export default stashWithdrawalsRoute;
