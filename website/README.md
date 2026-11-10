# Hidden & Dangerous 2 reconstruction dashboard

A static, read-only React + TypeScript + Vite site backed by the progress analyzer.
Its game-menu navigation and record panels use original CSS/SVG and self-hosted open-source fonts;
no commercial artwork, game files, extracted assets or original engine libraries
are distributed. The menu opens in dark mode; a light theme is also available. In the section menu, Up/Down move focus, Enter opens a section, Home/End select the first/last entry, and Escape returns to Overview. Normal Tab navigation and pointer controls remain available.

## Run locally

Requirements: Node.js 24 LTS (see `.nvmrc`), npm, Python 3.10+, and Git.
Run from this repository's `website/` directory:

```sh
npm ci
npm run dev
```

Open `http://127.0.0.1:5173/HD2-Decomp/` (or the port Vite prints).
`npm run dev` first regenerates the public snapshot from the current worktree.
To regenerate it independently:

```sh
npm run data
```

Production verification and preview:

```sh
npm run build
npm run preview
```

`npm run build` regenerates data, validates the exact public schema and claims
against the current analyzer/inputs, runs strict TypeScript checking, builds
Vite's static output, then audits **all** output files against the Vite asset
manifest. Unsafe paths, symlinks, raw exports, logs and undeclared files fail
the build. Validate an already generated snapshot with `npm run validate:data`.
Source/commit URLs in the JSON must match the fixed GitHub repository and an
immutable commit ID. Generated data, dependencies and `dist/`
are ignored by Git. The npm lockfile fixes dependency versions. No database,
authentication, runtime API, external font request or analytics is used.

## Views

- **Overview:** current milestone, project areas, whole/partial/uncredited function
  counts, source receipts, and five separate verification dimensions.
- **Roadmap:** the Phase 2 machine index plus Phase 1, Phase 3 and postponed
  checklists from the analyzer. Requirements, acceptance scope, dependencies and
  evidence expand inline. Only a reported blocked state is shown as blocked;
  unresolved work does not synthesize one.
- **Dependency Explorer:** React Flow, with milestone, first-frame and subsystem/
  checkpoint scopes; pan/zoom; evidence/status filters; node and edge inspection.
  Function nodes are hidden initially, revealed in batches of 30, capped at 150.
  Dependency arrows run prerequisite → dependent. Grouping/address associations
  are different edge roles. Inferred edges are dashed, unknown evidence dotted.
  Unknown address endpoints remain a separate list, not fabricated graph nodes.
- **Function Explorer:** original-address search (including `0x` prefixes), program,
  reconstruction-status and documented-group filters, with 25 rows per page.
  Inspection exposes coverage, exclusions, source mappings and evidence. Receipt
  address matches do not establish per-function test results or call edges.
- **Development Activity:** the latest 30 commits in this worktree's history and
  the checked-in subsystem receipts. Commit history and verification are separate
  views; same-day receipt order is not asserted. Local commits have no public
  commit link until the published reference contains them.

Navigation uses URL hashes and Vite uses the explicit `/HD2-Decomp/` base path.
Deep links such as `#/functions?address=00416e10` work without server rewrite rules.

## Data and publication boundary

The original analyzer is unchanged. `scripts/export_public.py` imports its
`analyze()` result as the primary source and writes `public/data/progress.json`.
The adapter selects display fields rather than copying the raw report:

- Function counts, claims, milestones, requirements, graph identities, relationship
  bases and receipt scopes come from the analyzer. Dates are repository dates.
- Git supplies a snapshot SHA and committed-change history, without author emails
  or local checkout names. These are not claims of verified functionality.
- The client area's state follows the active machine-index milestone. Server
  planning is derived from the unchecked Phase 3 requirements, excluding postponed
  backlog items. The engine-replacement scope statement is matched literally in
  `plan.md`; absent or changed evidence produces unknown scope. Original LS3D
  ledger counts are never assigned to Engine Replacement.
- Local log fields and log bodies are omitted. Private/absolute artifact references
  in prose are redacted. Evidence links are restricted to tracked source/research
  text, with no commercial extensions, ignored directories or symlink targets.
