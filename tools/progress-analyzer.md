# Read-only reconstruction progress analyzer

From the repository root, using Python 3.10 or later:

```sh
python tools/progress_analyzer.py > /tmp/hd2-progress.json
python -m json.tool /tmp/hd2-progress.json > /dev/null
```

`--root /path/to/checkout` reads another checkout or exported source tree.
The CLI only reads files and emits JSON to stdout. It does not update the ledger,
README, plan, handoff or implementation; invoke builds, Git, Ghidra or network
services; or run game binaries, DLLs, Windows fixtures or Proton. Imports do not
write Python bytecode caches. Shell redirection writes only the destination you
choose. No third-party packages, commercial files or ignored exports are required.

The existing `function_progress.py` remains the authoritative full local census
validator. Its `--check` needs ignored inventories in
`analysis/work/function-progress/`; this analyzer deliberately does not read them.
Other existing validators, such as `validate_mission_sample.py` and
`validate_engine_host.py`, verify assets, local exports or binary instruction
anchors. Their existence or a recorded PASS does not establish a rendered mission.

## Output contract (schema version 1)

- `inputs`: repository-relative paths and SHA-256 of the exact input bytes used.
  No time, absolute checkout path or volatile Git metadata enters the output.
  Identical inputs produce identical, sorted-key JSON with a trailing newline.
- `functions`: frozen baseline, exact complete/partial counts per program and
  total, and all ledger entries with original addresses, coverage, limitations,
  verification text, evidence links and JSON pointers to their source rows.
  `without_ledger_claim` is denominator minus claimed entries; it means no ledger
  claim, not unresearched or unimplemented. Partial functions earn no fractional
  credit. No percentage, effort, playability or time estimate is computed.
- `checklist`: literal checkboxes from `plan.md`, including continuation lines,
  source-only wording, section ancestry and line locations. Counts include the
  historical and postponed backlog. They are not a completion metric. Checked
  items retain the plan's scope and exclusions; no desktop status is synthesized.
- `milestones`: current reported status and acceptance from the Phase 2 machine
  index, its explicit prerequisite edges and original function seeds joined to
  current ledger status. Reference addresses remain separate from credited
  entries. The index's cached ledger digest and cached reference statuses are
  compared with the ledger and discrepancies appear in `diagnostics`.
- `checkpoints`: all JSON receipts in `analysis/checkpoints/`, retaining scope,
  original entries, source-only/desktop flags (null when absent) and verbatim
  verification reports. `observations` maps recognized verification keys to the
  native, i386 or desktop dimension without interpreting prose as a test result.
  Unrecognized keys remain in `reported_verification`. Application compile/link
  reports are compilation evidence; they do not establish runtime integration.
- `first_frame`: a bounded dependency graph for P2.2, described below.
- `graph`: unified nodes, directed edges and groups for a future static React
  Flow frontend; no website or layout engine is included.
- `diagnostics`: stale cross-references and missing/changed evidence anchors.
- `epistemic_policy`: machine-readable definitions of evidence strength and scope.

The frozen program hashes identify the scope being reported. This tool checks
metadata shapes, identities, duplicate entries, reconstruction-claim rules,
counts and interval ordering. It does **not** independently verify binary hashes,
inventory membership, instruction bounds, evidence-link contents or equivalence.
Per-function `inventory_membership` is therefore `unknown`. Native/i386 passes,
integration and desktop acceptance cannot be assigned to individual functions
merely from a shared receipt or ledger evidence link.

## Website graph records

`graph.nodes` uses `{id, type, data}` records with `type` equal to `milestone`,
`subsystem`, or `function`. IDs are namespaced identities:

- `milestone:P2.2` (machine-index milestone ID).
- `subsystem:world_composition` (catalog ID, independent of display title/status).
- `subsystem:checkpoint:p2.2-script-receivers-20261109` (receipt filename stem).
- `function:client:00416e10` (program and original address, never a source filename).

Keep catalog IDs and receipt filenames stable when revising descriptions. A
renamed receipt is a new graph identity. Each node's `data` contains its label,
reported status, provenance, and `verification_levels` keyed by all five stages.
Unestablished levels remain `unknown`. Function nodes expose reconstruction
coverage/limitations and evidence links. Functions referenced by plans but absent
from the ledger are represented as `no_ledger_claim`; they add **no** credit.
Milestone status is retained verbatim; this adapter does not translate closure
of an unloaded or source-only checkpoint into loaded-world delivery.

`graph.edges` uses `{id, source, target, type, data}` records. Dependency edges
have `type: dependency`, `data.relationship: depends_on`, and point from
**prerequisite to dependent**. This reverses the human-readable
`first_frame.dependencies` endpoint names intentionally. Other edges use
`type: relationship` and these explicit roles:

| Relationship | Direction and meaning |
| --- | --- |
| `contains_subsystem` | Milestone → catalog subsystem; catalog grouping |
| `original_function_seed` | Milestone → original function seed; machine-index scope |
| `original_address_reference` | Milestone → reference address; not reconstruction credit |
| `original_function_reference` | Catalog subsystem → documented function reference |
| `checkpoint_function_scope` | Receipt subsystem → function mentioned in receipt |

Each edge carries `data.basis` (`explicit` or `inferred`), `evidence_state`
(`verified` or `unknown`), and `provenance` containing repository-relative paths
with JSON pointers or exact quotes/line numbers. Inferred edges carry a rationale.
Receipt address associations are labeled inferred because receipt entry lists
do not include program IDs: the adapter joins only addresses with a unique known
program-qualified node. This is a scope association, never a per-function PASS
or a call edge. Ambiguous/unresolved associations remain in
`graph.unknown_relationships` with a stable ID, null target, provenance and reason;
no invented endpoint or dependency is emitted.

