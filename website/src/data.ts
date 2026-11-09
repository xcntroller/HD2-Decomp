export type Evidence = {
  path: string;
  url: string | null;
  line?: number | null;
  pointer?: string | null;
  availability?: string;
};
export type Level = {
  status: string;
  scope?: string;
  evidence: Evidence[];
  reports?: unknown[];
};
export type GraphNode = {
  id: string;
  type: "milestone" | "subsystem" | "function";
  data: {
    label: string;
    status: string;
    claim?: string;
    coverage?: string;
    limitations?: string;
    scope?: string;
    program?: string;
    original_entry?: string;
    date?: string;
    evidence_state: string;
    evidence: Evidence[];
    verification_levels: Record<string, Level>;
  };
};
export type GraphEdge = {
  id: string;
  source: string;
  target: string;
  type: string;
  data: {
    relationship: string;
    basis: string;
    evidence_state: string;
    rationale?: string;
    evidence: Evidence[];
  };
};
export type Group = {
  id: string;
  member_ids: string[];
  relationship_ids: string[];
};
export type FunctionRecord = {
  id: string;
  program: string;
  entry: string;
  status: string;
  coverage: string;
  limitations: string;
  verification?: string;
  evidence: Evidence[];
};
export type Requirement = {
  id: string;
  line: number;
  checked: boolean;
  explicit_source_only: boolean;
  headings: string[];
  text: string;
  evidence: Evidence[];
};
export type Milestone = {
  id: string;
  title: string;
  reported_status: string;
  acceptance: string;
  prerequisites: string[];
  requirements: string[];
  evidence: Evidence[];
};
export type Checkpoint = {
  id: string;
  title: string;
  date: string;
  scope: string;
  source_only: boolean | null;
  desktop_verified: boolean | null;
  evidence: Evidence[];
  observations: { stage: string; reported: unknown; evidence: Evidence[] }[];
};
export type Counts = {
  complete?: number;
  partial?: number;
  frozen_internal_non_thunk?: number;
  without_ledger_claim?: number;
};
export type Dataset = {
  schema_version: number;
  snapshot: {
    source_commit: string;
    published_commit: string | null;
    repository: string;
    latest_receipt_date: string | null;
  };
  areas: {
    id: string;
    name: string;
    status: string;
    milestone: string | null;
    evidence: Evidence[];
  }[];
  active_milestone: string;
  counts: Counts;
  by_program: Record<string, Counts>;
  functions: FunctionRecord[];
  milestones: Milestone[];
  checklist: Requirement[];
  checkpoints: Checkpoint[];
  commits: {
    sha: string;
    date: string;
    subject: string;
    url: string | null;
    status: string;
    publication: string;
  }[];
  graph: {
    nodes: GraphNode[];
    edges: GraphEdge[];
    groups: Group[];
    unknown_relationships: {
      id: string;
      source: string;
      target: null;
      original_entry?: string;
      basis: string;
      reason: string;
      evidence: Evidence[];
    }[];
  };
  diagnostics: Record<string, number>;
  inputs: { path: string; sha256: string }[];
};

export function normalize(input: unknown): Dataset {
  if (
    !input ||
    typeof input !== "object" ||
    (input as Dataset).schema_version !== 1
  )
    throw new Error("This snapshot uses an unsupported data format.");
  const value = input as Partial<Dataset>;
  const array = <T>(v: T[] | undefined): T[] => (Array.isArray(v) ? v : []);
  return {
    schema_version: 1,
    snapshot: value.snapshot ?? {
      source_commit: "",
      published_commit: null,
      repository: "",
      latest_receipt_date: null,
    },
    areas: array(value.areas),
    active_milestone: value.active_milestone ?? "Unknown",
    counts: value.counts ?? {},
    by_program: value.by_program ?? {},
    functions: array(value.functions),
    milestones: array(value.milestones),
    checklist: array(value.checklist),
    checkpoints: array(value.checkpoints),
    commits: array(value.commits),
    graph: {
      nodes: array(value.graph?.nodes),
      edges: array(value.graph?.edges),
      groups: array(value.graph?.groups),
      unknown_relationships: array(value.graph?.unknown_relationships),
    },
    diagnostics: value.diagnostics ?? {},
    inputs: array(value.inputs),
  };
}
export const number = (v?: number) =>
  typeof v === "number" ? v.toLocaleString("en-US") : "—";
export const plain = (v?: string) => (v || "").replace(/\*\*|`/g, "");
export const dateLabel = (v?: string | null) =>
  v && !Number.isNaN(Date.parse(v))
    ? new Intl.DateTimeFormat("en-GB", {
        day: "numeric",
        month: "short",
        year: "numeric",
        timeZone: "UTC",
      }).format(new Date(v))
    : "Date not recorded";
export function safeLink(url?: string | null): string | undefined {
  if (
    !url ||
    !/^https:\/\/github\.com\/xcntroller\/HD2-Decomp\/(commit\/[0-9a-f]{40}|blob\/[0-9a-f]{40}\/(?:[\w.-]+\/)*[\w.-]+(?:#L[1-9][0-9]*)?)$/.test(
      url,
    )
  )
    return undefined;
  if (url.split("/").includes("..")) return undefined;
  return url;
}
export function statusLabel(value?: string): string {
  const labels: Record<string, string> = {
    "in-progress": "In progress",
    "not-started": "Future",
    planned: "Planned",
    separate_scope: "Separate scope",
    "complete-source-only": "Complete · source only",
    "complete-unloaded-construction-user-accepted": "Accepted · unloaded owner",
    partial: "Partial",
    complete: "Complete function",
    no_ledger_claim: "Uncredited",
    unknown: "Unknown",
    unresolved: "Unresolved",
    supported_subset: "Source subset",
    recorded_pass: "Recorded pass",
    recorded_checkpoint: "Source checkpoint",
    reported: "Scoped report",
    source_only_checkpoint: "Source only",
    reported_not_verified: "Not desktop verified",
    reported_verified: "Reported verified",
  };
  return value ? (labels[value] ?? value.replaceAll("_", " ")) : "Unknown";
}
export function tone(value?: string) {
  if (
    value === "complete" ||
    value?.startsWith("complete-") ||
    value === "recorded_pass"
  )
    return "green";
  if (
    value === "in-progress" ||
    value === "partial" ||
    value === "supported_subset"
  )
    return "amber";
  if (value?.includes("blocked")) return "red";
  return "muted";
}
export function listedFunctions(data: Dataset): FunctionRecord[] {
  const claimed = new Set(data.functions.map((f) => f.id));
  return [
    ...data.functions,
    ...data.graph.nodes
      .filter(
        (n) =>
          n.type === "function" &&
          !claimed.has(n.id) &&
          n.data.original_entry &&
          n.data.program,
      )
      .map((n) => ({
        id: n.id,
        entry: n.data.original_entry!,
        program: n.data.program!,
        status: "no_ledger_claim",
        coverage: "",
        limitations: "",
        evidence: n.data.evidence,
      })),
  ].sort(
    (a, b) =>
      a.program.localeCompare(b.program) || a.entry.localeCompare(b.entry),
  );
}
