# Forward Phase 2 dependency research

Research branch: `research/future-dependencies`. Scope: P2.4–P2.7, with P2.6 first.
No game implementation, ledger changes, main plan/handoff edits, website changes,
networking changes or original executable/DLL execution belong to this directory.

Start with [overview](phase2-overview.md), [save contracts](p2.6-save-load.md),
[results](p2.7-results.md), [batches](implementation-guide.md) and [handoff](HANDOFF.md).
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
