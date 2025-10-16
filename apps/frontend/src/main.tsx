import React from "react";
import ReactDOM from "react-dom/client";
import { BrowserRouter as Router } from "react-router-dom";
import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import * as Sentry from "@sentry/react";
import App from "./app/app";
import "./styles.css";
import "@radix-ui/themes/styles.css";
import { WebSocketProvider } from "./app/WebSocketContext";

Sentry.init({
  dsn: "https://b449f6b3711bcef239d2028c85f349ff@o4510082700345344.ingest.de.sentry.io/4510173450010704",
  sendDefaultPii: true,
});

const queryClient = new QueryClient();

const rootElement = document.getElementById("root");
if (rootElement) {
  ReactDOM.createRoot(rootElement).render(
    <React.StrictMode>
      <WebSocketProvider deviceId="my-esp32-device">
        <QueryClientProvider client={queryClient}>
          <Router>
            <App />
          </Router>
        </QueryClientProvider>
      </WebSocketProvider>
    </React.StrictMode>,
  );
}
