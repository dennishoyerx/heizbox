import { generateUuid } from '../utils';

export const getDb = (env: Env) => {
  return env.db;
};

interface Env {
  db: D1Database;
}
