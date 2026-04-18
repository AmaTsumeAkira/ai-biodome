import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { viteSingleFile } from 'vite-plugin-singlefile';

export default defineConfig({
  plugins: [react(), viteSingleFile()],
  build: {
    target: 'es2018',
    minify: 'terser',
    terserOptions: {
      compress: { drop_console: true, drop_debugger: true },
    },
  },
  server: {
    proxy: {
      '/api': 'http://192.168.1.100',
      '/ws': { target: 'ws://192.168.1.100:81', ws: true },
    },
  },
});
