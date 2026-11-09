import type { Dataset } from "./data";
import { dateLabel, number, statusLabel } from "./data";
import { Badge, EvidenceLinks, Icon } from "./components";
import presentation from "./presentation.json";

export default function Overview({ data }: { data: Dataset }) {
  const active = data.milestones.find((m) => m.id === data.active_milestone);
  const latest = data.checkpoints.filter(
    (c) => c.date === data.snapshot.latest_receipt_date,
  );
  const stages = Object.entries(presentation.stages);
  const sourceNodes = data.graph.nodes.filter(
    (n) => n.id.startsWith("subsystem:") && !n.id.includes(":checkpoint:"),
  );
  const issueCount = Object.values(data.diagnostics).reduce((a, b) => a + b, 0);
  return (
    <>
      <header className="overview-heading">
        <div>
          <p className="eyebrow">
            Reconstruction record <span className="tiny-square" /> Base game
          </p>
          <h1>
            Hidden &amp; Dangerous <span>2</span>
          </h1>
          <div className="file-subtitle">RECONSTRUCTION PROJECT</div>
          <p>
            {presentation.description} Source recovery, outstanding
            dependencies, and recorded checks.
          </p>
        </div>
        <span className="snapshot-stamp">
          <Icon name="layers" />
          <span>
            Report date
            <strong>{dateLabel(data.snapshot.latest_receipt_date)}</strong>
          </span>
        </span>
      </header>
      <section className="hero-grid" aria-label="Current project position">
        <article className="milestone-card">
          <div className="section-top">
            <span className="eyebrow">Current milestone</span>
            <Badge value={active?.reported_status} />
          </div>
          <span className="milestone-id mono">{data.active_milestone}</span>
          <h2>{active?.title ?? "Milestone evidence unavailable"}</h2>
          <p>
            {active?.acceptance ??
              "No acceptance contract is available in this snapshot."}
          </p>
          <div className="hero-bottom">
            <a className="button primary" href="#/roadmap">
              Explore the roadmap <Icon name="arrow" size={16} />
            </a>
            <EvidenceLinks items={active?.evidence} compact />
          </div>
          <svg
            className="field-map"
            viewBox="0 0 460 380"
            fill="none"
            aria-hidden="true"
          >
            <g stroke="currentColor">
              <path d="M-50 340C10 180 120 280 190 160S310 30 490 120M-40 365C30 170 140 310 215 160S350 60 490 150M-60 300C20 145 90 235 160 130S295-10 490 90M-40 250C50 105 85 180 140 85S300-70 490 55M-30 220C45 75 75 150 125 45S330-70 480 15M-20 395C60 210 170 330 240 190S375 90 490 195M0 435C90 235 190 370 275 210S400 130 500 235" />
              <path
                strokeDasharray="6 5"
                d="M30 290L160 230 250 270 350 150 440 185"
              />
              <path d="M245 245v50m-25-25h50M335 145h30m-15-15v30" />
            </g>
          </svg>
        </article>
        <article className="panel reality-card">
          <div className="section-top">
            <span className="eyebrow">First mission frame</span>
            <Icon name="info" />
          </div>
          <h3>Verification status</h3>
          <p>
            Status for each verification level. Native and compilation reports
            retain the scope described in their receipts.
          </p>
          <ol className="verification-list">
            {stages.map(([key, label], i) => {
              const relevant = sourceNodes.filter(
                (n) => n.data.verification_levels[key]?.status !== "unknown",
              );
              const hasUnresolved = relevant.some(
                (n) => n.data.verification_levels[key]?.status === "unresolved",
              );
              return (
                <li key={key}>
                  <span className="stage-number mono">0{i + 1}</span>
                  <span>{label}</span>
                  <span
                    className={`stage-status ${hasUnresolved ? "open" : ""}`}
                  >
                    {key === "source_recovery" &&
                    relevant.some((n) => n.data.status === "supported_subset")
                      ? "Subsets recovered"
                      : hasUnresolved
                        ? "Open"
                        : "Scoped / unknown"}
                  </span>
                </li>
              );
            })}
          </ol>
          <a className="text-link" href="#/dependencies">
            Inspect the first-frame boundary <Icon name="arrow" size={15} />
          </a>
        </article>
      </section>
      <section aria-labelledby="function-counts">
        <div className="section-top">
          <h2 id="function-counts">Function ledger</h2>
          <a className="text-link" href="#/functions">
            Browse original addresses <Icon name="arrow" size={15} />
          </a>
        </div>
        <div className="stats-grid">
          {[
            [
              "Complete",
              data.counts.complete,
              "Whole original function bodies",
              "green",
            ],
            [
              "Partial",
              data.counts.partial,
              "Supported subsets, with exclusions",
              "amber",
            ],
            [
              "Uncredited",
              data.counts.without_ledger_claim,
              "Frozen entries without a ledger claim",
              "muted",
            ],
            [
              "Frozen inventory",
              data.counts.frozen_internal_non_thunk,
              "Base client + original LS3D engine",
              "muted",
            ],
          ].map(([label, value, detail, color]) => (
            <article className="stat-card" key={String(label)}>
              <span className={`stat-label ${color}`}>
                <i />
                {label}
              </span>
              <strong className="mono">
                {number(value as number | undefined)}
              </strong>
              <p>{detail}</p>
            </article>
          ))}
        </div>
        <p className="data-caption">
          <Icon name="info" size={14} /> An estimated function inventory is not
          a percentage of game completion, playability, or time remaining.
          Partial bodies earn no fractional completion credit.
        </p>
      </section>
      <section aria-labelledby="areas">
        <div className="section-top">
          <h2 id="areas">Project areas</h2>
          <span className="small muted">
            Only evidence-backed progress is shown
          </span>
        </div>
        <div className="areas-grid">
          {data.areas.map((area) => (
            <article className="panel area-card" key={area.id}>
              <div className="section-top">
                <span className="area-icon">
                  <Icon
                    name={
                      area.id === "client"
                        ? "code"
                        : area.id === "server"
                          ? "server"
                          : "layers"
                    }
                  />
                </span>
                <Badge value={area.status} />
              </div>
              <h3>{area.name}</h3>
              <p>
                {area.id === "client"
                  ? `Current boundary: ${area.milestone ?? "unknown"}. Source recovery and integration are tracked independently.`
                  : area.id === "server"
                    ? "Server lifecycle and live-network verification have their own delivery scope. No server function census is included in this snapshot."
                    : "Separate from the current client scope. Existing LS3D source counts do not measure replacement-engine progress."}
              </p>
              <EvidenceLinks items={area.evidence} compact />
            </article>
          ))}
        </div>
      </section>
      <section className="activity-preview">
        <div>
          <p className="eyebrow">Latest source reports</p>
          <h2>Recent checkpoints</h2>
          <p>
            {latest.length
              ? `${latest.length} receipts share the newest recorded date. Native and i386 reports preserve their fixture limits; same-day ordering is not established.`
              : "No verified subsystem receipt is included in this snapshot."}
          </p>
          <a className="text-link" href="#/activity">
            View development activity <Icon name="arrow" size={16} />
          </a>
        </div>
        <div className="panel receipt-preview">
          {latest
            .slice()
            .sort((a, b) => a.title.localeCompare(b.title))
            .slice(0, 3)
            .map((c) => (
              <article key={c.id}>
                <span className="receipt-dot" />
                <div>
                  <span className="small muted">
                    {dateLabel(c.date)} ·{" "}
                    {c.source_only === true
                      ? "Source only"
                      : "Scope in receipt"}
                  </span>
                  <h3>{c.title}</h3>
                  <p>{c.scope}</p>
                  <EvidenceLinks items={c.evidence} compact />
                </div>
              </article>
            ))}
        </div>
      </section>
      {issueCount > 0 && (
        <aside className="notice">
          <Icon name="info" />
          <div>
            <strong>{issueCount} documentary consistency notices</strong>
            <p>
              The analyzer detected stale cached references in the phase index.
              Function counts use the current ledger. These notices do not
              establish a gameplay defect.
            </p>
            <details>
              <summary>Show data notices</summary>
              <ul>
                {Object.entries(data.diagnostics).map(([code, count]) => (
                  <li key={code}>
                    {code.replaceAll("_", " ")}: {count}
                  </li>
                ))}
              </ul>
            </details>
          </div>
        </aside>
      )}
    </>
  );
}
