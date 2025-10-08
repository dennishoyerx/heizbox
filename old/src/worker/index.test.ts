import { describe, it, expect } from 'vitest';
import app from './index'; // Assuming your Hono app is exported from index.ts
import { groupSessions, calculateConsumption } from './utils';
import type { SessionRow } from '../types';

describe('Heizbox Worker', () => {
  describe('Utils', () => {
    it('should correctly group sessions with a gap of more than 1 hour', () => {
      const rows: SessionRow[] = [
        { id: 1, created_at: '2025-10-07T10:00:00.000Z', duration: '5.1' },
        { id: 2, created_at: '2025-10-07T10:05:00.000Z', duration: '4.9' },
        { id: 3, created_at: '2025-10-07T11:10:00.000Z', duration: '5.3' }, // > 1h gap
        { id: 4, created_at: '2025-10-07T11:15:00.000Z', duration: '5.0' },
      ];
      const grouped = groupSessions(rows);
      expect(grouped).toHaveLength(2);
      expect(grouped[0]).toHaveLength(2); // Reversed order
      expect(grouped[1]).toHaveLength(2);
      expect(grouped[0][0].id).toBe(3);
      expect(grouped[1][0].id).toBe(1);
    });

    it('should return an empty array if no rows are provided for grouping', () => {
      const grouped = groupSessions([]);
      expect(grouped).toEqual([]);
    });

    it('should calculate consumption correctly', () => {
      expect(calculateConsumption(0)).toBe('0.00');
      expect(calculateConsumption(1)).toBe('0.05');
      expect(calculateConsumption(2)).toBe('0.05');
      expect(calculateConsumption(3)).toBe('0.10');
      expect(calculateConsumption(4)).toBe('0.10');
    });
  });

  describe('API Routes', () => {
    const mockEnv = {
      db: {
        prepare: (query: string) => ({
          bind: () => ({
            all: async () => {
              if (query.includes('SELECT id, created_at, duration FROM sessions WHERE created_at >= ?1 AND created_at < ?2')) {
                return {
                  results: [
                    { id: 1, created_at: new Date().toISOString(), duration: '5.1' },
                  ],
                };
              }
              return { results: [] };
            },
            first: async () => {
               if (query.includes('SELECT COUNT(*) as count')) {
                return { count: 0 };
              }
              return null;
            },
            run: async () => ({ success: true }),
          }),
        }),
      },
      __STATIC_CONTENT: {
        get: async (key: string) => {
          if (key === '__STATIC_CONTENT_MANIFEST') {
            return JSON.stringify({ 'index.html': 'index.123.html' });
          }
          if (key === 'index.123.html') {
            return '<html><body>Mocked Content</body></html>';
          }
          return null;
        }
      }
    };

    it('GET /api/sessions should return grouped sessions and consumption', async () => {
      const res = await app.request('/api/sessions', {}, mockEnv);
      expect(res.status).toBe(200);
      const json = await res.json();
      expect(json).toHaveProperty('sessions');
      expect(json).toHaveProperty('totalConsumption');
      expect(json.sessions).toHaveLength(1);
      expect(json.totalConsumption).toBe('0.05');
    });

    it('GET /api/create should create a new session', async () => {
      const res = await app.request('/api/create?duration=10.5', {}, mockEnv);
      expect(res.status).toBe(200);
      const text = await res.text();
      expect(text).toBe('OK');
    });

    it('GET /api/create should return 400 for missing duration', async () => {
      const res = await app.request('/api/create', {}, mockEnv);
      expect(res.status).toBe(400);
    });

    it('GET / should serve static content', async () => {
      const res = await app.request('/', {}, mockEnv);
      expect(res.status).toBe(200);
      const text = await res.text();
      expect(text).toBe('<html><body>Mocked Content</body></html>');
    });
  });
});