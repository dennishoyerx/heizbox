/// <reference types='vitest' />
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import tailwindcss from '@tailwindcss/vite';
import { fileURLToPath, URL } from 'node:url';
import { sentryVitePlugin } from "@sentry/vite-plugin";

export default defineConfig(() => ({
  root: __dirname,
  cacheDir: '../../node_modules/.vite/apps/frontend',
  server: {
    port: 5173,
    host: '127.0.0.1',
    proxy: {
      '/api': process.env.VITE_PUBLIC_API_URL || 'http://127.0.0.1:8787'
    }
  },
  preview: {
    port: 4200,
    host: 'localhost',
  },
  resolve: {
    alias: {
      '@heizbox/utils': fileURLToPath(new URL('../../libs/utils/src/index.ts', import.meta.url)),
      '@heizbox/types': fileURLToPath(new URL('../../libs/types/src/index.ts', import.meta.url)),
    },
  },
  plugins: [
    react(),
    tailwindcss(),
    sentryVitePlugin({
      org: "dennis-hoyer",
      project: "heizbox-frontend",
      authToken: process.env.SENTRY_AUTH_TOKEN,
    }),
  ],
  // Uncomment this if you are using workers.
  // worker: {
  //  plugins: [ nxViteTsPaths() ],
  // },
  build: {
    outDir: './dist',
    emptyOutDir: true,
    reportCompressedSize: true,
    commonjsOptions: {
      transformMixedEsModules: true,
    },
  },
  define: {
    '__STATIC_CONTENT_MANIFEST': JSON.stringify('{}'),
    'import.meta.env.VITE_PUBLIC_API_URL': JSON.stringify(process.env.VITE_PUBLIC_API_URL || 'http://127.0.0.1:8787')
  }
}));
