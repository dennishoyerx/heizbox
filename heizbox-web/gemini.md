# Projekt: Heizbox Web

Dieses Dokument beschreibt den Aufbau, die Ziele und die Verwendung der "Heizbox Web" Anwendung. Es dient als zentrale Anlaufstelle für Entwickler, um sich schnell in das Projekt einzuarbeiten.

## 1. Zielsetzung

Die Anwendung dient zur Visualisierung und Verwaltung von Daten aus dem "Heizbox"-System. Ihr Hauptzweck ist die Bereitstellung einer modernen Weboberfläche, die Heiz-Sessions aus einer Cloudflare D1-Datenbank anzeigt. Die Lösung ermöglicht es, den Verbrauch und die Nutzung der Heizbox einfach und übersichtlich zu überwachen.

## 2. Architektur & Aufbau

Die Anwendung ist als **Single Page Application (SPA)** mit einem API-Backend aufgebaut und nutzt vollständig die Cloudflare-Plattform.

- **Systemarchitektur:**
  - Das **Frontend** ist eine in sich geschlossene React-Anwendung, die im Browser des Benutzers ausgeführt wird.
  - Das **Backend** ist ein Cloudflare Worker, der als API-Gateway fungiert und die Geschäftslogik enthält.
  - Die **Datenbank** ist eine Cloudflare D1-Instanz, die direkt vom Worker angesprochen wird.
  - Statische Frontend-Assets werden über Cloudflare Pages (via KV-Binding `__STATIC_CONTENT`) ausgeliefert.

- **Technologie-Stack:**
  - **Frontend:**
    - **Framework:** React
    - **Build-Tool:** Vite
    - **Styling:** Tailwind CSS
  - **Backend (API):**
    - **Laufzeitumgebung:** Cloudflare Workers
    - **Routing:** Hono
  - **Datenbank:**
    - **Dienst:** Cloudflare D1 (SQLite-basiert)
  - **Infrastruktur & Deployment:**
    - **Plattform:** Cloudflare
    - **Tool:** Wrangler CLI

## 3. Features & Funktionalität

### Kernfeature 1: Anzeige von Heiz-Sessions

- **Ziel:** Übersichtliche Darstellung der Heiz-Aktivitäten des aktuellen Tages (definiert als 09:00 Uhr morgens bis 09:00 Uhr des Folgetages).
- **Ablauf:**
  1. Das Frontend ruft den Endpunkt `/api/sessions` auf.
  2. Das Backend fragt die D1-Datenbank nach allen Einträgen im relevanten Zeitraum ab.
  3. Klicks, die zeitlich weniger als eine Stunde auseinanderliegen, werden serverseitig zu einer "Session" gruppiert.
  4. Der Gesamtverbrauch wird basierend auf der Anzahl der Klicks berechnet.
  5. Die gruppierten Sessions und der Gesamtverbrauch werden als JSON an das Frontend gesendet.
- **Output:** Eine interaktive Liste, in der jede Session eine aufklappbare Karte ist, die die einzelnen Klicks mit Zeitstempel und Dauer anzeigt.

### Kernfeature 2: Erstellung eines neuen Heiz-Events

- **Ziel:** Ermöglicht der Heizbox-Hardware, einen neuen Heizvorgang zu protokollieren.
- **Ablauf:**
  1. Ein externes Gerät (die Heizbox) sendet eine `GET`-Anfrage an den Endpunkt `/api/create`.
  2. Die Dauer des Heizvorgangs wird als Query-Parameter `duration` übergeben.
  3. Das Backend fügt einen neuen Datensatz mit der Dauer und einem aktuellen Zeitstempel in die Datenbank ein.
  4. Um doppelte Einträge zu verhindern, wird ein neuer Eintrag ignoriert, wenn innerhalb der letzten 30 Sekunden bereits ein Eintrag mit der exakt gleichen Dauer erstellt wurde.
- **Output:** Eine einfache Text-Antwort (`OK` oder eine Fehlerbeschreibung).

## 4. API & Endpunkte (API)

Alle API-Routen sind unter dem Präfix `/api` verfügbar.

| Methode | Pfad                  | Beschreibung                                                                                             | Beispiel-Payload | Beispiel-Response                                                                                             |
| :------ | :-------------------- | :------------------------------------------------------------------------------------------------------- | :--------------- | :------------------------------------------------------------------------------------------------------------ |
| `GET`   | `/api/sessions`       | Ruft die gruppierten Sessions des aktuellen Tages (seit 09:00 Uhr) ab. Dient zur Anzeige im Frontend.      | -                | `{"sessions": [[{"id": 1, "created_at": "...", "duration": "52"}], ...], "totalConsumption": "0.10"}` |
| `GET`   | `/api/json`           | Ruft **alle** Sessions aus der Datenbank als ungefilterte JSON-Liste ab. Nützlich für Debugging.         | -                | `[{"id": 1, "created_at": "...", "duration": "52"}, ...]`                                                    |
| `GET`   | `/api/create`         | Erstellt einen neuen Heiz-Eintrag. Benötigt einen `duration` Query-Parameter.                            | `?duration=52`  | `OK` (Text-Response)                                                                                          |

