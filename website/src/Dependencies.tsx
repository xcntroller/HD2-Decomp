import { useMemo, useState } from "react";
import {
  Background,
  Controls,
  Handle,
  MarkerType,
  Position,
  ReactFlow,
  type Node,
  type NodeProps,
  type Edge,
} from "@xyflow/react";
import "@xyflow/react/dist/style.css";
import type { Dataset, GraphNode, GraphEdge } from "./data";
import { plain, statusLabel, tone } from "./data";
import { Badge, Empty, EvidenceLinks, Icon, PageHeading } from "./components";
import presentation from "./presentation.json";

type VisualNode = Node<
  { label: string; status: string; kind: string },
  "record"
>;
function RecordNode({ data, selected }: NodeProps<VisualNode>) {
  return (
    <div className={`graph-record ${selected ? "selected" : ""}`}>
      <Handle type="target" position={Position.Left} />
      <p className="eyebrow">{data.kind}</p>
      <strong>{data.label}</strong>
      <span className={`graph-status ${tone(data.status)}`}>
        {statusLabel(data.status)}
      </span>
      <Handle type="source" position={Position.Right} />
    </div>
  );
}
const nodeTypes = { record: RecordNode };

function layout(nodes: GraphNode[], edges: GraphEdge[]): VisualNode[] {
  const ranks = new Map(nodes.map((n) => [n.id, 0]));
  // A bounded longest-path layout of explicit dependencies. Inferred edges do not determine order.
  const ordered = edges.filter(
    (e) =>
      e.data.basis === "explicit" &&
      e.data.evidence_state === "verified" &&
      e.data.relationship === "depends_on",
  );
  for (let i = 0; i < nodes.length; i++) {
    let changed = false;
    for (const e of ordered) {
      const next = Math.min(nodes.length, (ranks.get(e.source) ?? 0) + 1);
      if (ranks.has(e.target) && next > (ranks.get(e.target) ?? 0)) {
        ranks.set(e.target, next);
        changed = true;
      }
    }
    if (!changed) break;
  }
  const last = Math.max(0, ...ranks.values()) + 1;
  const rows = new Map<number, number>();
  return nodes.map((n) => {
    const column = n.type === "function" ? last : (ranks.get(n.id) ?? 0);
    const row = rows.get(column) ?? 0;
    rows.set(column, row + 1);
    return {
      id: n.id,
      type: "record",
      position: { x: column * 340, y: row * 160 },
      data: { label: n.data.label, status: n.data.status, kind: n.type },
    };
  });
}

