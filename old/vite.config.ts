import path from 'path'
import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import tsconfigPaths from 'vite-tsconfig-paths'

import tailwindcss from '@tailwindcss/vite'

// https://vitejs.dev/config/
export default defineConfig({
  root: 'src/react-app',
  build: {
    outDir: '../dist',
  },
  plugins: [
    react(),
    tailwindcss({ config: './src/react-app/tailwind.config.cjs' }),
    tsconfigPaths(),
  ],
  server: {
    host: '127.0.0.1',
    proxy: {
      '/api': 'http://127.0.0.1:8787'
    }
  },
  define: {
     '__STATIC_CONTENT_MANIFEST': JSON.stringify('{}')
  },
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src/react-app'),
    },
  },
})