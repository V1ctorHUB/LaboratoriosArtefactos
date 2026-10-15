import react from '@vitejs/plugin-react'
import { defineConfig } from 'vite'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    // La ruta del proyecto contiene ":" (02:2026) y Vite la rechaza
    // como "outside of Vite serving allow list". En desarrollo local
    // se desactiva esa verificación.
    fs: { strict: false },
  },
})
