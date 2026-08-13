import { config } from '../config/index.js';

export const isOriginAllowed = (origin: string): boolean => {
  // Kein Origin-Header (curl, ESP32, nicht-Browser-Clients) -> immer erlauben
  if (!origin) return true;
  const allowedOrigins = config.cors.allowedOrigins;
  let originUrl: URL;
  try {
    originUrl = new URL(origin);
  } catch {
    return false;
  }

  for (const allowedOrigin of allowedOrigins) {
    if (allowedOrigin.startsWith('.')) {
      // Wildcard: e.g., .hzbx.de
      const domain = allowedOrigin.substring(1);
      if (originUrl.hostname === domain || originUrl.hostname.endsWith('.' + domain)) {
        return true;
      }
    } else if (allowedOrigin.includes(':')) {
      // Exact match with port: e.g., localhost:5173
      if (originUrl.host === allowedOrigin) {
        return true;
      }
    } else {
      // Exact match without port: e.g., hzbx.de
      if (originUrl.hostname === allowedOrigin) {
        return true;
      }
    }
  }

  return false;
};