### Beispiel-Requests

**Abrufen der Sessions für das Frontend:**
```bash
curl "https://heizbox.dh19.workers.dev/api/sessions"
```

**Erstellen eines neuen Heiz-Events:**
```bash
curl "https://heizbox.dh19.workers.dev/api/create?duration=52"
```

## 5. Beispiele / Use-Cases

### Use-Case 1: Ein Benutzer öffnet die Webseite

1.  Der Benutzer navigiert zur Anwendungs-URL.
2.  Die React-Anwendung wird geladen und führt einen `fetch`-Aufruf an `/api/sessions` aus.
3.  Der Server antwortet mit einem JSON-Objekt, das die gruppierten Sessions enthält:
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
4.  Das Frontend rendert zwei aufklappbare "Session"-Karten, die die jeweiligen Klicks enthalten.

### Use-Case 2: Die Heizbox wird betätigt

1.  Die Hardware der Heizbox misst eine Betätigungsdauer von 52 Sekunden.
2.  Sie sendet eine HTTP-Anfrage an den konfigurierten Endpunkt: `GET /api/create?duration=52`.
3.  Der Cloudflare Worker empfängt die Anfrage, validiert sie und fügt einen neuen Eintrag in die D1-Datenbank ein.
4.  Der Worker antwortet mit dem Status `200 OK` und dem Text `OK`.

## 6. Deployment & Umgebung

### Lokale Entwicklung

Für die lokale Entwicklung werden zwei Terminals benötigt, um Frontend und Backend parallel auszuführen.

1.  **Terminal 1: Frontend-Entwicklungsserver (Vite)**
    ```bash
    # Startet den Vite-Server, typischerweise auf http://localhost:5173
    # Leitet API-Anfragen (/api/*) an den lokalen Worker weiter (Port 8787)
    npm run dev:vite
    ```

2.  **Terminal 2: Backend-Worker (Wrangler)**
    ```bash
    # Startet den Worker lokal und simuliert die D1-Datenbank
    # Die API ist unter http://localhost:8787 verfügbar
    npm run dev:wrangler
    ```
    *Alternativ kann `npm run dev` genutzt werden, um beide Prozesse mit `concurrently` zu starten.*

### Datenbank-Synchronisation

Um die lokale D1-Datenbank mit dem Stand der Remote-Datenbank zu synchronisieren:
```bash
npm run db:sync
```

### Veröffentlichen (Deployment)

Das Deployment auf die Cloudflare-Plattform erfolgt mit einem einzigen Befehl:
```bash
# Führt `vite build` und `wrangler deploy` aus
npm run deploy
```

## 7. Testing & Debugging

- **Test-Strategie:**
  - Das Projekt verwendet `vitest` für Unit-Tests.
  - Tests können mit `npm run test` ausgeführt werden.
  - Der Fokus liegt auf der Validierung der Backend-Logik (z.B. Session-Gruppierung, Verbrauchs-Berechnung).
- **Debugging:**
  - **Lokal:** `console.log`-Ausgaben im Worker sind im `wrangler dev`-Terminal sichtbar. Das Frontend kann mit den Browser-Entwicklertools debuggt werden.
  - **Remote:** Logs des produktiven Workers können über das Cloudflare Dashboard unter `Workers & Pages -> Logs` eingesehen werden. Dies ist in `wrangler.toml` aktiviert.

## 8. Wartung & Erweiterbarkeit

- **Code-Style:** Das Projekt verwendet ESLint zur Sicherstellung eines konsistenten Code-Stils. Vor einem Commit sollte `npm run lint` ausgeführt werden, um den Code zu überprüfen.
- **Abhängigkeiten:** Updates sollten regelmäßig über `npm update` eingespielt und getestet werden.
- **Erweiterbarkeit:**
  - Neue API-Endpunkte sollten in `src/worker/index.ts` innerhalb der Hono-App definiert werden.
  - Neue Frontend-Komponenten gehören in den Ordner `src/react-app/`.
  - Typdefinitionen, die von Front- und Backend geteilt werden, liegen in `src/types/`.
- **Design-Entscheidungen:** Größere Änderungen an der Architektur oder die Einführung neuer Kerntechnologien sollten dokumentiert werden, idealerweise in Form eines Architecture Decision Records (ADR).