# P2.8 shared multiplayer world research

Research only, on `research/future-dependencies`. This checkpoint prepares the
bounded Phase 2 client receiver and records the additional requirements for
Phase 3 live networking. It implements no gameplay, transport, server or co-op.
The P2.4–P2.7 contracts remain in [future-dependencies](../future-dependencies/README.md).

Read [shared entry/world](shared-world.md), [mode matrix](authority-modes.md),
[player identities](player-control.md), [initial sync/input lifetime](initial-sync.md), [packet map](packet-synchronization.md),
[delivery batches](implementation-guide.md) and [handoff](HANDOFF.md).
[Co-op feasibility](coop-feasibility.md) deliberately separates original behavior
from inferences and proposed extensions.

Actual main source was read at **c09c22c01d1049d7f087142064a3c2c53488f582**.
That snapshot still declines application mission loading. Source/native coverage
does not establish a rendered mission, control, multiplayer integration or desktop
acceptance. The networking branch was read through Git at
e777570c6d0dade25d6a43b6a2e417ebb0756379; its paused worktree was not changed.

Original evidence: unpacked client, preferred image base00400000,
SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Addresses below are original preferred-image virtual addresses. Member offsets
are hexadecimal. Module/command-table indices without0x are decimal; application
commands explicitly written0xNN are hexadecimal DWORD values. Authority/presentation
“bit1/bit2” means masks1/2 (`flags&1`/`flags&2`), not bit indices. Existing world,
Scene2, module lifecycle, offline spawn and persistence contracts are reused.
New Ghidra work was targeted, sequential, read-only and no-analysis. Instruction
checks resolve misleading decompiler register arguments, x87 bit widths and
virtual receivers. No original executable/DLL, Proton or Windows fixture ran.
Commercial bytes, full disassembly and decompilation stay ignored/outside Git.

[Evidence manifest](evidence/manifest.json) records export hashes and instruction
anchors; [contracts](evidence/contracts.json) records explicit typed relationships;
[source review](evidence/source-review.json) pins actual source digests and gaps;
[verification](evidence/verification.json) records the focused checks. Raw local
exports are needed to repeat static verification, not to render the explorer.

[Focused graph](p2.8-graph.json) and the additive
[Phase 2 graph](../future-dependencies/phase2-graph.json) retain schema
`hd2.future-dependencies` version1, the existing three node types and edge-ID
hashing. No website/analyzer changes. Node `data.source_commit_sha` is an additive
optional per-node implementation-claim revision; otherwise the top-level SHA is
the default. Combined historical nodes keep their old ledger claims; newer
P2.8 observations link separately to the review. The focused export refreshes
only its ledger/source claims against c09c22c, never research readiness or desktop
status. Batch nodes use the existing `subsystem` type and stable B28 labels.
Unknown-relationship entries use the existing shape. Dependency direction remains
prerequisite → dependent; calls are caller → callee and are nonblocking unless an
independent `depends_on` edge says otherwise. No progress percentages.

Regenerate both graphs with
`python3 analysis/future-dependencies/export_graph.py`, then
`python3 analysis/multiplayer-world/export_graph.py`. Preserve revisions when
linking source paths; a current-checkout address mention is not an implementation.

The version1 evidence sidecars are independent of graph record shapes.
`manifest.exports` uses the existing entry/export/decompilation hash/call-site
shape; `module_packet_dispatch` records24/28 and38/3C targets plus byte-span
hashes; `human_state_packet_dispatch` records B0C0/C4;
`actor_initial_sync_dispatch` records actor50/54. `reserved_slots` explicitly
records cold null18 rather than fabricating a module. Instruction anchors contain
address/size/hash and a derived claim, never raw code. `contracts` contains stable
groups/batches and typed relationships; `classification` distinguishes
required_source_boundary, conditional_runtime and deferred_extension. Batch
predecessors become explicit blocking edges. These fields do not change frozen
function status or declare complete nested payload formats. Source-review paths
with `revision` resolve through Git at that SHA, including networking references.
