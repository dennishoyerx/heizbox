# Heizbox Monorepo Setup

## 1. Repository Setup
- Monorepo-Basis: NX + pnpm
- Workspace-Struktur:
  /apps
    /frontend       → React + Vite + Tailwind + TypeScript
    /backend        → Cloudflare Worker + Hono + D1-Datenbank
    /esp32          → PlatformIO-Projekt (ESP32, C++)
  /libs
    /ui             → Shadcn-UI-Komponentenbibliothek
    /types          → Geteilte Typdefinitionen (TS)
    /utils          → Gemeinsame Hilfsfunktionen für Frontend/Backend
  /docs
    /architecture   → ADRs und technische Notizen
    /readme         → Projekt-Readmes in Deutsch und Englisch

- Package Manager: pnpm
- Task Runner: NX (mit integrierten Targets für build, dev, lint, test)
- Git Hooks: Husky + lint-staged
- Linter/Formatter: ESLint + Prettier (einheitlich für alle Projekte)
- CI/CD-kompatibel (z. B. GitHub Actions oder Cloudflare Deploy Hooks)

## 2. Frontend: Heizbox Web
- Framework: React 18 + TypeScript
- Build Tool: Vite
- Styling: Tailwind CSS + shadcn/ui + Framer Motion
- Architektur: Feature-Folder-Struktur mit Atoms/Molecules/Organisms
- API: Kommuniziert mit dem Cloudflare-Worker über /api/*
- Dev-Server-Port: 5173
- Scripts:
  - dev: pnpm nx serve frontend
  - build: pnpm nx build frontend
  - deploy: pnpm run deploy (führt Vite-Build + Wrangler Deploy aus)

## 3. Backend: Cloudflare Worker (API)
- Framework: Hono
- Datenbank: Cloudflare D1
- Struktur:
  /src/index.ts → Einstiegspunkt für Worker
  /src/routes/sessions.ts → GET /api/sessions
  /src/routes/create.ts → GET /api/create
  /src/lib/db.ts → D1-Abfragen und Hilfsfunktionen
- Wrangler-Konfiguration automatisch generieren (wrangler.toml)
- Environment-Variablen: D1_BINDING, CF_ACCOUNT_ID, CF_API_TOKEN
- Dev-Port: 8787
- Scripts:
  - dev: pnpm nx serve backend
  - build: pnpm nx build backend
  - deploy: wrangler deploy

## 4. Embedded: ESP32 PlatformIO
- Unter /apps/esp32 ein PlatformIO-Projekt erzeugen
- Board: ESP32-WROOM
- Framework: Arduino oder ESP-IDF
- Beispiel-Sketch:
  - sendet periodisch GET-Anfragen an /api/create?duration=xx
  - WLAN-Konfig in separater config.h
- Scripts:
  - build: pio run
  - upload: pio run --target upload
  - monitor: pio device monitor

## 5. Infrastruktur
- NX build graph zeigt Abhängigkeiten (libs/ui, libs/types etc.)
- pnpm workspaces auf apps/* und libs/*
- Optionale Erweiterung:
  - Cloudflare Pages für Frontend
  - D1-Migration-Skripte im Backend-Ordner
  - Storybook für UI-Komponenten (libs/ui)

## 6. Dokumentation
- Haupt-README.md in Deutsch und Englisch
- Inhalt:
  - Ziel, Architektur, Setup, API-Endpoints, Deployment
  - Referenz: Heizbox Systembeschreibung (Cloudflare, React, ESP32)
- ADR-Verzeichnis (/docs/architecture)
  - Beispiel: 001-monorepo-setup.md
  - Beispiel: 002-data-flow-cloudflare-to-frontend.md

## 7. Ergebnis
- Vollständig initialisiertes NX-Monorepo mit pnpm.
- Getrennte, aber integrierte Entwicklungsumgebungen für:
  - React Frontend (Vite)
  - Cloudflare Worker Backend (Hono)
  - ESP32 Embedded Code (PlatformIO)
- Einheitliche Developer Experience mit `pnpm dev` (NX parallel)
- Projekt startklar für Deployment und Teamarbeit.
