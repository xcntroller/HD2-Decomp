import { useEffect, useMemo, useState } from "react";
import type { Dataset, FunctionRecord } from "./data";
import { listedFunctions, number, plain } from "./data";
import { Badge, Empty, EvidenceLinks, Icon, PageHeading } from "./components";

export default function Functions({
  data,
  address = "",
}: {
  data: Dataset;
  address?: string;
}) {
  const [query, setQuery] = useState(address);
  const [program, setProgram] = useState("all");
  const [status, setStatus] = useState("all");
  const [group, setGroup] = useState("all");
  const [page, setPage] = useState(0);
  const [selected, setSelected] = useState<FunctionRecord | null>(null);
  useEffect(() => {
    setQuery(address);
  }, [address]);
  const all = useMemo(() => listedFunctions(data), [data]);
  const nodes = useMemo(
    () => new Map(data.graph.nodes.map((n) => [n.id, n])),
    [data],
  );
  const groups = data.graph.groups.filter((g) =>
    g.member_ids.some((id) => id.startsWith("function:")),
  );
  const membership = data.graph.groups.find((g) => g.id === group);
  const filtered = all.filter(
    (f) =>
      (!query ||
        f.entry.includes(query.trim().replace(/^0x/i, "").toLowerCase())) &&
      (program === "all" || f.program === program) &&
      (status === "all" || f.status === status) &&
      (group === "all" || membership?.member_ids.includes(f.id)),
  );
  useEffect(() => setPage(0), [query, program, status, group]);
  const pageCount = Math.max(1, Math.ceil(filtered.length / 25));
  const current = Math.min(page, pageCount - 1);
  const selection =
    selected && filtered.some((f) => f.id === selected.id) ? selected : null;
  return (
    <>
      <PageHeading
        eyebrow="Original code / function records"
        title="Original function ledger"
      >
        Search original addresses and inspect documented coverage, limitations
        and source mappings.
      </PageHeading>
      <div className="toolbar function-toolbar">
        <label className="search-field">
          <span>Original address</span>
          <span className="search-input">
            <Icon name="search" size={17} />
            <input
              aria-label="Search original function address"
              placeholder="Search an address, e.g. 00416e10"
              value={query}
              onChange={(e) => setQuery(e.target.value)}
            />
          </span>
        </label>
        <label>
          Program
          <select value={program} onChange={(e) => setProgram(e.target.value)}>
            <option value="all">All programs</option>
            <option value="client">Base game client</option>
            <option value="engine">Original LS3D engine</option>
          </select>
        </label>
        <label>
          Reconstruction status
          <select value={status} onChange={(e) => setStatus(e.target.value)}>
            <option value="all">All statuses</option>
            <option value="complete">Complete</option>
            <option value="partial">Partial</option>
            <option value="no_ledger_claim">Uncredited references</option>
          </select>
        </label>
        <label>
          Documented group
          <select value={group} onChange={(e) => setGroup(e.target.value)}>
            <option value="all">All groups</option>
            {groups.map((g) => (
              <option value={g.id} key={g.id}>
                {nodes.get(g.id)?.data.label ?? g.id}
              </option>
            ))}
          </select>
        </label>
      </div>
      <aside className="notice compact-notice">
        <Icon name="info" size={18} />
        <p>
          <strong>
            {number(data.counts.without_ledger_claim)} frozen entries have no
            ledger claim.
          </strong>{" "}
          The full inventory is not published. Only{" "}
          {number(all.filter((f) => f.status === "no_ledger_claim").length)}{" "}
          additional explicitly referenced addresses can be listed here. Receipt
          group membership can be inferred from address matches; it does not
          prove a call dependency.
        </p>
      </aside>
      <div className={`function-layout ${selection ? "has-selection" : ""}`}>
        <section className="panel table-panel">
          <div className="section-top table-top">
            <strong>{number(filtered.length)} listed addresses</strong>
            <span className="small muted">25 per page</span>
          </div>
          {filtered.length ? (
            <>
              <div className="table-scroll">
                <table>
                  <thead>
                    <tr>
                      <th scope="col">Original address</th>
                      <th scope="col">Program</th>
                      <th scope="col">Status</th>
                      <th scope="col">Sources</th>
                    </tr>
                  </thead>
                  <tbody>
                    {filtered
                      .slice(current * 25, current * 25 + 25)
                      .map((f) => (
                        <tr
                          className={
                            selection?.id === f.id ? "selected-row" : ""
                          }
                          key={f.id}
                        >
                          <td>
                            <button
                              className="address-button mono"
                              onClick={() => setSelected(f)}
                            >
                              {f.entry}
                              <Icon name="arrow" size={13} />
                            </button>
                          </td>
                          <td>
                            {f.program === "client"
                              ? "Game client"
                              : "Original engine"}
                          </td>
                          <td>
                            <Badge value={f.status} />
                          </td>
                          <td className="mono muted">{f.evidence.length}</td>
                        </tr>
                      ))}
                  </tbody>
                </table>
              </div>
              <div className="pagination">
                <button
                  className="button secondary"
                  disabled={current === 0}
                  onClick={() => setPage(current - 1)}
                >
                  Previous
                </button>
                <span className="small">
                  Page {current + 1} of {pageCount}
                </span>
                <button
                  className="button secondary"
                  disabled={current + 1 >= pageCount}
                  onClick={() => setPage(current + 1)}
                >
                  Next
                </button>
              </div>
            </>
          ) : (
            <Empty title="No matching address">
              Try another address, program, status or documented group.
            </Empty>
          )}
        </section>
        {selection && (
          <aside className="panel inspector" aria-label="Function inspection">
            <div className="section-top">
              <p className="eyebrow">Original function record</p>
              <button
                className="icon-button"
                aria-label="Close function inspection"
                onClick={() => setSelected(null)}
              >
                ×
              </button>
            </div>
            <h2 className="mono">{selection.entry}</h2>
            <Badge value={selection.status} />
            <h4>Documented coverage</h4>
            <p>
              {plain(selection.coverage) ||
                "No reconstruction claim is recorded for this referenced address."}
            </p>
            <h4>Limitations</h4>
            <p>
              {plain(selection.limitations) ||
                (selection.status === "complete"
                  ? "No exclusions are recorded in the complete-function claim. This does not establish integration or desktop acceptance."
                  : "Behavior and verification remain unknown.")}
            </p>
            {selection.verification && (
              <>
                <h4>Recorded function verification</h4>
                <p>{selection.verification}</p>
              </>
            )}
            <h4>Documented groups</h4>
            <ul className="small group-list">
              {groups
                .filter((g) => g.member_ids.includes(selection.id))
                .map((g) => (
                  <li key={g.id}>{nodes.get(g.id)?.data.label ?? g.id}</li>
                ))}
            </ul>
            <h4>Source mappings & evidence</h4>
            <EvidenceLinks items={selection.evidence} />
            <p className="small muted">
              Source mappings are ledger evidence links, not a proven one-to-one
              mapping of every original instruction. Native, i386, integration
              and desktop levels are not inferred for this function.
            </p>
          </aside>
        )}
      </div>
    </>
  );
}
