import type { Context } from 'hono';

interface WebSocketMessage {
  topic: string;
  data: any;
}

// In-memory storage for connected WebSocket clients
// In a real-world scenario with multiple Workers, this would need a distributed store (e.g., Durable Objects).
const connectedClients = new Set<WebSocket>();

export const handleWebSocket = async (c: Context) => {
  // Check if the request is a WebSocket upgrade
  const upgradeHeader = c.req.header('Upgrade');
  if (upgradeHeader !== 'websocket') {
    return c.text('Expected WebSocket upgrade', 426);
  }

  const webSocketPair = new WebSocketPair();
  const [client, server] = Object.values(webSocketPair);

  server.accept();

  connectedClients.add(server);

  server.addEventListener('message', async (event) => {
    try {
      const message: WebSocketMessage = JSON.parse(event.data as string);
      console.log('Received WebSocket message:', message);

      // Assuming ESP32 sends status updates to 'heizbox/status'
      if (message.topic === 'heizbox/status') {
        // Broadcast to all connected clients
        connectedClients.forEach((ws) => {
          if (ws !== server) { // Don't send back to the sender
            ws.send(JSON.stringify(message));
          }
        });
      }
    } catch (error) {
      console.error('Error processing WebSocket message:', error);
      server.send(JSON.stringify({ error: 'Invalid message format' }));
    }
  });

  server.addEventListener('close', () => {
    console.log('WebSocket closed');
    connectedClients.delete(server);
  });

  server.addEventListener('error', (error) => {
    console.error('WebSocket error:', error);
    connectedClients.delete(server);
  });

  return new Response(null, { status: 101, webSocket: client });
};
