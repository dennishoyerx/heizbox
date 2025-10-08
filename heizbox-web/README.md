# Project: Heizbox Web

This document describes the structure, goals, and usage of the "Heizbox Web" application. It serves as a central point of reference for developers to quickly get acquainted with the project.

## 1. Objective

The application is used for visualizing and managing data from the "Heizbox" system. Its main purpose is to provide a modern web interface that displays heating sessions from a Cloudflare D1 database. The solution enables easy and clear monitoring of the consumption and usage of the Heizbox.

## 2. Architecture & Setup

The application is built as a **Single Page Application (SPA)** with an API backend and fully leverages the Cloudflare platform.

- **System Architecture:**
  - The **Frontend** is a self-contained React application executed in the user's browser.
  - The **Backend** is a Cloudflare Worker that acts as an API gateway and contains the business logic.
  - The **Database** is a Cloudflare D1 instance, directly accessed by the Worker.
  - Static Frontend assets are delivered via Cloudflare Pages (via KV-Binding `__STATIC_CONTENT`).

- **Technology Stack:**
  - **Frontend:**
    - **Framework:** React
    - **Build Tool:** Vite
    - **Styling:** Tailwind CSS
  - **Backend (API):**
    - **Runtime Environment:** Cloudflare Workers
    - **Routing:** Hono
  - **Database:**
    - **Service:** Cloudflare D1 (SQLite-based)
  - **Infrastructure & Deployment:**
    - **Platform:** Cloudflare
    - **Tool:** Wrangler CLI

## 3. Features & Functionality

### Core Feature 1: Display of Heating Sessions

- **Objective:** Clear display of the current day's heating activities (defined as 09:00 AM to 09:00 AM the following day).
- **Process:**
  1. The Frontend calls the `/api/sessions` endpoint.
  2. The Backend queries the D1 database for all entries within the relevant period.
  3. Clicks that are less than one hour apart are grouped server-side into a "Session".
  4. Total consumption is calculated based on the number of clicks.
  5. The grouped sessions and total consumption are sent as JSON to the Frontend.
- **Output:** An interactive list where each session is an expandable card showing individual clicks with timestamps and duration.

### Core Feature 2: Creation of a New Heating Event

- **Objective:** Allows the Heizbox hardware to log a new heating process.
- **Process:**
  1. An external device (the Heizbox) sends a `GET` request to the `/api/create` endpoint.
  2. The duration of the heating process is passed as a query parameter `duration`.
  3. The Backend inserts a new record with the duration and a current timestamp into the database.
  4. To prevent duplicate entries, a new entry is ignored if an entry with the exact same duration was created within the last 30 seconds.
- **Output:** A simple text response (`OK` or an error description).

## 4. API & Endpoints

All API routes are available under the `/api` prefix.

| Method | Path                  | Description                                                                                             | Example Payload | Example Response                                                                                             |
| :----- | :-------------------- | :------------------------------------------------------------------------------------------------------ | :-------------- | :----------------------------------------------------------------------------------------------------------- |
| `GET`  | `/api/sessions`       | Retrieves grouped sessions for the current day (since 09:00 AM). Used for display in the Frontend.      | -               | `{"sessions": [[{"id": 1, "created_at": "...", "duration": "52"}], ...], "totalConsumption": "0.10"}` |
| `GET`  | `/api/json`           | Retrieves **all** sessions from the database as an unfiltered JSON list. Useful for debugging.          | -               | `[{"id": 1, "created_at": "...", "duration": "52"}, ...]`                                                   |
| `GET`  | `/api/create`         | Creates a new heating entry. Requires a `duration` query parameter.                                     | `?duration=52`  | `OK` (Text Response)                                                                                         |

### Example Requests

**Retrieving Sessions for the Frontend:**
```bash
curl "https://heizbox.dh19.workers.dev/api/sessions"
```

**Creating a New Heating Event:**
```bash
curl "https://heizbox.dh19.workers.dev/api/create?duration=52"
```

## 5. Examples / Use-Cases

### Use-Case 1: A User Opens the Website

1.  The user navigates to the application URL.
2.  The React application loads and performs a `fetch` call to `/api/sessions`.
3.  The server responds with a JSON object containing the grouped sessions:
    ```json
    {
      "sessions": [
        [
          { "id": 102, "created_at": "2025-10-07T15:30:05.000Z", "duration": "5.1" },
          { "id": 103, "created_at": "2025-10-07T15:32:10.000Z", "duration": "4.9" }
        ],
        [
          { "id": 99, "created_at": "2025-10-07T11:05:00.000Z", "duration": "5.3" }
        ]
      ],
      "totalConsumption": "0.10"
    }
    ```
4.  The Frontend renders two expandable "Session" cards, each containing the respective clicks.

### Use-Case 2: The Heizbox is Activated

1.  The Heizbox hardware measures an activation duration of 52 seconds.
2.  It sends an HTTP request to the configured endpoint: `GET /api/create?duration=52`.
3.  The Cloudflare Worker receives the request, validates it, and inserts a new entry into the D1 database.
4.  The Worker responds with status `200 OK` and the text `OK`.

## 6. Deployment & Environment

### Local Development

For local development, two terminals are required to run the Frontend and Backend in parallel.

1.  **Terminal 1: Frontend Development Server (Vite)**
    ```bash
    # Starts the Vite server, typically on http://localhost:5173
    # Forwards API requests (/api/*) to the local Worker (Port 8787)
    npm run dev:vite
    ```

2.  **Terminal 2: Backend Worker (Wrangler)**
    ```bash
    # Starts the Worker locally and simulates the D1 database
    # The API is available at http://localhost:8787
    npm run dev:wrangler
    ```
    *Alternatively, `npm run dev` can be used to start both processes with `concurrently`.*

### Database Synchronization

To synchronize the local D1 database with the remote database:
```bash
npm run db:sync
```

### Publishing (Deployment)

Deployment to the Cloudflare platform is done with a single command:
```bash
# Executes `vite build` and `wrangler deploy`
npm run deploy
```

## 7. Testing & Debugging

- **Test Strategy:**
  - The project uses `vitest` for unit tests.
  - Tests can be run with `npm run test`.
  - The focus is on validating the Backend logic (e.g., session grouping, consumption calculation).
- **Debugging:**
  - **Local:** `console.log` outputs in the Worker are visible in the `wrangler dev` terminal. The Frontend can be debugged with browser developer tools.
  - **Remote:** Logs of the production Worker can be viewed via the Cloudflare Dashboard under `Workers & Pages -> Logs`. This is enabled in `wrangler.toml`.

## 8. Maintenance & Extensibility

- **Code Style:** The project uses ESLint to ensure consistent code style. Before committing, `npm run lint` should be run to check the code.
- **Dependencies:** Updates should be regularly applied and tested via `npm update`.
- **Extensibility:**
  - New API endpoints should be defined in `src/worker/index.ts` within the Hono app.
  - New Frontend components belong in the `src/react-app/` folder.
  - Type definitions shared by Frontend and Backend are located in `src/types/`.
  - **Design Decisions:** Major changes to the architecture or the introduction of new core technologies should be documented, ideally in the form of an Architecture Decision Record (ADR).