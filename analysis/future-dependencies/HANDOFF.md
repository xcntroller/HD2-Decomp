# Forward research handoff — Car packed-format checkpoint

Branch `research/future-dependencies`. Implementation status remains pinned to
52c568964063207b0d15bea8ef98af09f89d84df. Main source, ledger, root plan/handoff,
website/analyzer, networking and other worktrees unchanged. No original EXE/DLL,
Windows fixture or Proton execution; no desktop result or new reconstruction credit.
No push or merge.

New: [Car version10](p2.6-car-format.md), [numeric byte map](evidence/car-packed-v10.json)
and [Player nested state](p2.6-human-state-leaves.md). Existing actor/context/module/
result contracts remain prerequisites. Do not redo confirmed vtables, IDs, globals
or result arithmetic. [Batches](implementation-guide.md) now distinguish layout,
mesh, Car restore and Player leaf work; whole live save/load remains blocked.

Resolved in this checkpoint:

-108 fixed Car header fields cover000..170;171..173 are unwritten padding.
 Complete variable boundaries:12-byte point records;20-byte geometry headers plus
 four bytes per changed LOD0/LOD1 vertex;20-byte ancillary records;100-byte wheels.
 Full raw word/byte map, point quantizer, six-bit mesh displacement quantizer,
 suffix hash/lookup, existing-array matching and skipped-record behavior recorded.
-IMPORTANT correction:004F7300 size is174h+12P+20G+4*sum(n0+n1)+20A+100W.
 Its decompilation lost the live changed-vertex byte accumulator. Assembly counts
 both LODs. The previous formula would underallocate deformed saves.
-Outer packed byte count and internal computed size are distinct;004F8510 checks
 version10, not buffer length. It destructively detaches before changing model
 pose.004E95B0/004E98C0/004F5840 side effects and required real collision/model/
 mesh owners mapped. Allocation padding and null ancillary-child bytes are not
 deterministically initialized by the original writer.
-10006 receiver is inline Car+24E0. Its bytes contain no actor-ID resolution;
 previous seat/attachment label was unsupported and is corrected. Mounted Human
 references remain in established Human1001/1006, not a new Car passenger array.
-Player B0 kind1 owns18-byte0840 and optional68-byte0841 (64 bytes plus counter).
 Reader omission retains constructor state; no count-sized input allocation.
 Common B0/real Human-controller dependencies remain mandatory.

Evidence:257 fingerprinted exports, seven new targeted Car support exports.
Prior indices retained. New assembly anchors cover size accumulation, byte
quantization, short-name hashing and24E0 receiver. Full exports/disassembly remain
ignored or temporary, outside Git. Heavy Ghidra was sequential readOnly/noanalysis.
[Verification](evidence/verification.json): direct header copy fields independently
matched assembly, field nonoverlap/padding/wheel extent, constants, new export and
anchor digests, deterministic graph/provenance/dependency validation. Graph320 nodes,
659 edges, unchanged schema1 with new catalog groups and sidecar documented in README.
This branch contains newer main-agent commits; exporter now reads ledger/source
mentions from the actual pinned Git snapshot. Optional revision on evidence/inputs
is documented. Previous HEAD-dependent joins must not be used to refresh status.
No codec execution against original save files, runtime equivalence or desktop test.

Next research boundary, in order:

1.004F8510 tipped-pose correction004F8600..004F89EF: match x87 AngleTo gate,
 source axes, basis reconstruction and normalize branches against instructions.
 Then verify real mesh clone/lock/invalidate and part post-read receiver semantics;
 the byte format is closed, whole Car restore is not. Resolve actual producers of
24E0 fields without assuming seats. Reuse existing Car loading/mass/deformation owners.
2. Human ordinary00576FE0/005774A0 command leaves, Guard/Script/Active extensions,
 inline98 animation0077FE20/00780100,00783FD0/00784310,007938C0/00794430.
 Player payload discovery need not repeat; common B0 and actual control tail remain.
3. Pending Script2600 tick receiver, rotations/frame animation and derived context
 bytes. Existing tick rejects pending_pc!=-1; module4 stays before11/12/13 replay.
4. Remaining demonstrated nonempty module consumers and optional missionA8;
 full GameData/HumanStore, reset0069C390 and failed activation/repeat shutdown.
006B7410/006B7D30 are already mapped; reopen only their unknown called leaves.
5. P2.7 repeated activation/modal gates, protection producers, PRNG seed lifetime.
 Rank/medal/nine-draw arithmetic and producer order are already confirmed.

Regenerate graph with `python3 analysis/future-dependencies/export_graph.py`.
Research readiness does not promote implementation readiness; Car layout group
remains implementation-blocked on real owners/integration. Explicit native length,
count/index/cleanup guards and zero padding are sensible deviations, not original
atomic rollback or proven successful recovery.
