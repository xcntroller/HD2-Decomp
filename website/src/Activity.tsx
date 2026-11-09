import { useState } from "react";
import type { Dataset } from "./data";
import { dateLabel, plain, safeLink } from "./data";
import { Badge, Empty, EvidenceLinks, Icon, PageHeading } from "./components";
import presentation from "./presentation.json";

export default function Activity({ data }: { data: Dataset }) {
  const [tab, setTab] = useState("checkpoints");
  const stages: Record<string, string> = presentation.stages;
  return (
    <>
      <PageHeading
        eyebrow="Changes / verification receipts"
        title="Development log"
      >
        Commits and subsystem verification receipts from this repository
        snapshot.
      </PageHeading>
      <div className="tabs" role="group" aria-label="Activity view">
        <button
          className={tab === "checkpoints" ? "active" : ""}
          onClick={() => setTab("checkpoints")}
        >
          Subsystem checkpoints <span>{data.checkpoints.length}</span>
        </button>
        <button
          className={tab === "commits" ? "active" : ""}
          onClick={() => setTab("commits")}
        >
          Recent commits <span>{data.commits.length}</span>
        </button>
      </div>
      {tab === "checkpoints" ? (
        <>
          <p className="small muted">
            Receipts are ordered by date, then title. Same-day sequence is
            unknown. “Verified” here means a repository report exists; these
            checks were not rerun by the website.
          </p>
          <div className="checkpoint-list">
            {data.checkpoints
              .slice()
              .sort(
                (a, b) =>
                  b.date.localeCompare(a.date) ||
                  a.title.localeCompare(b.title),
              )
              .map((c) => (
                <details className="panel checkpoint" key={c.id}>
                  <summary>
                    <span className="checkpoint-icon">
                      <Icon name="check" />
                    </span>
                    <div>
                      <span className="small muted">{dateLabel(c.date)}</span>
                      <h3>{c.title}</h3>
                      <p>{c.scope}</p>
                    </div>
                    <div className="checkpoint-labels">
                      <Badge
                        value={
                          c.source_only ? "source_only_checkpoint" : "unknown"
                        }
                      />
                      <span className="small muted">
                        {c.desktop_verified === false
                          ? "No desktop acceptance"
                          : c.desktop_verified === true
                            ? "Desktop report in receipt"
                            : "Desktop status unknown"}
                      </span>
                    </div>
                    <span className="expand-marker">+</span>
                  </summary>
                  <div className="checkpoint-body">
                    {c.observations.map((v, i) => (
                      <article className="verification-report" key={i}>
                        <p className="eyebrow">{stages[v.stage] ?? v.stage}</p>
                        <p>
                          {typeof v.reported === "string"
                            ? plain(v.reported)
                            : JSON.stringify(v.reported, null, 2)}
                        </p>
                        <EvidenceLinks items={v.evidence} />
                      </article>
                    ))}
                    <EvidenceLinks items={c.evidence} />
                  </div>
                </details>
              ))}
          </div>
          {!data.checkpoints.length && (
            <Empty title="No verification receipts are published" />
          )}
        </>
      ) : (
        <>
          <p className="small muted">
            History belongs to the analyzed worktree snapshot. Local-only
            commits retain their SHA and have no public link.
          </p>
          <div className="panel commits">
            {data.commits.map((commit) => (
              <article className="commit-row" key={commit.sha}>
                <span className="commit-dot" />
                <div>
                  <span className="small muted">
                    {dateLabel(commit.date)} · committed change
                  </span>
                  <h3>{commit.subject}</h3>
                  <span className="small muted">
                    {commit.publication === "local"
                      ? "Local commit · not publicly linked"
                      : "Public repository history"}{" "}
                    · verification is separate
                  </span>
                </div>
                {safeLink(commit.url) ? (
                  <a
                    href={safeLink(commit.url)}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="commit-hash mono"
                  >
                    {commit.sha.slice(0, 7)} <Icon name="external" size={12} />
                  </a>
                ) : (
                  <span className="mono small">{commit.sha.slice(0, 7)}</span>
                )}
              </article>
            ))}
          </div>
          {!data.commits.length && (
            <Empty title="Commit history is unavailable" />
          )}
        </>
      )}
    </>
  );
}
