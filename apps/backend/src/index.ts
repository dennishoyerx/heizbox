import { Hono } from 'hono';
import { handleGetSessions, handleGetJson, handleCreateSession, handleStaticAssets } from './handlers';

const app = new Hono<{ Bindings: Env }>();

// API Routes
app.get('/api/sessions', handleGetSessions);
app.get('/api/json', handleGetJson);
app.get('/api/create', handleCreateSession);
app.get('/api/add', handleCreateSession); // Legacy endpoint

// Static asset handler
app.get('/*', handleStaticAssets);

export default app;
