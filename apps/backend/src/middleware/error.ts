import * as Sentry from "@sentry/cloudflare";
import { HTTPException } from "hono/http-exception";
import { ValidationError } from "../utils/validation.js";
import type { Context } from "hono";

export const errorHandler = (err: Error, c: Context) => {
  Sentry.captureException(err);

  if (err instanceof ValidationError) {
    return c.json({ error: err.message }, 400);
  }

  if (err instanceof HTTPException) {
    return err.getResponse();
  }

  console.error("Unhandled error:", err);
  return c.json({ error: "Internal server error" }, 500);
};