Addresses explicitly marked `reference-only-no-standalone-counted-entry` in the
phase index also remain in `unknown_relationships`, with `basis: explicit` for
the documented address reference and a null function target. They are preserved
without inventing standalone function nodes or granting credit.

Edge IDs are SHA-256 of the canonical `[source, target, relationship]` tuple.
They survive status, title, evidence and input-order changes. `graph.groups`
contains group IDs, member IDs and relationship IDs, so a frontend can construct
compound nodes, filters or shared memberships without interpreting filenames.
A function can belong to multiple groups. Groups themselves imply no execution
dependency. JSON pointers are RFC 6901 locations within the hashed input; source
links are repository-relative for the frontend to attach its own repository URL.

No node positions are generated. A static frontend can lay out the normalized
records and map the three node types to components. It should style grouping,
inferred and evidence-unknown edges separately, and traverse only verified,
explicit dependency edges by default. `graph.unknown_call_graph` makes the missing
function-level call graph explicit. Unknown relationships are not silently filled
using shared evidence files, seed order or adjacent checklist entries.

## First-frame dependencies and evidence

`progress-dependencies.json` is a small, reviewed catalog of required loading,
actor/collision, script/runtime-input, containing-world, activation and camera
work. Each node has one of five stages: `source_recovery`, `native_verification`,
`i386_compilation`, `integration`, or `desktop_acceptance`. No sequence between
these stages is synthesized. Native and i386 whole-loading gates remain unknown;
scoped fixture and application-build passes are exposed in checkpoint observations.

Each catalog claim and edge cites exact, unique text from a repository contract.
The analyzer resolves those anchors to line numbers and records input hashes.
`evidence_state: verified` means the quoted statement exists, not that this run
reproduced its verification or certified fidelity to the original game. Catalog
statuses are scoped documentary claims: `unresolved`, `supported_subset`,
`recorded_pass`, or `unknown`; the current catalog makes no whole-gate PASS claim.
An unknown gate is an evidence gap, not a reported failure.

Edges point from `dependent` to `prerequisite`. Their `kind` distinguishes runtime
requirements from validation requirements. They are not all original call edges
or statements about temporal order. `basis: explicit` is reserved for requirements
stated in the cited contract. The activation-to-first-Human-frame ordering is a
reviewer inference with its rationale attached. Inferred edges are excluded from
`explicit_dependency_closures`, `prerequisites_first` and
`unresolved_dependencies`. The full edge list still exposes them for a website
that displays inferred relationships separately.

`render_target` is the first actual controller-owned world frame.
`delivery_target` additionally requires user desktop acceptance and the documented
validation gates. `unresolved_dependencies` lists edges whose prerequisites are
unresolved, unknown or only a supported subset. It does not mean development is
blocked on external input. No timed critical path, exhaustive call graph, or
complete dependency inventory is claimed. Phase 2 milestone prerequisites remain
in `milestones`, including the accepted unloaded P2.1 boundary.

An edited, missing or ambiguous quote changes its node status to `unknown` and
excludes affected edges from explicit closure traversal. `catalog_claim` retains
the previous claim for review; consumers must display `status`, and check
`evidence_state` before using edges. Review diagnostics before relying on a
closure: changed evidence may remove prerequisites from that bounded view.
Malformed mandatory metadata, unsafe input paths, duplicate JSON keys and graph
cycles fail with exit 1 and a JSON error on stderr; stdout is empty. Documentary
drift produces a usable report with diagnostics and exit 0.

## Maintenance and limits

The analyzer reads the current checkout, independently of the reconstruction
session's other worktree. It is not a live feed from that session. Update this
branch's source snapshot deliberately to analyze later work. It assumes input
files remain stable during a run; the input hashes identify the bytes read, not
an atomic multi-file Git snapshot.

New ledger rows, plan checkboxes, milestones and checkpoint receipts are extracted
automatically. New receipts never automatically close a catalog node. When a
contract changes, review the entire affected boundary and update catalog claims
and anchors together. Keep unresolved consumers (including SOUND, remaining
actor/physics/objective work) visible; this initial graph groups some of them
under the full loading/world boundary instead of claiming a fully recovered
internal order. Do not turn absence of a ledger claim or receipt into failure.

All default inputs are checked-in metadata/contracts at the time of introduction;
the checkpoint directory is scanned for JSON receipts, including any local
additions. The analyzer reads only `.md`/`.json` under the permitted metadata
locations and rejects `analysis/work`, `analysis/ghidra`, traversal and escaping
symlinks. It does not follow receipt log paths or ledger evidence links into
assets, implementation files or ignored work directories. Every consumed metadata file,
including the catalog, appears in `inputs`.

Validation uses the existing function tracker tests and temporary synthetic
audits, following the repository agreement against new permanent tests. There
is no desktop behavior change or claim of desktop verification for this tooling.

Validation at introduction: 33 temporary analyzer checks and all 27 existing
`test_function_progress.py` tests passed. The temporary checks covered a
metadata-only source export, byte-identical repeated CLI output, unchanged input
files/no bytecode writes, ledger invariants, stale references, evidence drift,
cycles, unsafe paths, graph endpoint/provenance integrity, stable IDs,
inferred/unknown relationships and receipt scope isolation. Temporary audit
sources were removed. Run the retained tracker suite with:

```sh
python -B -m unittest discover -s tools -p 'test_function_progress.py'
```

The introduction snapshot extracts 34 complete and 1,651 partial ledger entries
against the frozen 18,783 denominator. It reports one stale Phase 2 ledger digest
and 14 stale cached reference statuses; it does not repair those source records.
