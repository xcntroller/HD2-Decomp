# Research explorer verification

Source baseline: master `602197821fb2edc381add17b9974d9ced39f85e7`.
Research checkpoint: `cc4571a7a6b183b00d4ce64ef821cbe16f3fea42`, already in
that baseline. Research implementation snapshot: `52c568964063207b0d15bea8ef98af09f89d84df`.
Work is isolated on `feature/research-dashboard`.

## Passed

- Locked npm installation; TypeScript; Vite production build at `/HD2-Decomp/`;
  existing snapshot validator and extended static-artifact audit. Only declared
  assets, licensed fonts and the two validated public JSON snapshots are emitted.
- All 27 existing function-progress unit tests.
- Temporary adapter checks against actual repository data: deterministic join,
  stable identities, original edge directions and types, explicit unknown targets,
  snapshot separation, stale ledger diagnostics and batch order. Negative checks
  reject duplicate IDs, missing endpoints, calls marked blocking and invalid
  evidence JSON pointers. Every research provenance path/pointer is checked by
  the adapter; the verification record's graph digest must match.
- Temporary headless Chromium checks against the production preview: overview,
  function drill-down/address search, select-based keyboard inspection, roadmap
  navigation, batch contracts, source switching, actual public evidence URLs,
  blocking-edge conditions, 150-node rendering cap, cross-milestone batch reset,
  desktop and 390px mobile layouts, and no uncaught browser exceptions.
- Screenshots inspected. Initial feedback identified misleading data presentation;
  corrections remove false milestone conflicts, use the selected source for graph
  filtering, reset incompatible batch filters, restrict batch membership, clear
  stale inspection selections and fix mobile wrapping. The final targeted browser
  check passed after those corrections.
- No reconstruction, networking, ledger, main plan/handoff or research files changed.
  No original game, DLL, Ghidra, Windows fixture or Proton executed. No publishing.

## Interpretation and remaining review

The one research stale-input diagnostic on this baseline is the older function
ledger digest. Research claims remain pinned rather than silently refreshed.
Only comparable function implementation statuses produce conflict diagnostics;
research milestone `not_delivered` and roadmap `not-started` are separate scoped
assertions, not a manufactured contradiction. The original progress snapshot
retains its own documentary diagnostics.

Batch function associations are explicit address mentions in guide contracts,
not an exhaustive function census. The shared B27 contract remains shared;
address ranges are not expanded into invented recovered functions. External
milestone prerequisites remain in batch records. Conditional dependency edges
are inspectable but do not generate unconditional scheduling ranks. No critical
path, progress percentage, remaining-work estimate or whole-batch implementation
claim is synthesized.

User desktop review accepted the corrected data presentation, branching graphs
and inspector formatting, followed by explicit authorization to commit. This
confirms website interaction review only; it makes no game desktop acceptance
claim. Nothing was pushed or deployed.

## Branching layout correction

Research and batch canvases now use a deterministic directed layout rather than
fixed rows. Conditional dependency edges contribute to visual placement without
becoming unconditional scheduling constraints. Recursive relationships are
collapsed before layering, and disconnected records use a compact grid. Batch
prerequisite arrows include the guide's external P2.3 milestone. Both canvases
fit wide graphs below the former default zoom floor. Research JSON is unchanged.

Temporary checks passed against the actual research graph: dependency direction
in geometry, parallel branches, deterministic placement under reordered inputs,
cycles without runaway ranks, and 150 distinct disconnected-node positions.
Headless Chromium verified branching research and batch views, the external
prerequisite, blocking filtering, mobile width and no runtime errors; screenshots
were inspected. Production build and static-artifact validation passed. The user subsequently confirmed this layout in desktop review.

## Inspector formatting

Inspector arrays render as semantic lists, nested records as labeled fields,
empty lists as “None recorded” and null values as “Unknown”, rather than raw
JSON brackets/braces. Nested evidence uses the existing safe link component and
preserves unpublished availability. The production build/artifact audit and a
temporary Chromium check of actual research arrays, nested verification fields,
evidence availability, absence of JSON blocks, mobile width and runtime errors
passed. This presentation change does not alter source data.