export default function Dependencies({
  data,
  dark,
}: {
  data: Dataset;
  dark: boolean;
}) {
  const [scope, setScope] = useState("first-frame");
  const [group, setGroup] = useState("");
  const [basis, setBasis] = useState("explicit");
  const [status, setStatus] = useState("all");
  const [showFunctions, setShowFunctions] = useState(false);
  const [limit, setLimit] = useState(30);
  const [selectedId, setSelectedId] = useState("");
  const [selectedEdge, setSelectedEdge] = useState<GraphEdge | null>(null);
  const lookup = useMemo(
    () => new Map(data.graph.nodes.map((n) => [n.id, n])),
    [data],
  );
  const groups = data.graph.groups.filter(
    (g) => lookup.get(g.id)?.type === "subsystem",
  );
  const selectedGroup = data.graph.groups.find((g) => g.id === group);
  const base = data.graph.nodes.filter((n) =>
    scope === "milestones"
      ? n.type === "milestone"
      : scope === "group"
        ? n.id === group ||
          (n.type !== "function" && selectedGroup?.member_ids.includes(n.id))
        : n.type === "subsystem" && !n.id.includes(":checkpoint:"),
  );
  const groupMembers =
    scope === "group"
      ? (selectedGroup?.member_ids ?? [])
      : base.flatMap(
          (n) => data.graph.groups.find((g) => g.id === n.id)?.member_ids ?? [],
        );
  const related = [...new Set(groupMembers)]
    .map((id) => lookup.get(id))
    .filter((n): n is GraphNode => n?.type === "function");
  const visible = [
    ...base,
    ...(showFunctions ? related.slice(0, limit) : []),
  ].filter((n) => status === "all" || n.data.status === status);
  const ids = new Set(visible.map((n) => n.id));
  const sourceEdges = data.graph.edges.filter(
    (e) =>
      ids.has(e.source) &&
      ids.has(e.target) &&
      (e.data.relationship === "depends_on" || showFunctions) &&
      (basis === "all" ||
        (basis === "unknown"
          ? e.data.evidence_state === "unknown"
          : e.data.basis === "explicit" &&
            e.data.evidence_state === "verified")),
  );
  const rfNodes = layout(visible, sourceEdges).map((n) => ({
    ...n,
    selected: n.id === selectedId,
  }));
  const rfEdges: Edge[] = sourceEdges.map((e) => ({
    id: e.id,
    source: e.source,
    target: e.target,
    type: "smoothstep",
    style: {
      stroke:
        e.data.evidence_state === "unknown"
          ? "#a381b5"
          : e.data.basis === "inferred"
            ? "#bf8a3b"
            : "#81927a",
      strokeWidth: 1.6,
      strokeDasharray:
        e.data.evidence_state === "unknown"
          ? "2 5"
          : e.data.basis === "inferred"
            ? "7 5"
            : undefined,
    },
    markerEnd: {
      type: MarkerType.ArrowClosed,
      color: e.data.basis === "inferred" ? "#bf8a3b" : "#81927a",
    },
    interactionWidth: 22,
    data: e.data,
  }));
  const graphSelection = lookup.get(selectedId);
  const sourceFunction = data.functions.find((f) => f.id === selectedId);
  const selected =
    graphSelection && sourceFunction
      ? {
          ...graphSelection,
          data: {
            ...graphSelection.data,
            coverage: sourceFunction.coverage,
            limitations: sourceFunction.limitations,
            evidence: sourceFunction.evidence,
          },
        }
      : graphSelection;
  const incoming = data.graph.edges.filter(
    (e) => e.target === selectedId && e.data.relationship === "depends_on",
  );
  const functions = (
    data.graph.groups.find((g) => g.id === selectedId)?.member_ids ?? []
  )
    .map((id) => lookup.get(id))
    .filter((n): n is GraphNode => n?.type === "function");
  const stageLabels: Record<string, string> = presentation.stages;
  const reset = (next: string) => {
    setScope(next);
    setStatus("all");
    setSelectedId("");
    setSelectedEdge(null);
    setShowFunctions(false);
    setLimit(30);
    if (next === "group" && !group) setGroup(groups[0]?.id ?? "");
  };
  return (
    <>
      <PageHeading
        eyebrow="Loading route / dependency records"
        title="Mission-loading dependencies"
      >
        Select a node or edge to inspect its evidence. Function nodes are hidden
        until requested.
      </PageHeading>
      <div className="toolbar graph-toolbar">
        <label>
          Graph scope
          <select value={scope} onChange={(e) => reset(e.target.value)}>
            <option value="first-frame">First mission frame</option>
            <option value="milestones">Phase 2 milestones</option>
            <option value="group">Subsystem / checkpoint</option>
          </select>
        </label>
        {scope === "group" && (
          <label>
            Subsystem group
            <select
              value={group}
              onChange={(e) => {
                setGroup(e.target.value);
                setSelectedId(e.target.value);
                setLimit(30);
                setStatus("all");
              }}
            >
              {groups.map((g) => (
                <option key={g.id} value={g.id}>
                  {lookup.get(g.id)?.data.label ?? g.id}
                </option>
              ))}
            </select>
          </label>
        )}
        <label>
          Relationship evidence
          <select value={basis} onChange={(e) => setBasis(e.target.value)}>
            <option value="explicit">Verified explicit only</option>
            <option value="all">Include inferred & unknown</option>
            <option value="unknown">Unknown evidence only</option>
          </select>
        </label>
        <label>
          Node status
          <select value={status} onChange={(e) => setStatus(e.target.value)}>
            <option value="all">All statuses</option>
            {[...new Set([...base, ...related].map((n) => n.data.status))]
              .sort()
              .map((s) => (
                <option value={s} key={s}>
                  {statusLabel(s)}
                </option>
              ))}
          </select>
        </label>
        <label>
          Inspect node
          <select
            value={ids.has(selectedId) ? selectedId : ""}
            onChange={(event) => {
              setSelectedId(event.target.value);
              setSelectedEdge(null);
            }}
          >
            <option value="">Select a visible node</option>
            {visible.map((node) => (
              <option key={node.id} value={node.id}>
                {node.data.label}
              </option>
            ))}
          </select>
        </label>
        <label className="checkbox-control">
          <input
            type="checkbox"
            checked={showFunctions}
            onChange={(e) => setShowFunctions(e.target.checked)}
          />{" "}
          Reveal related functions
        </label>
      </div>
      <div className="graph-legend">
        <span>
          <i className="legend-line" />
          Explicit requirement
        </span>
        <span>
          <i className="legend-line inferred" />
          Inferred relationship
        </span>
        <span>
          <i className="legend-line unknown" />
          Evidence unknown
        </span>
        <span className="mono">
          {visible.length} nodes · {sourceEdges.length} relationships
        </span>
      </div>
      <div className="dependency-layout">
        <section
          className="panel graph-panel"
          aria-label="Interactive directed dependency graph"
        >
          {visible.length ? (
            <ReactFlow<VisualNode>
              key={`${scope}:${group}:${status}:${showFunctions}:${limit}`}
              nodes={rfNodes}
              edges={rfEdges}
              nodeTypes={nodeTypes}
              fitView
              fitViewOptions={{ padding: 0.2, maxZoom: 1 }}
              minZoom={0.15}
              maxZoom={1.8}
              nodesDraggable={false}
              nodesConnectable={false}
              deleteKeyCode={null}
              colorMode={dark ? "dark" : "light"}
              onNodeClick={(_, node) => {
                setSelectedId(node.id);
                setSelectedEdge(null);
              }}
              onEdgeClick={(_, edge) => {
                setSelectedEdge(
                  data.graph.edges.find((e) => e.id === edge.id) ?? null,
                );
                setSelectedId("");
              }}
            >
              <Background gap={22} size={1} />
              <Controls showInteractive={false} />
            </ReactFlow>
          ) : (
            <Empty title="No matching graph nodes">
              Select another scope or status. No placeholder dependencies are
              added.
            </Empty>
          )}
          <div className="graph-caption small">
            Prerequisite → dependent · pan to explore, scroll to zoom · select a
            node or edge
          </div>
        </section>
        <aside
          className="panel inspector graph-inspector"
          aria-label="Graph inspection"
        >
          {selectedEdge ? (
            <>
              <p className="eyebrow">Relationship evidence</p>
              <h3>
                {lookup.get(selectedEdge.source)?.data.label}{" "}
                <span className="muted">→</span>{" "}
                {lookup.get(selectedEdge.target)?.data.label}
              </h3>
              <span className="tag">
                {selectedEdge.data.relationship.replaceAll("_", " ")}
              </span>
              <p>
                <strong>
                  {selectedEdge.data.basis === "inferred"
                    ? "Inferred"
                    : "Explicit"}
                </strong>{" "}
                · evidence {selectedEdge.data.evidence_state}
              </p>
              {selectedEdge.data.rationale && (
                <p>{selectedEdge.data.rationale}</p>
              )}
              <EvidenceLinks items={selectedEdge.data.evidence} />
            </>
          ) : selected ? (
            <>
              <p className="eyebrow">{selected.type} inspection</p>
              <h3>{selected.data.label}</h3>
              <Badge value={selected.data.status} />
              <p>
                {plain(
                  selected.data.claim ??
                    selected.data.scope ??
                    selected.data.coverage,
                )}
              </p>
              <h4>Verification levels</h4>
              <div className="node-levels">
                {Object.entries(selected.data.verification_levels).map(
                  ([key, value]) => (
                    <div key={key}>
                      <span>{stageLabels[key] ?? key}</span>
                      <span className="small muted">
                        {statusLabel(value.status)}
                      </span>
                    </div>
                  ),
                )}
              </div>
              <h4>Prerequisites / open dependencies</h4>
              {incoming.length ? (
                <ul className="prerequisite-list">
                  {incoming.map((e) => (
                    <li key={e.id}>
                      <button
                        className="text-button"
                        onClick={() => {
                          setSelectedId(e.source);
                          setSelectedEdge(null);
                        }}
                      >
                        {lookup.get(e.source)?.data.label ?? e.source}
                      </button>
                      <span className="small muted">
                        {statusLabel(lookup.get(e.source)?.data.status)} ·{" "}
                        {e.data.basis} · evidence {e.data.evidence_state}
                      </span>
                    </li>
                  ))}
                </ul>
              ) : (
                <p className="small muted">
                  No explicit prerequisites are listed for this node. Missing
                  call relationships remain unknown.
                </p>
              )}
              {selected.data.limitations && (
                <>
                  <h4>Limitations</h4>
                  <p>{selected.data.limitations}</p>
                </>
              )}
              <h4>Related functions</h4>
              {functions.length ? (
                <div className="function-chips">
                  {functions.slice(0, 20).map((n) => (
                    <a
                      className="tag mono"
                      key={n.id}
                      href={`#/functions?address=${n.data.original_entry}`}
                    >
                      {n.data.original_entry}
                    </a>
                  ))}
                  {functions.length > 20 && (
                    <span className="small muted">
                      +{functions.length - 20} more; use the function group
                      filter
                    </span>
                  )}
                </div>
              ) : (
                <p className="small muted">
                  No function scope is explicitly associated.
                </p>
              )}
              <h4>Source files & evidence</h4>
              <EvidenceLinks items={selected.data.evidence} />
            </>
          ) : (
            <div className="inspector-intro">
              <span className="area-icon">
                <Icon name="graph" size={26} />
              </span>
              <h3>Node inspection</h3>
              <p>
                Select any node for its recorded state, prerequisites, function
                scope and evidence. Select an edge to inspect the relationship's
                provenance.
              </p>
              <div className="inspector-hint">
                <strong>Evidence scope</strong>
                <p>
                  A verified edge means the cited requirement is in the
                  repository. It does not certify original-game equivalence.
                </p>
              </div>
            </div>
          )}
        </aside>
      </div>
      {showFunctions && related.length > limit && (
        <button
          className="button secondary load-more"
          onClick={() => setLimit(Math.min(limit + 30, 150))}
          disabled={limit >= 150}
        >
          Reveal up to 30 more functions ({Math.min(limit, related.length)} /{" "}
          {related.length})
        </button>
      )}
      <details className="panel unknown-relations">
        <summary>
          {data.graph.unknown_relationships.length} address references without
          function nodes
        </summary>
        <p className="small muted">
          Reference-only addresses and ambiguous program associations are
          retained outside the rendered graph.
        </p>
        {data.graph.unknown_relationships.map((u) => (
          <article key={u.id}>
            <span className="mono">
              {u.original_entry ?? "Unknown address"}
            </span>
            <p>{u.reason}</p>
            <EvidenceLinks items={u.evidence} compact />
          </article>
        ))}
      </details>
    </>
  );
}
