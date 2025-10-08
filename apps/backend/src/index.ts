import { Hono } from 'hono';
import { handleGetSessions, handleGetJson, handleCreateSession, handleStaticAssets } from './handlers';
import { handleWebSocket } from './handlers/websocket';

const app = new Hono<{ Bindings: Env }>();

// API Routes
app.get('/api/sessions', handleGetSessions);
app.get('/api/json', handleGetJson);
app.get('/api/create', handleCreateSession);
app.get('/api/add', handleCreateSession); // Legacy endpoint
app.get('/ws/status', handleWebSocket);

// Static asset handler
app.get('/*', handleStaticAssets);

export default app;
