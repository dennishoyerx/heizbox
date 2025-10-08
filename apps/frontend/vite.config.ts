/// <reference types='vitest' />
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import tailwindcss from '@tailwindcss/vite';

export default defineConfig(() => ({
  root: __dirname,
  cacheDir: '../../node_modules/.vite/apps/frontend',
  server: {
    port: 5173,
    host: '127.0.0.1',
    proxy: {
      '/api': 'http://127.0.0.1:8787'
    }
  },
  preview: {
    port: 4200,
    host: 'localhost',
  },
  plugins: [
    react(),
    tailwindcss(),
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
    '__STATIC_CONTENT_MANIFEST': JSON.stringify('{}')
  }
}));
