import { Hono } from 'hono';
import { handleWebSocket } from '../handlers/websocket';

const websocket = new Hono<{ Bindings: Env }>();

websocket.get('/status', handleWebSocket);

export default websocket;
