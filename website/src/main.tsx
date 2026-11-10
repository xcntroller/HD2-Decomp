import { lazy, Suspense, useEffect, useState } from "react";
import { createRoot } from "react-dom/client";
import type { Dataset } from "./data";
import { normalize } from "./data";
import { Empty, Icon } from "./components";
import Overview from "./Overview";
import Roadmap from "./Roadmap";
import Functions from "./Functions";
import Activity from "./Activity";
import presentation from "./presentation.json";
import "@fontsource/barlow-condensed/latin-600.css";
import "@fontsource/barlow-condensed/latin-700.css";
import "@fontsource/ibm-plex-mono/latin-400.css";
import "./styles.css";
import "./field.css";

const Research = lazy(() => import("./Research"));
const Dependencies = lazy(() => import("./Dependencies"));
const allowed = new Set(presentation.navigation.map((n) => n.id));
function route() {
  const [path, query = ""] = window.location.hash
    .slice(1)
    .replace(/^\//, "")
    .split("?");
  return {
    page: allowed.has(path) ? path : "overview",
    query: new URLSearchParams(query),
  };
}
function initialTheme() {
  try {
    const stored = localStorage.getItem("hd2-theme");
    if (stored) return stored === "dark";
  } catch {
    /* Storage is optional. */
  }
  return true;
}
function App() {
  const [current, setCurrent] = useState(route);
  const [data, setData] = useState<Dataset | null>(null);
  const [error, setError] = useState("");
  const [attempt, setAttempt] = useState(0);
  const [dark, setDark] = useState(initialTheme);
  useEffect(() => {
    const change = () => {
      setCurrent(route());
      window.scrollTo(0, 0);
    };
    window.addEventListener("hashchange", change);
    return () => window.removeEventListener("hashchange", change);
  }, []);
  useEffect(() => {
    document.documentElement.dataset.theme = dark ? "dark" : "light";
    try {
      localStorage.setItem("hd2-theme", dark ? "dark" : "light");
    } catch {
      /* Theme still works without persistence. */
    }
  }, [dark]);
  useEffect(() => {
    const controller = new AbortController();
    setError("");
    fetch(`${import.meta.env.BASE_URL}data/progress.json`, {
      signal: controller.signal,
    })
      .then((response) => {
        if (!response.ok)
          throw new Error("The public progress snapshot could not be loaded.");
        return response.json();
      })
      .then(normalize)
      .then(setData)
      .catch((reason) => {
        if (reason.name !== "AbortError")
          setError(
            reason instanceof Error
              ? reason.message
              : "Progress data is unavailable.",
          );
      });
    return () => controller.abort();
  }, [attempt]);
  const page = presentation.navigation.find((n) => n.id === current.page);
  return (
    <div className="app-shell">
      <a
        className="skip-link"
        href="#main"
        onClick={(event) => {
          event.preventDefault();
          document.getElementById("main")?.focus();
        }}
      >
        Skip to content
      </a>
      <aside className="sidebar">
        <a href="#/overview" className="brand">
          <span className="brand-symbol">
            HD<span>2</span>
          </span>
          <span>
            {presentation.title}
            <small>{presentation.subtitle}</small>
          </span>
        </a>
        <div className="sidebar-label">Project records</div>
        <nav
          aria-label="Primary navigation"
          onKeyDown={(event) => {
            const links = Array.from(
              event.currentTarget.querySelectorAll<HTMLAnchorElement>("a"),
            );
            const current = links.indexOf(
              (event.target as HTMLElement).closest("a") as HTMLAnchorElement,
            );
            let next: number | undefined;
            if (event.key === "ArrowDown") next = (current + 1) % links.length;
            if (event.key === "ArrowUp")
              next = (current - 1 + links.length) % links.length;
            if (event.key === "Home") next = 0;
            if (event.key === "End") next = links.length - 1;
            if (event.key === "Escape") {
              event.preventDefault();
              links[0]?.click();
              links[0]?.focus();
            }
            if (next !== undefined) {
              event.preventDefault();
              links[next]?.focus();
            }
          }}
        >
          {presentation.navigation.map((item) => (
            <a
              key={item.id}
              href={`#/${item.id}`}
              className={current.page === item.id ? "active" : ""}
              aria-current={current.page === item.id ? "page" : undefined}
            >
              <span>{item.label}</span>
              {current.page === item.id && <span className="nav-dot" />}
            </a>
          ))}
        </nav>
        <div className="sidebar-bottom">
          <span className="sidebar-status">
            <i />
            Public project record
          </span>
          <p className="menu-snapshot mono">
            {data?.snapshot.source_commit?.slice(0, 7) || "Unknown"}
          </p>
          <div className="menu-keys">↑ ↓ Select · Enter Open</div>
          <a
            href="https://github.com/xcntroller/HD2-Decomp"
            target="_blank"
            rel="noopener noreferrer"
          >
            View on GitHub <Icon name="external" size={14} />
          </a>
        </div>
      </aside>
      <div className="main-shell">
        <header className="topbar">
          <div className="breadcrumb">
            <span>HD2</span>
            <span>/</span>
            <strong>{page?.label}</strong>
          </div>
          <div className="topbar-right">
            <span className="small muted snapshot-label">
              Snapshot{" "}
              <span className="mono">
                {data?.snapshot.source_commit?.slice(0, 7) || "—"}
              </span>
            </span>
            <button
              className="theme-button"
              aria-label={dark ? "Switch to light mode" : "Switch to dark mode"}
              onClick={() => setDark(!dark)}
            >
              <Icon name="moon" size={17} />
              <span>{dark ? "Light mode" : "Dark mode"}</span>
            </button>
          </div>
        </header>
        <main id="main" tabIndex={-1}>
          {error ? (
            <Empty title="Progress snapshot unavailable">
              <span>{error}</span>
              <button
                className="button secondary retry"
                onClick={() => setAttempt(attempt + 1)}
              >
                Retry data load
              </button>
            </Empty>
          ) : !data ? (
            <div className="loading" role="status">
              <span className="loading-dot" />
              Loading the repository snapshot…
            </div>
          ) : (
            <Suspense
              fallback={
                <div className="loading" role="status">
                  Loading dependency explorer…
                </div>
              }
            >
              {current.page === "research" ? (
                <Research dark={dark} />
              ) : current.page === "overview" ? (
                <Overview data={data} />
              ) : current.page === "roadmap" ? (
                <Roadmap data={data} />
              ) : current.page === "functions" ? (
                <Functions
                  data={data}
                  address={current.query.get("address") ?? ""}
                />
              ) : current.page === "activity" ? (
                <Activity data={data} />
              ) : (
                <Dependencies data={data} dark={dark} />
              )}
            </Suspense>
          )}
        </main>
        <footer>
          <span>
            Hidden & Dangerous 2 reconstruction{" "}
            <span className="footer-dot">·</span> Independent project dashboard
          </span>
          <span>Read-only snapshot</span>
        </footer>
      </div>
    </div>
  );
}

createRoot(document.getElementById("root")!).render(<App />);