- Source URLs are pinned to the branch's upstream commit by default. A link is
  emitted only when that published tree contains the file and its blob matches
  the current working bytes. Missing/unpublished evidence remains labeled and
  unlinked. Relationship provenance includes safe source locations/JSON pointers;
  input hashes retain the analyzer's metadata provenance.

For a known published build, set `HD2_PUBLIC_REF` to its public commit/ref:

```sh
HD2_PUBLIC_REF=origin/tooling/progress-analyzer npm run build
```

The adapter does not fetch or push. The default upstream is local Git metadata;
keep it current deliberately. `HD2_PUBLIC_REF` is an assertion by the builder
that the selected ref is public; it must never point to an unpublished/private
revision. With no upstream, public source/commit links are omitted. Builds are
deterministic for the same input files, Git history, HEAD and published ref.

`src/presentation.json` contains only names, navigation and presentation labels.
Typography, map decoration and dossier styling are CSS/SVG presentation. They
carry no progress claims, dates, dependency edges or completion percentages.
The graph's visual positions are layout aids, not original call-order evidence.

## Available data and limits

The initial source snapshot reports P2.2 in progress, 34 complete and 1,651 partial
functions against an estimated frozen denominator of 18,783. The remaining
17,098 entries have no ledger claim. The public metadata can name only 31 extra
uncredited references; it does not contain a complete list of those 17,098 entries.
No fractional credit or game-completion percentage is computed.

There are nine checked-in source checkpoint receipts. Native/i386 reports keep
fixture boundaries; they do not establish a loaded mission or desktop acceptance.
Dedicated Server is planned, while Engine Replacement is separate from the current
scope with no replacement-progress census. No active/completed progress is claimed
for either area. Function-level call relationships are not available. The adapter
preserves the analyzer's 15 stale-reference notices without changing those records.

Each build describes its analyzed Git revision. Once the Pages workflow is
enabled on `master`, every master push rebuilds the snapshot from that exact
revision. Uncommitted changes in another worktree are not a live data source.
A missing or unsupported JSON file has a retry/error view. Absent collections and
counts show empty or unknown states rather than fabricated progress.

## GitHub Pages deployment

[Deployment setup, exact instructions and troubleshooting](DEPLOYMENT.md) covers
`.github/workflows/pages.yml`. It builds on every push to `master` and supports
manual dispatch on `master`. Only `website/dist/` is uploaded using the official
Pages actions, with deployment permissions limited to a separate deploy job.
No custom secrets or PAT are needed.

An administrator must first select **Settings → Pages → GitHub Actions** and
allow `master` in the `github-pages` environment. Integrating this workflow onto
`master` with Pages enabled starts automatic deployment. Nothing is pushed,
merged, deployed or changed in repository settings by a local build.

Expected public URL: https://xcntroller.github.io/HD2-Decomp/

Do not publish the repository or raw analyzer output. Vite's `.vite/manifest.json`
is used for the local artifact audit; the official Pages uploader excludes hidden
files. The published files are the built HTML/JS/CSS, licensed fonts, font notices
and validated JSON. No source maps are produced. Hash routing requires no 404
fallback. `npm run preview` serves the same `/HD2-Decomp/` prefix locally.

Font licenses are distributed as `font-licenses.txt` in the static output.

## Verification of the first version

- Clean `npm ci --offline` from the lockfile, TypeScript checking, Vite production
  build and public-artifact audit passed.
- 44 temporary headless Chromium checks passed: navigation, keyboard menu controls,
  dark/light persistence, roadmap scope, address/program/status/group filters,
  pagination, node/edge inspection, graph disclosure limits, mobile/tablet layouts,
  missing JSON, retry and incomplete-snapshot states. No uncaught browser errors.
- Four more browser checks passed at `/HD2-Decomp/`: address deep links, reload,
  lazy React Flow assets and error-free page loading. Screenshots were inspected.
  Four targeted checks also passed for desktop menu label bounds, mobile node
  selection without canvas clicking, and edge-provenance inspection (52 browser
  checks in total).
