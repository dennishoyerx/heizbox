# Heizbox Project - README (English)

## Goal
A unified development environment for Frontend (React/Vite), Backend (Cloudflare Worker API), and Embedded (ESP32 PlatformIO).

## Architecture
- **Monorepo Base:** NX + pnpm
- **Frontend:** React 18 + TypeScript, Vite, Tailwind CSS, shadcn/ui, Framer Motion
- **Backend:** Cloudflare Worker, Hono, D1 Database
- **Embedded:** PlatformIO (ESP32, C++)
- **Shared Libraries:** UI Components (shadcn/ui), Type Definitions, Utility Functions

## Setup
1.  **Install Node.js and pnpm:** Ensure Node.js (LTS) and pnpm are installed.
2.  **Clone Repository:** `git clone [URL]`
3.  **Install Dependencies:** `pnpm install` in the root directory.
4.  **Install NX CLI globally:** `pnpm add -g nx`

## API Endpoints
- `/api/create?duration=xx`: Creates a new heating session.
- `/api/sessions`: Retrieves all heating sessions.

## Deployment
- **Frontend (Cloudflare Pages):** `pnpm nx build frontend` then `wrangler pages deploy dist` (or via GitHub Actions).
- **Backend (Cloudflare Workers):** `pnpm nx build backend` then `wrangler deploy` (or via GitHub Actions).

## Reference
- Heizbox System Description (Cloudflare, React, ESP32)
