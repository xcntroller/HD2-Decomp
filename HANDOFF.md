# P22-HUM handoff — 2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the FULL selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Check HEAD/status before acting. Transfer
was byte-verified in ignored WORK `human-return-main-20261111`. Other LIFE/UI/PACK/
research worktrees and `/tmp/hd2-p22-hum-continue` are separate; preserve them and
external master merges. Latest preceding checkpoint5a1a98d; older externally
rebased checkpoint hashes may differ from earlier handoffs.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra **sequentially**. No push. Source-only checkpoints
may be committed; interactive acceptance requires the user. Application mission
loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Authority spawn/map receipt](analysis/checkpoints/p2.2-human-authority-spawn-map-20261112.json)
adds14 partials/extends7, no promotions:34 complete/2649 partial (2644 client+5 engine)
of18783. Full original contracts/evidence/exclusions are consolidated in the active
Human analysis.00611C90 now uses actual zone/identity/occupancy selection, supplied
ARG2 stats, same factory/A8/activation, late mode/A8 reads, match/flags/voice,
direct versus wire and final Script event. Actual activation borrows this policy.
Direct NULL stats, recursive418 and original captured identity (even-1) retained.
00475840 scans IDENTITY10/14 slots, NOT active list.005ED700 preserves cursor,
modulo, neutral-owner packet before occupancy,32-attempt/style2 quirks. Actual
shared downward sphere query, no collision stand-in.

Actual type37 zone now loads/retains/releases its flags/spawn group and vertex
instance in SAME Scene2/module9 lifetime. Release callbacks see native borrowers
until AFTER release.005EC8B0 uses same flags/HUD/vertex registry;00475D50 command
lookup correction preserves reserved-hole pending fallback.005EBFD0/005EDA60/
005ED110/005ED260 are ABSENT frozen: zero new credit. Non7 zone005EC060/005EC540
still fail at required decision/containment receivers; genuine mode7 returns kept.

Rejection0F scalar FIFO(peer5/reason1) reaches actual00728450→00699620,90 write and
first live-squad spawn identity clear before HUD map show. Actual map show/pointer/
zone selection/goal captions borrow SAME process squad/peer64 and zone98. Status2
goal intentionally retains faction visibility. Missing/retired owners still fail.
Latest focused ASan/UBSan/LSan authority/zone/packet/map/received rejection and spawn
regression PASS. Engine/factory/geometry/HUD/vertex endpoints are explicit doubles,
not full World/render/transport. Strict complete actual World/authority factory/
HUD i386 and sequential latest launcher/probe compile/link PASS
`human-authority-spawn-{host,application}-i386.log`. No Windows output ran.
Temporary audits stay ignored until full checkbox completion.

Earlier [presentation spawn](analysis/checkpoints/p2.2-human-presentation-spawn-20261112.json)
5a1a98d, [Car/contact](analysis/checkpoints/p2.2-human-car-contact-20261112.json),
[process input](analysis/checkpoints/p2.2-human-process-input-20261112.json),
[application messages](analysis/checkpoints/p2.2-human-application-packets-20261111.json)
and [peer lifecycle](analysis/checkpoints/p2.2-human-peer-lifecycle-20261111.json)
remain valid. One process input owner;0072E890 record CLEAR differs from received
00723000 manager REPLACEMENT. Input link stays published through B0 retirement.
Presentation2 retires peer/Human;17 sets result/hint/log, not retirement.

## Immediate next work

1. Full control entry/status9. Read `analysis/multiplayer-world/shared-world.md`
   and WORK `human-control-entry-next-index.txt`. Read-only research checkout has
   `analysis/work/future-dependencies/p28-entry-handoff/00730a40.c` and00731610.c;
   not yet copied/pinned in MAIN.00730A40 toggles peerAC then actual peer6C/client334
   wrapper08 (authority named stage74==2 peers under418; client direct local slot).
   These actual wrapper producers/owners remain required: do not fabricate them.
   Then implemented0072E890, then00731610(true): requires manager14, resets each
   input twice via0073E660(1,0,1)/(1,1,1), clears ONLY authority queue ifbit1,
   otherwise ONLY presentation queue under430. Need original0073E660 and typed
   wrapper source before claiming complete entry. Current source only clears both
   queues at distinct cold reset; do not reuse that method here.
2. Received0 is distinct:00724110 sets5044=1, optional descriptor4E88/profile38
   service00724370(false), then canonical manager REPLACEMENT00723000 BEFORE old
   game retirement. Existing original C in `human-process-input-originals`.
   Still requires current-state/map/load/sync/request9 owner boundaries. Actual
   controller status9/received0 integration must retain original state publication/
   failure order. Source World input acquisition stays an explicit lazy adapter.
3. Non7 zone frame/decision, occupied attachment80 and00479700 kind2/9C runtime
   links, then full containing reaction/frame, failure/re-entry/ordered teardown.
   Application loading/rendering/user desktop acceptance remain separate.
   Commit larger verified checkpoints, consolidate docs, continue the FULL checkbox.

## Commands and evidence

WORK=`analysis/work/mission-world-20261103`. Source originals/indexes and hashes
are pinned in receipts; research checkout reused read-only. Client
`analysis/work/hd2.unpacked.exe` SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
`python3 -B tools/research_index.py find ADDRESS...` before new exports;
`neighbors ADDRESS` has no `--count`. Candidates are not equivalence proof.
`python3 -B tools/function_progress.py --render` / `--check`, `git diff --check`.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
Inspect ERROR even on exit0; never overlap with heavy compilation.