## Research checkpoint 8d30062 compatibility

Preview worktree fast-forwarded to research merge `578c7d6`. The updated graph
contains 310 research nodes and 629 edges. The guide introduces 31 parent/child
batches, nested dotted/hyphenated identities, three-column child tables and
forward references. The importer now accepts both supported table forms,
validates full IDs and the prerequisite DAG independently of table row order,
retains full prerequisite/owner wording, and uses explicit graph `batch_ids`
associations where supplied. Batches without a dedicated heading retain their
table summary and tracked links to associated research contracts. Conditional
“where actually called” batch edges remain visually distinguished.

Production build/artifact audit and a temporary data/browser check passed:
all research nodes/edges retained, every guide batch imported, exact declared
node associations, full child prerequisite IDs, child-batch selection, formatted
inspection, 390px mobile width and no runtime errors. Research files unchanged.
The user accepted the updated preview and explicitly authorized committing these
compatibility changes. Nothing was pushed or deployed.

## Named checkpoint receipts and historical evidence

Reproduced the Pages failure against merged master `3b8cd19`. Its current graph
is 369 nodes/774 edges, SHA-256
`372de641d0af085499a0923d6dc91f778a4b882b037c34ae1b3c54b20230bb0f`.
The legacy receipt describes the earlier 320-node/659-edge graph; the matching
receipt already exists in `evidence/p24-verification.json`. Website import now
selects tracked receipts by exact graph digest, checks counts/source SHA and
preserves the receipt's own verification scope. The overview links that receipt.
No research files or receipts were changed, and the digest check remains strict.

Research `revision` references now read regular Git blobs at the exact recorded
commit for input hashing and JSON-pointer validation. Evidence/source-mention
links use the same revision, subject to the existing public-ancestor boundary.
This avoids applying pinned ledger pointers to a newer ledger or silently
substituting current source for historical evidence.

Passed: production build with `GITHUB_ACTIONS=true`, current `GITHUB_SHA` and
matching `HD2_PUBLIC_REF`, TypeScript and static-artifact audit; all 27 existing
analyzer tests; temporary negative receipt tests for wrong digest, counts, source,
schema, missing checks and untracked receipts; historical input hashes, links,
pointers and invalid revision rejection. A temporary Chromium check passed for
the selected P2.4 receipt/readiness, all 369 research nodes and 774 edges, pinned
source URLs, P2.4 graph/batch views, formatted inspection, mobile width and runtime
errors. Temporary audit sources removed. No game, DLL, Ghidra or Proton executed.
The user reviewed the result and explicitly authorized committing the fix.
Nothing was pushed or deployed.

## Combined P2.8 receipt compatibility

Reproduced against merged master `256f2fe`. The combined graph contains 596
research nodes and 1,751 edges, SHA-256
`8e016b11ecb8dac6dca923b5d3dc5c86593361ff6d744506e9028c408f241caa`.
Its matching receipt already exists in
`analysis/multiplayer-world/evidence/verification.json`; it uses explicit
`combined_*` fields and `historical_combined_claim_sha`. Earlier receipt discovery
was restricted to the future-dependencies evidence directory and the single-graph
shape. No graph or receipt data was modified to resolve the mismatch.

Discovery now follows graph-input evidence directories. Matching validates the
selected graph role's exact digest, node/edge counts, implementation-claim SHA
and recorded checks, and retains the original scoped receipt. The UI derives
milestones from graph records so the new P2.8 research can be reached.

Passed: Pages-equivalent production build with the exact workflow SHA/public ref,
TypeScript and static-artifact audit; all 27 existing analyzer tests; temporary
regressions for original, P2.4 and current combined receipts; rejection of wrong
hash/count/source, missing fields/checks, focused-only certification and unrelated
receipt directories. Temporary Chromium checks passed for the combined receipt
link, 596/1,751 record preservation, P2.8 overview/filter/drill-down, bounded
canvas, prior milestone navigation, formatted inspection, 390px mobile width and
no uncaught errors. Temporary audit sources removed. No original game, DLL,
Ghidra or Proton executed. The user explicitly authorized committing the verified
fix. Nothing was pushed or deployed.
