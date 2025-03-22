import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    host: "0.0.0.0", // Позволява достъп извън контейнера
    port: 4173,
    strictPort: true,
    hmr: {
      clientPort: 4173,
    }
  }
})