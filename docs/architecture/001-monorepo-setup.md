# 001-monorepo-setup.md

## Status
Accepted

## Context
We need a unified development environment for our Heizbox project, encompassing a React frontend, a Cloudflare Worker backend, and ESP32 embedded code. A monorepo approach will help manage shared code, dependencies, and consistent tooling across these disparate parts.

## Decision
We will use NX with pnpm to establish a monorepo. This setup provides:
-   **Workspace Management:** pnpm workspaces for efficient dependency management.
-   **Task Runner:** NX for consistent build, test, and linting commands across projects.
-   **Code Sharing:** Dedicated `libs` directory for shared UI components, types, and utilities.
-   **Project Structure:** Clear separation of `apps` (frontend, backend, esp32) and `libs`.

## Consequences
-   **Pros:** Improved code sharing, consistent tooling, simplified dependency management, easier refactoring across projects.
-   **Cons:** Initial setup complexity, potential for larger `node_modules` (though mitigated by pnpm's hoisting), learning curve for NX.
