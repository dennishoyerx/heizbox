import { Hono } from 'hono'
import type { Context } from 'hono'
import { SessionService } from '../services/sessionService.js';

const session = new Hono<{ Bindings: Env }>()

const handleGetSession = async (c: Context<{ Bindings: Env }>) => {
	try {
		const service = new SessionService(c.env.db);
		const sessionData = await service.getCurrentSessionData();
		return c.json(sessionData);
	} catch (error) {
		console.error('Error in handleGetSession:', error)
		const err = error as Error
		return c.json({ error: 'Failed to retrieve session data', details: err.message }, 500)
	}
}

session.get('/', handleGetSession)

export default session
