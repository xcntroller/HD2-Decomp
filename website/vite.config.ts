import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// Relative assets + hash navigation also work at /<repository>/ on GitHub Pages.
export default defineConfig({
  plugins: [react()],
  base: "./",
  build: { sourcemap: false },
});
