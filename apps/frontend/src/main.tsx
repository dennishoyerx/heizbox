import React from 'react';
import ReactDOM from 'react-dom/client';
import { BrowserRouter } from 'react-router-dom';
import { Theme } from '@radix-ui/themes';
import * as Sentry from "@sentry/react";
import App from './app/App';
import '@radix-ui/themes/styles.css';
import './styles.css';

Sentry.init({
    dsn: "https://b449f6b3711bcef239d2028c85f349ff@o4510082700345344.ingest.de.sentry.io/4510173450010704",
    sendDefaultPii: true
});

const rootElement = document.getElementById('root');
if (rootElement) {
  ReactDOM.createRoot(rootElement).render(
    <React.StrictMode>
      <BrowserRouter>
        <App />
      </BrowserRouter>
    </React.StrictMode>,
  );
}