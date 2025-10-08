# 002-data-flow-cloudflare-to-frontend.md

## Status
Proposed

## Context
The frontend (React) needs to fetch data from the backend (Cloudflare Worker API). We need a clear and efficient data flow strategy.

## Decision
-   The React frontend will make HTTP requests to the Cloudflare Worker API.
-   The API will expose RESTful endpoints (e.g., `/api/sessions`, `/api/create`).
-   Data will be exchanged using JSON format.
-   Shared TypeScript types (from `libs/types`) will be used for API request/response payloads to ensure type safety across frontend and backend.

## Consequences
-   **Pros:** Standardized communication, clear separation of concerns, type safety with shared types.
-   **Cons:** Requires careful API design, potential for boilerplate code on both ends for data fetching/handling.
