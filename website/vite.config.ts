import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// Hash routes stay under this prefix and need no server-side rewrite rules.
export default defineConfig({
  plugins: [react()],
  base: "/HD2-Decomp/",
  build: { sourcemap: false, manifest: true },
});
