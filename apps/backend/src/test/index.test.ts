
import { describe, it, expect, vi } from 'vitest';
import app from '../index';
import type { SessionRow } from '@heizbox/types';

describe('API Routes', () => {
  it('should return sessions from the database', async () => {
    const mockDb = {
      prepare: vi.fn(() => ({
        bind: vi.fn(() => ({
          all: vi.fn(() => Promise.resolve({
            results: [
              { id: '1', created_at: '2024-01-01T12:00:00Z', duration: 10, cycle: 1 },
              { id: '2', created_at: '2024-01-01T12:05:00Z', duration: 10, cycle: 2 },
            ] as SessionRow[],
          })),
        })),
      })),
    };

    const env = { 
      db: mockDb, 
      __STATIC_CONTENT: {
        get: vi.fn((key) => {
          if (key === '__STATIC_CONTENT_MANIFEST') {
            return Promise.resolve('{}');
          }
          return Promise.resolve(null);
        }),
      }, 
    } as any;

    const res = await app.request('/api/sessions', { method: 'GET' }, env);
    const json = await res.json();

    expect(res.status).toBe(200);
    expect(json.sessions).toHaveLength(1);
    expect(json.sessions[0]).toHaveLength(2);
    expect(json.totalConsumption).toBe('0.05');
  });

  it('should create a session', async () => {
    const mockDb = {
      prepare: vi.fn(() => ({
        bind: vi.fn(() => ({
          first: vi.fn(() => Promise.resolve({ count: 0 })),
          run: vi.fn(() => Promise.resolve()),
        })),
      })),
    };

    const env = { 
      db: mockDb, 
      __STATIC_CONTENT: {
        get: vi.fn((key) => {
          if (key === '__STATIC_CONTENT_MANIFEST') {
            return Promise.resolve('{}');
          }
          return Promise.resolve(null);
        }),
      }, 
    } as any;

    const res = await app.request('/api/create?duration=10&cycle=1', { method: 'GET' }, env);
    const text = await res.text();

    expect(res.status).toBe(200);
    expect(text).toBe('OK');
  });
});
