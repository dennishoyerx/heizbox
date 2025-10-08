# Heizbox Projekt - README (Deutsch)

## Ziel
Eine einheitliche Entwicklungsumgebung für Frontend (React/Vite), Backend (Cloudflare Worker API) und Embedded (ESP32 PlatformIO).

## Architektur
- **Monorepo-Basis:** NX + pnpm
- **Frontend:** React 18 + TypeScript, Vite, Tailwind CSS, shadcn/ui, Framer Motion
- **Backend:** Cloudflare Worker, Hono, D1-Datenbank
- **Embedded:** PlatformIO (ESP32, C++)
- **Geteilte Bibliotheken:** UI-Komponenten (shadcn/ui), Typdefinitionen, Hilfsfunktionen

## Setup
1.  **Node.js und pnpm installieren:** Stellen Sie sicher, dass Node.js (LTS) und pnpm installiert sind.
2.  **Repository klonen:** `git clone [URL]`
3.  **Abhängigkeiten installieren:** `pnpm install` im Root-Verzeichnis.
4.  **NX CLI global installieren:** `pnpm add -g nx`

## API Endpoints
- `/api/create?duration=xx`: Erstellt eine neue Heizsitzung.
- `/api/sessions`: Ruft alle Heizsitzungen ab.

## Deployment
- **Frontend (Cloudflare Pages):** `pnpm nx build frontend` und dann `wrangler pages deploy dist` (oder über GitHub Actions).
- **Backend (Cloudflare Workers):** `pnpm nx build backend` und dann `wrangler deploy` (oder über GitHub Actions).

## Referenz
- Heizbox Systembeschreibung (Cloudflare, React, ESP32)
