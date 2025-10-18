import { config } from '../config/index.js';

// Pre-compile allowlist als Set für O(1) Lookup
const allowedOriginsSet = new Set(
  config.cors.allowedOrigins.flatMap(origin => {
    if (origin.startsWith('.')) {
      // Wildcard-Domains: Speichere nur Suffix
      return [origin.substring(1)];
    }
    // Exakte Domains: Beide Protokolle
    return [`https://${origin}`, `http://${origin}`];
  })
);

export const isOriginAllowed = (origin: string): boolean => {
  // Exakter Match
  if (allowedOriginsSet.has(origin)) {
    return true;
  }

  // Wildcard-Match (nur Subdomain-Suffixe)
  for (const allowed of config.cors.allowedOrigins) {
    if (allowed.startsWith('.')) {
      const suffix = allowed.substring(1);
      if (origin.endsWith(suffix) || origin === `https://${suffix}` || origin === `http://${suffix}`) {
        return true;
      }
    }
  }

  return false;
};