- 16 temporary publication-adapter checks and all 27 existing function-progress
  tests passed. Projection counts/claims, relationship bases, unknown references,
  input hashes, deterministic generation, private-path removal, URL safety and
  unpublished-blob handling were checked.

Temporary audit sources are removed after verification, following the repository
agreement against new permanent tests. No game, engine DLL, Windows fixture or
Proton was executed. Browser checks establish website behavior, not game desktop
acceptance. Reconstruction code, ledger, plan, handoff and analyzer are unchanged.

## Research intelligence

`#/research` opens the P2.4–P2.7 briefing, a bounded React Flow explorer and
ordered implementation-batch board. Research and current implementation are
separate snapshots on shared stable IDs. The overview and inspector expose
blockers, original addresses, lifetime/ordering constraints, verification scope,
source coverage and safe evidence links. Select a subsystem, search an address,
or explicitly request function nodes to drill down. The canvas starts at 40
nodes and caps at 150; narrower filters reach further records. The keyboard
inspection selector includes both visible nodes and relationships. Incoming and
outgoing relationships can also be opened from the inspector, even outside the
current canvas. Source, milestone, subsystem, batch, type, evidence, relationship
and implementation-state filters keep disclosure bounded.

`website/scripts/research_adapter.py` reads the committed future-dependencies
export without running its generator (which pins historical implementation
claims). It regenerates current progress using the existing analyzer, joins by
stable namespaced identity, retains both sources on shared nodes/edges and
reports stale input digests and state disagreements. Calls remain caller→callee;
dependencies remain prerequisite→dependent. Ownership and cleanup edges retain
separate types. Visual layout follows directed dependency edges, including
conditional dependencies. If no dependency edges are visible, it follows the
selected relationship topology instead. Cycles are grouped before layering;
isolated records use a compact grid. Positions do not imply scheduling or a
critical path. Scheduling validation remains limited to verified, unconditional
blocking dependencies. Conditions remain in the inspector. The batch graph uses
actual prerequisite branches from the guide and includes external milestone
prerequisites; its positions carry no additional ordering claims. No estimate is
invented.

The batch contract includes purpose, original entries, failure handling and
focused/desktop acceptance. Contract-ready means bounded research readiness;
no whole-batch implementation claim is synthesized. The original implementation
roadmap and dependency explorer are preserved and linked from Research.

### Incorporating research checkpoints

Use ordinary reviewed Git integration of the research commit into the deployment
source branch. Do not fetch a moving branch from Pages. Compare the research ref
read-only first, then merge/cherry-pick the agreed checkpoint through the usual
repository process. This implementation was based on local master `6021978`;
inspection confirmed checkpoint `cc4571a` is already its ancestor, and the
research tree matches that branch unchanged. Its implementation snapshot is
still pinned to `52c5689`. No research findings, ledger, reconstruction source,
main plan or handoff are edited by the website adapter.

`npm run build` exports both snapshots, validates evidence paths and JSON
pointers, unique identities, endpoints, edge classifications, batch ordering,
research verification digest and the scheduling DAG, then builds and audits the
static artifact. `research.json` must exactly match a fresh deterministic join
at artifact audit time. Missing versioned research produces an explicit
unavailable view; malformed present research fails the build. Current-input
mismatches are retained as diagnostics, not silently promoted claims. Public
links use the existing publication policy: only safe tracked files matching the
asserted public revision receive immutable GitHub links. Unpublished records
remain visible without a fabricated link. Pages still builds only exact master
revisions, uses `/HD2-Decomp/`, and uploads only the audited `website/dist/`.

The batch importer supports both parent four-column and child three-column guide
tables. Dotted and hyphenated child IDs are preserved in full. Dependencies may
reference batches later in the guide; validation checks the complete prerequisite
DAG rather than assuming table order. Full owner/conditional prerequisite wording
is displayed alongside the graph. When the research graph supplies `batch_ids`,
those declared associations take precedence over address-mention fallback matching.
A batch without its own heading retains its table summary and links to associated
research contracts; the website does not invent missing acceptance requirements.
