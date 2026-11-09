import { useState } from "react";
import type { Dataset, Requirement } from "./data";
import { plain } from "./data";
import { Badge, Empty, EvidenceLinks, Icon, PageHeading } from "./components";

function Requirements({ rows }: { rows: Requirement[] }) {
  return (
    <ul className="requirements">
      {rows.map((row) => (
        <li key={row.id}>
          <span className={`requirement-check ${row.checked ? "checked" : ""}`}>
            {row.checked ? <Icon name="check" size={13} /> : "·"}
          </span>
          <div>
            <p>{plain(row.text)}</p>
            {row.explicit_source_only && (
              <Badge value="source_only_checkpoint" />
            )}
            <EvidenceLinks items={row.evidence} compact />
          </div>
        </li>
      ))}
    </ul>
  );
}
export default function Roadmap({ data }: { data: Dataset }) {
  const [phase, setPhase] = useState("all");
  const [state, setState] = useState("all");
  const legacy = new Map<string, Requirement[]>();
  for (const row of data.checklist) {
    if (row.headings[0]?.startsWith("Phase 2")) continue;
    const key = row.headings.some((h) => h.includes("Postponed"))
      ? row.headings.at(-1)!
      : row.headings.join(" / ");
    legacy.set(key, [...(legacy.get(key) ?? []), row]);
  }
  const milestones = data.milestones.filter(
    (m) =>
      state === "all" ||
      (state === "active"
        ? m.reported_status === "in-progress"
        : state === "completed"
          ? m.reported_status.startsWith("complete")
          : state === "blocked"
            ? m.reported_status.includes("blocked")
            : m.reported_status === "not-started"),
  );
  return (
    <>
      <PageHeading eyebrow="Work plan" title="Reconstruction roadmap">
        Delivery requirements, prerequisites and evidence from the
        reconstruction plan.
      </PageHeading>
      <div className="toolbar">
        <label>
          Project phase
          <select value={phase} onChange={(e) => setPhase(e.target.value)}>
            <option value="all">All phases</option>
            <option value="phase1">Phase 1 · Front end</option>
            <option value="phase2">Phase 2 · Missions</option>
            <option value="phase3">Phase 3 · Server</option>
            <option value="backlog">Postponed backlog</option>
          </select>
        </label>
        <label>
          Milestone status
          <select value={state} onChange={(e) => setState(e.target.value)}>
            <option value="all">All states</option>
            <option value="completed">Completed scope</option>
            <option value="active">Active</option>
            <option value="blocked">Explicitly blocked</option>
            <option value="future">Future</option>
          </select>
        </label>
        <span className="toolbar-note">
          Unresolved prerequisites do not automatically mean blocked.
        </span>
      </div>
      {(phase === "all" || phase === "phase2") && (
        <section>
          <div className="section-top">
            <h2>
              Phase 2{" "}
              <span className="muted">
                / Real missions & game-dependent flows
              </span>
            </h2>
            <span className="small mono">ACTIVE {data.active_milestone}</span>
          </div>
          {!milestones.length && (
            <Empty title="No milestones in this state">
              This snapshot does not report a matching Phase 2 milestone. No
              blocked state is inferred from open work.
            </Empty>
          )}
          <div className="roadmap-list">
            {milestones.map((m) => (
              <details
                key={m.id}
                className={`roadmap-item ${m.id === data.active_milestone ? "current" : ""}`}
                open={m.id === data.active_milestone || undefined}
              >
                <summary>
                  <span className="roadmap-marker">
                    <Icon
                      name={
                        m.reported_status.startsWith("complete")
                          ? "check"
                          : "roadmap"
                      }
                      size={18}
                    />
                  </span>
                  <span className="mono roadmap-id">{m.id}</span>
                  <span className="roadmap-title">{m.title}</span>
                  <Badge value={m.reported_status} />
                  <span className="expand-marker">+</span>
                </summary>
                <div className="roadmap-body">
                  <p className="eyebrow">Acceptance contract</p>
                  <p>{plain(m.acceptance)}</p>
                  <div className="prerequisites">
                    <strong>Prerequisites</strong>
                    {m.prerequisites.length ? (
                      m.prerequisites.map((id) => (
                        <span className="tag mono" key={id}>
                          {id}
                        </span>
                      ))
                    ) : (
                      <span className="muted small">
                        None listed in the phase index
                      </span>
                    )}
                  </div>
                  <Requirements
                    rows={data.checklist.filter((c) =>
                      m.requirements.includes(c.id),
                    )}
                  />
                  <EvidenceLinks items={m.evidence} />
                  <a className="text-link" href="#/dependencies">
                    Inspect dependency evidence <Icon name="arrow" size={14} />
                  </a>
                </div>
              </details>
            ))}
          </div>
        </section>
      )}
      {[...legacy.entries()]
        .filter(
          ([, rows]) =>
            phase === "all" ||
            (phase === "phase1"
              ? rows[0].headings[0]?.startsWith("Phase 1")
              : phase === "phase3"
                ? rows[0].headings[0]?.startsWith("Phase 3") &&
                  !rows[0].headings.some((h) => h.includes("Postponed"))
                : phase === "backlog" &&
                  rows[0].headings.some((h) => h.includes("Postponed"))),
        )
        .filter(
          ([, rows]) =>
            state === "all" ||
            (state === "completed"
              ? rows.every((r) => r.checked)
              : state === "future"
                ? rows.every((r) => !r.checked)
                : false),
        )
        .map(([title, rows]) => (
          <details className="panel historical-phase" key={title}>
            <summary>
              <span>
                <span className="eyebrow">Plan checklist · recorded scope</span>
                <h3>{title}</h3>
              </span>
              <span className="tag">
                {rows.every((r) => r.checked)
                  ? "Checklist checked · scope applies"
                  : "Open / future checklist"}
              </span>
            </summary>
            <Requirements rows={rows} />
          </details>
        ))}
      {phase !== "all" && phase !== "phase2" && state === "blocked" && (
        <Empty title="No explicit blocked milestone is recorded" />
      )}
      <p className="data-caption">
        <Icon name="info" size={14} /> A checked source-only item is not desktop
        delivery. The postponed function-completion pass is not a prerequisite
        for Phase 2.
      </p>
    </>
  );
}
