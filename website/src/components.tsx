import type { ReactNode } from "react";
import type { Evidence } from "./data";
import { safeLink, statusLabel, tone } from "./data";

export function Icon({ name, size = 20 }: { name: string; size?: number }) {
  const paths: Record<string, ReactNode> = {
    overview: (
      <>
        <rect x="3" y="3" width="7" height="7" rx="1.5" />
        <rect x="14" y="3" width="7" height="7" rx="1.5" />
        <rect x="3" y="14" width="7" height="7" rx="1.5" />
        <rect x="14" y="14" width="7" height="7" rx="1.5" />
      </>
    ),
    roadmap: (
      <>
        <path d="M5 3v18M5 5h13l-3 4 3 4H5" />
      </>
    ),
    graph: (
      <>
        <rect x="8" y="2" width="8" height="5" rx="1" />
        <rect x="2" y="17" width="8" height="5" rx="1" />
        <rect x="14" y="17" width="8" height="5" rx="1" />
        <path d="M12 7v5M6 17v-5h12v5" />
      </>
    ),
    code: (
      <>
        <path d="m8 7-5 5 5 5m8-10 5 5-5 5m-3-13-2 20" />
      </>
    ),
    activity: <path d="M2 12h4l3-8 5 16 3-8h5" />,
    arrow: <path d="M5 12h14m-6-6 6 6-6 6" />,
    external: (
      <>
        <path d="M14 3h7v7m0-7-11 11M10 3H4a1 1 0 0 0-1 1v16a1 1 0 0 0 1 1h16a1 1 0 0 0 1-1v-6" />
      </>
    ),
    moon: <path d="M20 15.5A8.5 8.5 0 0 1 8.5 4 8.5 8.5 0 1 0 20 15.5Z" />,
    search: (
      <>
        <circle cx="10.5" cy="10.5" r="6.5" />
        <path d="m16 16 5 5" />
      </>
    ),
    check: <path d="m5 12 4 4L19 6" />,
    server: (
      <>
        <rect x="3" y="3" width="18" height="7" rx="2" />
        <rect x="3" y="14" width="18" height="7" rx="2" />
        <path d="M7 6.5h.01M7 17.5h.01m4-11h6m-6 11h6" />
      </>
    ),
    layers: (
      <>
        <path d="m12 3 10 5-10 5L2 8l10-5Zm-9 9 9 5 9-5m-18 5 9 5 9-5" />
      </>
    ),
    info: (
      <>
        <circle cx="12" cy="12" r="9" />
        <path d="M12 11v6m0-10h.01" />
      </>
    ),
  };
  return (
    <svg
      width={size}
      height={size}
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="1.6"
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden="true"
    >
      {paths[name] ?? paths.info}
    </svg>
  );
}
export function Badge({
  value,
  children,
}: {
  value?: string;
  children?: ReactNode;
}) {
  return (
    <span className={`badge ${tone(value)}`}>
      <i />
      {children ?? statusLabel(value)}
    </span>
  );
}
export function EvidenceLinks({
  items = [],
  compact = false,
}: {
  items?: Evidence[];
  compact?: boolean;
}) {
  const visible = compact ? items.slice(0, 3) : items;
  if (!items.length)
    return (
      <span className="muted small">No public evidence link recorded.</span>
    );
  return (
    <div className={`evidence ${compact ? "compact" : ""}`}>
      {visible.map((item, i) => {
        const href = safeLink(item.url);
        const label = item.path.split("/").at(-1) || "Evidence";
        return (
          <div key={`${item.path}:${item.pointer}:${item.line}:${i}`}>
            {href ? (
              <a
                href={href}
                target="_blank"
                rel="noopener noreferrer"
                title={`${item.path}${item.pointer ?? ""}`}
              >
                <Icon name="external" size={13} />
                {label}
                {item.line ? `:${item.line}` : ""}
              </a>
            ) : (
              <span className="muted small" title={item.path}>
                {label} · not publicly linked
              </span>
            )}
            {!compact && item.pointer && (
              <span className="pointer mono">{item.pointer}</span>
            )}
          </div>
        );
      })}
      {compact && items.length > 3 && (
        <span className="small muted">
          +{items.length - 3} source references
        </span>
      )}
    </div>
  );
}
export function Empty({
  title,
  children,
}: {
  title: string;
  children?: ReactNode;
}) {
  return (
    <div className="empty">
      <Icon name="search" size={28} />
      <h3>{title}</h3>
      <p>
        {children ?? "This snapshot does not contain evidence for this view."}
      </p>
    </div>
  );
}
export function PageHeading({
  eyebrow,
  title,
  children,
}: {
  eyebrow: string;
  title: string;
  children: ReactNode;
}) {
  return (
    <header className="page-heading">
      <p className="eyebrow">{eyebrow}</p>
      <h1>{title}</h1>
      <p>{children}</p>
    </header>
  );
}
