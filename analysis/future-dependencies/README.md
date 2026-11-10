# Forward Phase 2 dependency research

Research branch: `research/future-dependencies`. Scope: P2.4–P2.7, with P2.6 first.
No game implementation, ledger changes, main plan/handoff edits, website changes,
networking changes or original executable/DLL execution belong to this directory.

Start with [overview](phase2-overview.md), [save contracts](p2.6-save-load.md),
[results](p2.7-results.md), [batches](implementation-guide.md) and [handoff](HANDOFF.md).
Deep contracts: [actor identities/payloads](p2.6-actor-persistence.md),
[context layout](p2.6-context-layout.md), [module classification](p2.6-module-persistence.md).
The versioned [graph](phase2-graph.json) separates research, source recovery,
integration, verification and desktop acceptance. Unknown means unknown; neither
an original call nor a recovered body proves an integrated gameplay dependency.

Evidence uses the unpacked base client `hd2.unpacked.exe`, image base `00400000`,
SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Addresses are original preferred-image virtual addresses, lowercase eight digits.
Existing contracts are reused without reopening confirmed P2.2/P2.3 work.
New analysis uses sequential read-only/no-analysis Ghidra exports and targeted
static PE reads/disassembly. Decompiler prototypes are hypotheses until call-site
registers/stack are checked. In particular apparent missing `this` arguments are
not proof of globals or free functions. No dynamic equivalence is asserted.

Commercial binaries, full assembly/decompilation exports and assets stay outside
Git under ignored `analysis/work/future-dependencies/`. Tracked evidence records
contain derived contracts, instruction locations, export digests and provenance;
these are research findings, not commercial data copies. Local absolute paths
are deliberately absent from the website export.

The analyzer adapter contract is documented in `tools/progress-analyzer.md` and
implemented in `tools/progress_graph.py`. This export uses compatible namespaced
`{id,type,data}` nodes and `{id,source,target,type,data}` edges, with an explicit
schema ID/version and extra readiness/edge-semantics fields. No existing analyzer
or website importer is changed. Dependency arrows run prerequisite → dependent;
direct-call arrows run caller → callee. Only `depends_on` edges with
`blocking=true` are scheduling constraints. Ownership and calls are distinct.
The source SHA pins implementation claims; refresh by joining original addresses
against the new ledger and source at a later SHA, never by copying old readiness.

## Version1 additive export fields and catalogs

The existing schema ID/version, three node types, IDs and edge hashing remain
unchanged. This checkpoint adds documented optional `data.batch_ids` on nodes:
stable implementation-guide IDs, including child batches. These are associations,
not proof of implementation or dependency edges. New catalog groups use stable
`subsystem:future:*` IDs. Function status remains a pinned ledger claim; grouping
does not grant new source credit. Five verification levels remain separate.

Virtual edges add `data.dispatches`, an ordered array of concrete selectors
(`condition`, `vtable`, `slot` and applicable `actor_type`, `state_kind`,
`module_kind`, `virtual_slot`). Shared targets retain all selectors in ONE stable
edge. Legacy scalar fields remain when unambiguous; for multiple selectors use
the array. This fixes lost slot10/16 and Human kind6/8 associations without
changing edge identities. Direct calls never imply blocking dependencies.

`research_readiness` includes `contract_ready`, `partial_contract`,
`verified_no_op`, `delegates_base`, `fully_researched_persistence_contract` and
`conditionally_required`, alongside the prior address/external states.
`implementation_readiness: bounded_contract_available` permits source work on
that documented boundary; it is not a completed feature or transitive readiness.
`blocked` retains unresolved required leaves/lifecycle. Actual implementation,
integration and desktop status are independent. No progress percentage is used.

Additional version1 evidence catalogs contain derived findings only:

- [Actor dispatch](evidence/actor-persistence.json): `actors` with type/vtable,
  paired offsets48/4C, ownership/readiness/boundary; `human_states` with B0 kind
  vtables/offsets2C/30; `item_subtypes` supplies four item44/48 pairs and
  `common_item_asymmetry` preserves writer/reader tag routes; `unresolved` lists conditional uncovered types.
- [Module dispatch](evidence/module-persistence.json): original `slots` preserved,
  with additive label/classification/contract/confidence and no-I/O body digests.
- [Research contracts](evidence/research-contracts.json): stable `contracts`
  groups, `function_bounded_readiness`, `existing_associations` and typed
  `relationships`. These are explicit reviewed export inputs, not a call parser.
- [Result arithmetic](evidence/results-arithmetic.json): rank constants, ordered
  redistribution destinations/factor constants and outstanding lifecycle gaps.
- [Manifest](evidence/manifest.json): prior export indices retained;186 new
  fingerprinted exports appended, for250 total. Failed guesses are excluded;
  instruction anchors include byte digests for ambiguous receiver/mutation gates.

Regenerate with `python3 analysis/future-dependencies/export_graph.py`.
Only this export and research evidence change; the original analyzer/website do
not. Readiness and correction details are in the latest handoff/verification
receipt. Raw exports are local ignored evidence and are unnecessary for website
rendering; they are needed to reproduce the static verification itself.


Car checkpoint adds version1 [car-packed-v10.json](evidence/car-packed-v10.json):
header_fields use numeric byte offsets/sizes and explicit copy/derived operations;
sections/count_offsets/size_formula give variable framing, wheel_fields list the
25 persisted words, unwritten_header_bytes records padding. This sidecar adds no
new graph record shape or implementation credit. See [Car](p2.6-car-format.md)
and [Player](p2.6-human-state-leaves.md). Manifest now has257 exports, including
seven new Car helper exports; previous export indices remain stable.

After newer source commits entered this branch, the exporter was corrected to
read the ledger and recovered-source mentions from the pinned source SHA via Git.
Additive optional `revision` on input/provenance/source-mention records means
resolve that path (and JSON pointer/line) at that commit, not current HEAD. For
revisioned inputs, SHA256 hashes those revision's bytes. Records without revision
retain the existing working-tree path semantics. Source SHA stays52c5689; no
implementation status is silently refreshed. Export regeneration needs that commit
in the local Git object database. Website rendering can use the self-contained
export and attach revision-aware repository links; no website code is changed.
