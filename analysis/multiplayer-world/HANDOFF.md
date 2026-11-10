# P2.8 research handoff

Research-only checkpoint on `research/future-dependencies`. No implementation,
ledger/main plan/root handoff/launcher/website/networking worktree changes. No
original program execution, desktop verification, push or merge. Preserve all
P2.4–P2.7 research. Main source pinc09c22c01d1049d7f087142064a3c2c53488f582;
networking referencee777570c6d0dade25d6a43b6a2e417ebb0756379. Current main still
declines actual mission loading; source/native notes do not prove P2.3 acceptance.

Start with [shared game entry](shared-world.md), [mode matrix](authority-modes.md),
[player control](player-control.md), [packet map](packet-synchronization.md),
[source coverage](source-coverage.md), [batches](implementation-guide.md) and
[co-op assessment](coop-feasibility.md). [Acceptance](acceptance.md) separates the
bounded Phase2 receiver/source checks from real Phase3 live session verification.

High-value recovered contracts:

- Status9 and received application command0 both construct SAME00698510 and load
  through SAME006989B0/006CA780/Scene2/module registry.7/8 remains0068A710 connection
  owner. Received0 mutates configuration/map before destroying old state, then
  initializes and enqueues request9; it is not merely a status9 alias.
- Status9 preprocessing is00730A40 load toggles →0072E890 peer Human/nine-word
  reset →00731610 input reset/ONE selected FIFO retirement. Borrowers/process
  queues and world lifetimes must not be combined. Original failure is not atomic.
- Exact selected0x48 map wire pair and config widths recovered, including74-bit
  objective records and signed16 expansion.006B8700 creates kind12 objective
  nodes, copies resource toGameData38/name80 and borrows inline mission from
  application. Existing source admits only campaign/nonempty squad requests;
  removing validation would not supply the missing MP descriptor contract.
- Process flags7/3/2/1 gate same authority/presentation passes. Configuration04,
  rules224 and match88 are distinct. Client and authority advance game28 in
  different coordinator phases. Script authority uses nonoffline package; client
  presentation is not a second independent mission VM. Network finish retires
  application live squad rather than offline snapshotting it.
- Peer slot, external transport ID, Scene2 token, actor10 lookup, saved runtime
  identity, Human170/profile and roster index remain separate.00612A60 ECX=peer
  slot, EDX=actor identity,RET18; writes SAME actual Human into peer3C. Listen host
  presentation reuses authority actor; only local peer binds camera/roster.
- Process packet layers, request2BC, input20/22 and ordinary42 routing recovered;
  callback0072C360 EDX=payload with3 stack args/RET0C. Queues copy bytes, unlink
  under430 and decode unlocked; false stops current packet and retires its bytes.
  Module authority28/presentation24 and exact Human B0C4/C0 targets mapped.
- Reserved authority wrapper admits only B0kind1; presentation admits1/7. Their
  discard grammars differ. Human requests validate/queue actual typed commands;
  presentation applies actual state/action receivers. No generic packet skip.
- Custom queue2 has retained retry/ordered sequencing (ordinary42); input20/22
  chooses queue0/2. No DirectPlay-reliability naming or full ACK/socket protocol
  claim. Worker registers borrowed wrapper; unregister precedes deleting it.
- Disconnect0069E720 detaches actual HUD/Players/spectator/TeamMgr references,
  retires actual actor, clears3C/stats without shifting peer slots. Transport
  terminate callback separately unregisters/deletes wrapper and requests status11.

Smallest independent implementation batch: **B28.metadata**. Mandatory Phase2
source boundary then entry/mode/queues/retirement/boundary; live spawn/consumers
are conditional batches, not universal prerequisites for truthful source readiness.
Actual transport is Phase3. No fake server/connected callback or mode7 substitution.

The later pass additionally recovered00724110 process-input replacement before
old-world shutdown, module38/3C snapshot dispatch, the full outer0x15 peer framing
and46-bit peer-state pair, module12 type2/11/35 wire framing and actual respawn
ownership/limit-exhausted cleanup quirk. See [initial sync](initial-sync.md).

Concrete remaining runtime blockers: nested actual actor50/54 initial state,
writer-paired Physics/Script/Versions snapshot fields; actual HumanInput virtual14/18 history
timeline; reached Human command helper bit fields and accepted action ownership;
conditional Versions/HeavyFireSupport opcodes; real spawn/respawn/match tails,
worker ACK/wrap/loss/socket behavior and controlled Phase3 endpoint. Exact leaves
are named in the packet contract/graph. This checkpoint is architecture-ready,
not complete wire/gameplay compatibility. Optional00724370/00724570 profile-gated services
remain conditional; received-start00724110 itself is now mapped. Main currently
keeps Inputs as a world member, requiring an explicit canonical-manager lifetime
adaptation for the original received-start order.

Highest-value next research if live synchronization is selected: Human actor50/54
thunks005AA770/005AA780→005A9C40/005AA1B0 and the actual manager0073EC50
virtual14/18 timeline; then other reached populated snapshot leaves.
Request9 compares only actor **count**, not identities/content, and its0x15 result
is a second readiness gate after world initialization. Keep that work conditional
on Phase3 need; Agent #1 can start the bounded Phase2 receiver using this checkpoint.

[Manifest](evidence/manifest.json), [contracts](evidence/contracts.json),
[source review](evidence/source-review.json), [verification](evidence/verification.json)
and [focused graph](p2.8-graph.json) preserve address/fingerprint/revision provenance.
Combined Phase2 export keeps old IDs and readiness/ledger claims, appends P2.8
observations; focused graph pins its implementation claims toc09c22c. Schema1
additive per-node revision semantics are documented in [README](README.md).
Co-op remains a separate inference/proposal: shared world reuse is real; campaign
script authority, stock/profile/result retention, possession and shared restore
would require explicit extensions. No original campaign co-op mode was established.

Verification checkpoint:112 fingerprinted exports plus small instruction/vtable
anchors;22 focused checks pass. Combined graph596 nodes/1751 edges preserves all
prior IDs, owner/readiness/status claims and earlier evidence. Focused P2.8 view
185 nodes/763 edges, with revision-aware source/provenance links and a blocking
predecessor DAG. Both exports regenerate deterministically; byte-call targets,
ABI/field boundaries, source hashes and protected-path scope verified. These
counts describe evidence/export validation, not implementation progress. No game
build/sanitizer or desktop test applies to this static research-only change.
