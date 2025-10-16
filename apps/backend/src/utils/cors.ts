import { config } from '../config/index.js';

export const isOriginAllowed = (origin: string): boolean => {
  return config.cors.allowedOrigins.some((allowed) => {
    if (allowed.startsWith(".")) {
      return (
        origin.endsWith(allowed.substring(1)) || origin === allowed.substring(1)
      );
    }
    return origin === `https://${allowed}` || origin === `http://${allowed}`;
  });
};
