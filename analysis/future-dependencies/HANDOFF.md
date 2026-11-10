# Forward research handoff — 2026-11-10, deep persistence checkpoint

Branch `research/future-dependencies`. Implementation claims remain pinned to
`52c568964063207b0d15bea8ef98af09f89d84df`; no main source, ledger, root plan/
handoff, analyzer, website, networking or other worktree changes. Local research
only; no original EXE/DLL/Windows fixture/Proton run, desktop claim, push or merge.

Use [implementation guide](implementation-guide.md), [actor contracts](p2.6-actor-persistence.md),
[context layout](p2.6-context-layout.md), [module classification](p2.6-module-persistence.md)
and [results](p2.7-results.md). This is substantially more complete research,
not a declaration of compatible whole-world save/load or P2.7 delivery.

Latest evidence:250 fingerprinted static exports (186 added),17 concrete actor
pairs,7 Human B0 state pairs,4 item subtype pairs,18 registered modules classified.
[Manifest](evidence/manifest.json) retains prior export indices and lists failed/
excluded probes. Heavy Ghidra ran sequentially readOnly/noanalysis, transient
function definitions discarded. Full exports remain ignored, no commercial
binary/asset/disassembly dumps tracked. [Verification](evidence/verification.json)
records focused provenance/vtable/format/graph checks. Graph310 nodes/629 edges;
source recovery, research readiness and implementation readiness remain separate.

Important resolved/corrected contracts:

- Mission-slot IDs and runtime IDs differ; actor14 is an active-list index.
 00475D50 can return a still-owned retired reserved actor;00475DD0 is physical
 slot lookup only. Existing source's ordinary-lookup comment is misleading;
 do not copy it as a retired-lookup persistence contract.
- Module12 recreates only actual type11/2 branches;35 resolves existing actor
 created by physics004794D0. Its vtable00812750 pairs005E2630/005E2730, vec3 motion
 only; real borrowed physics90/005E2510 receiver required. No generic factory.
 Module13 is positional, with no per-actor type/ID.00476A50 recomputes Human
 counters/selection;006E0890/0B70 streams proximity entries/speeds/bucket pairs,
 not a deferred-reference table. NullActor33 delegates base, not no-op.
- Human1001 retires/recreates B0 before1003;1002 receiver is inline98,
 1004 pointed B4 (instruction-confirmed). Mounted target identities already
 exist. Car packed version10 remains the largest actor-format gap.
- Four actual item44/48 pairs and nested ownership now mapped. Common writer
 emits102/103 but reader recognizes202/203; jump table/instructions confirm
 this asymmetry. Preserve it explicitly. Items007DC6E0 is a borrowed index,
 not owner transfer; partial nested failure can leave a registered prefix.
- Script4 reads BEFORE Team11/runtime12/Actors13. Loaded-world identity and
 frame availability precede Script, not actor payload replay.2700 is compiled
 Whenever period/elapsed/enabled,2500 Gosub returns,2450 frame-slot names.
 Scalars now map to actual PC/signals/Delay/flags. Preserve confirmed0x400 current
 globals. Pending animation receiver, rotations and derived contexts remain.
- Mode/store/current aggregate and frontend006B7410/last path006B7D30 mapped;
 stock quantity follows the closed index chunk. No ignored return proves success.
- Offline result→transition→shutdown extracts SAME Human/squad before world
 destruction. Debrief activation0068CCF0/0068CB70 produces ranks/medals; accept/
 leave0068C300 commits them. Exact rank thresholds, medal precedence/uniqueness,
 consumed-delta clearing and nine-draw redistribution recovered. One zero-mean
 update still draws. Whole lifecycle is not idempotent merely because deltas clear.

Ready bounded source work: identity registry/proximity/positional dispatch,
mode-stock/current globals, base/item payload formats, context scalar/reference/
Gosub/Whenever format and result arithmetic. Mandatory owners and transitive
requirements remain in child batches; live/quick/restart/frontend commands are
NOT ready to enable wholesale. Preserve module order, partial mutation and real
cleanup; no successful placeholder, idle context or empty inventory substitute.

Next research in priority order:

1. Car004F7650/004F8510 exact variable solver layout (size004F7300), part/seat
 leaves; Human B0 command/AI/input plus inline98 animation leaves. Reuse recovered
 outer streams and item payloads, do not repeat dispatch discovery.
2. Actual pending animation2600 tick receiver; Script rotation/frame animation
 persistence and derived context kind1/2 byte consumers. Existing tick rejects
 pending_pc!=-1; scalar persistence alone cannot enable it.
3. Remaining nonempty module consumers: Tracks00401B20; Items0078B240/7F0;
 Team controllers; Sound backend; Versions apply; LMaps0048D270; Particles004428E0/
 00442C80; populated projectile writer if reached. Optional missionA8+10/+14
 remains unresolved. Preserve genuine no-op/reset-only classifications.
4. Full GameData/HumanStore and legacy aggregate case2, reset0069C390 and failed
 activation/shutdown tails; cleanup/repeat paths before enabling frontend resume.
5. P2.7 repeated debrief activation/modal gates, protection-table producers,
 original PRNG seed/state lifetime. Arithmetic and producer are mapped; do not
 replace them or assume ignored save writes succeeded.

Export remains schema1, documented additive batch_ids and virtual dispatches
array (shared targets retain all selectors). Stable nodes/edges and source SHA
preserved. Regenerate: `python3 analysis/future-dependencies/export_graph.py`.
Do not refresh implementation claims without deliberately selecting a new source
SHA and ledger. No permanent gameplay tests or source implementations added.
