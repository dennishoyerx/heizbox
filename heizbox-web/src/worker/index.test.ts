
import { describe, it, expect, vi } from 'vitest';
import app from './index';

// Mock functions from the main file
const formatDateForDB = (date: Date): string => {
  return date.toISOString();
};

const formatDateForDisplay = (date: Date): string => {
  const berlinDate = new Date(date.toLocaleString("sv-SE", { timeZone: "Europe/Berlin" }));
  return `${String(berlinDate.getDate()).padStart(2, '0')}.${String(berlinDate.getMonth() + 1).padStart(2, '0')} ${String(berlinDate.getHours()).padStart(2, '0')}:${String(berlinDate.getMinutes()).padStart(2, '0')}`;
};

describe('Date Formatting', () => {
  it('should format date for database', () => {
    const date = new Date('2024-01-01T12:00:00Z');
    expect(formatDateForDB(date)).toBe('2024-01-01T12:00:00.000Z');
  });

  it('should format date for display', () => {
    const date = new Date('2024-01-01T12:00:00Z');
    // Note: This test might be brittle due to timezone differences in the test environment.
    // It assumes the environment can handle the "Europe/Berlin" timezone correctly.
    expect(formatDateForDisplay(date)).toBe('01.01 13:00');
  });
});

describe('API Routes', () => {
  it('should return sessions from the database', async () => {
    const mockDb = {
      prepare: vi.fn(() => ({
        bind: vi.fn(() => ({
          all: vi.fn(() => Promise.resolve({
            results: [
              { id: 1, created_at: '2024-01-01T12:00:00Z', duration: 10 },
              { id: 2, created_at: '2024-01-01T12:05:00Z', duration: 10 },
            ],
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
      __STATIC_CONTENT_MANIFEST: '{}' 
    };
    const res = await app.request('/api/sessions', { method: 'GET' }, env);
    const json = await res.json();

    expect(json.sessions).toHaveLength(1);
    expect(json.sessions[0]).toHaveLength(2);
    expect(json.totalConsumption).toBe('0.05');
  });

  it('should create a session', async () => {
    const mockDb = {
      prepare: vi.fn(() => ({
        bind: vi.fn(() => ({
          all: vi.fn(() => Promise.resolve({
            results: [],
          })),
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
      __STATIC_CONTENT_MANIFEST: '{}' 
    };
    const res = await app.request('/api/create?duration=10', { method: 'GET' }, env);
    const text = await res.text();

    expect(text).toBe('OK');
  });
});
