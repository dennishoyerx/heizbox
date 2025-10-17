// File: apps/backend/src/routes/stash.ts
import { Hono } from 'hono';
import type { Context } from 'hono';
import { StashService } from '../services/stashService.js';
import type {
  CreateStashItemRequest,
  WithdrawStashItemRequest,
} from '@heizbox/types';

const stash = new Hono<{ Bindings: Env }>();

// GET /api/stash - Liste aller Items mit History
stash.get('/', async (c: Context<{ Bindings: Env }>) => {
  try {
    const service = new StashService(c.env.db);
    const items = await service.getStashItems();
    
    const total_current = items.reduce((sum, item) => sum + item.quantity_current, 0);
    const total_withdrawn = items.reduce((sum, item) => sum + item.total_withdrawn, 0);

    return c.json({
      items,
      total_current,
      total_withdrawn,
    });
  } catch (error) {
    console.error('Error in GET /api/stash:', error);
    const err = error as Error;
    return c.json({ error: 'Failed to retrieve stash items', details: err.message }, 500);
  }
});

// POST /api/stash - Neues Item anlegen
stash.post('/', async (c: Context<{ Bindings: Env }>) => {
  try {
    const body = await c.req.json<CreateStashItemRequest>();
    const { item_name, quantity_start, device_id, notes } = body;

    if (!item_name || typeof quantity_start !== 'number') {
      return c.json({ error: 'item_name and quantity_start are required' }, 400);
    }

    const service = new StashService(c.env.db);
    const item = await service.createStashItem(item_name, quantity_start, device_id, notes);

    // Benachrichtige Durable Object
    const durableObjectId = c.env.DEVICE_STATUS.idFromName(device_id || 'default');
    const stub = c.env.DEVICE_STATUS.get(durableObjectId);
    
    await stub.fetch(new Request('http://do/publish', {
      method: 'POST',
      body: JSON.stringify({
        type: 'stashUpdated',
        item,
      }),
    }));

    return c.json(item, 201);
  } catch (error) {
    console.error('Error in POST /api/stash:', error);
    const err = error as Error;
    return c.json({ error: 'Failed to create stash item', details: err.message }, 500);
  }
});

// PUT /api/stash/:id/withdraw - Entnahme erfassen
stash.put('/:id/withdraw', async (c: Context<{ Bindings: Env }>) => {
  try {
    const id = c.req.param('id');
    const body = await c.req.json<WithdrawStashItemRequest>();
    const { quantity, notes } = body;

    if (typeof quantity !== 'number' || quantity <= 0) {
      return c.json({ error: 'Valid quantity is required' }, 400);
    }

    const service = new StashService(c.env.db);
    const result = await service.withdrawFromStash(id, quantity, undefined, notes);

    // Benachrichtige Durable Object
    const durableObjectId = c.env.DEVICE_STATUS.idFromName('default');
    const stub = c.env.DEVICE_STATUS.get(durableObjectId);
    
    await stub.fetch(new Request('http://do/publish', {
      method: 'POST',
      body: JSON.stringify({
        type: 'stashUpdated',
        item: result.item,
        withdrawal: result.withdrawal,
      }),
    }));

    return c.json(result);
  } catch (error) {
    console.error('Error in PUT /api/stash/:id/withdraw:', error);
    const err = error as Error;
    return c.json({ error: 'Failed to withdraw from stash', details: err.message }, 500);
  }
});

// DELETE /api/stash/:id - Item löschen
stash.delete('/:id', async (c: Context<{ Bindings: Env }>) => {
  try {
    const id = c.req.param('id');
    const service = new StashService(c.env.db);
    await service.deleteStashItem(id);

    // Benachrichtige Durable Object
    const durableObjectId = c.env.DEVICE_STATUS.idFromName('default');
    const stub = c.env.DEVICE_STATUS.get(durableObjectId);
    
    await stub.fetch(new Request('http://do/publish', {
      method: 'POST',
      body: JSON.stringify({
        type: 'stashUpdated',
      }),
    }));

    return c.json({ success: true });
  } catch (error) {
    console.error('Error in DELETE /api/stash/:id:', error);
    const err = error as Error;
    return c.json({ error: 'Failed to delete stash item', details: err.message }, 500);
  }
});

// GET /api/stash/stats - Statistiken
stash.get('/stats', async (c: Context<{ Bindings: Env }>) => {
  try {
    const range = c.req.query('range') || '30d';
    const service = new StashService(c.env.db);
    const stats = await service.getStashStats(range);
    return c.json(stats);
  } catch (error) {
    console.error('Error in GET /api/stash/stats:', error);
    const err = error as Error;
    return c.json({ error: 'Failed to retrieve stash stats', details: err.message }, 500);
  }
});

// GET /api/stash/:id/withdrawals - Entnahme-Historie für ein Item
stash.get('/:id/withdrawals', async (c: Context<{ Bindings: Env }>) => {
  try {
    const id = c.req.param('id');
    const service = new StashService(c.env.db);
    const withdrawals = await service.getWithdrawalsByItem(id);
    return c.json(withdrawals);
  } catch (error) {
    console.error('Error in GET /api/stash/:id/withdrawals:', error);
    const err = error as Error;
    return c.json({ error: 'Failed to retrieve withdrawals', details: err.message }, 500);
  }
});

export default stash;

// Hinweis: DeviceStatus DO muss um stashUpdated-Handling erweitert werden
// In apps/backend/src/durable-objects/DeviceStatus.ts in processDeviceMessage():
// 
// } else if (message.type === 'stashUpdated') {
//   console.log('DeviceStatus: Processing stashUpdated message.', message);
//   this.publish(message); // Broadcast to all subscribers
// }