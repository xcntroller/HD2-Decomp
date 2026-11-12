# P22-HUM H2 handoff —2026-11-12

Active worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h2`, branch
`feature/p22-hum-h2`. Base of latest work is f771a60 (D6a), after fb89d76's move
frome742f84. Check HEAD/status; preserve unrelated worktrees and user changes.
Read AGENTS.md, Phase2 section2 plan.md, [Human plan](analysis/p22-hum-plan.md).
**H2 remains OPEN.** No agents/push, original EXE/DLL/Proton or Windows output
execution. Heavy compilation/Ghidra sequential. Actual application/render/user
desktop acceptance remains P2.2; this checkpoint is source-only.

## Latest — D6b1 worker/socket/control lifetime

[Contract](analysis/human-worker-control-source.md),
[receipt](analysis/checkpoints/p2.2-human-worker-control-20261112.json).
D6b1 is a completed bounded source checkpoint; **D6b/D6 remain open for D6b2**.
**34 complete/3063 partial (3058client+5engine)**:42 NEW frozen partials,
extended00722180, no promotions. Absent virtual helpers/wrappers receive no new
credit; no imported OS/engine function credit. Earlier source checkpoints remain
S01–S14/D0–D3/D4a/D4b/D6a; do not reopen their unchanged kernels.

- Original client raw proves worker process+33C/socket+34C. Actual singleton PE
  zero-fill produces otherwise untouched workerD4/socket74/78. Generic registry
  coldD4 stays undefined; actual singleton loader producer and start0 are explicit.
  **WorkerD8 IS process414**: same existing callback0072C360, cleared by worker
  ctor and installed by startup, not a second callback or input/quantizer owner.
- Actual socket lists/buffers/54 precede B4. Start preserves socket-thread creation
  BEFORE74/78 publication, workerB0/socket/end/A4/AC/D4/clockCC-D0/worker-thread order.
  Actual source loops maintain/receive and wait/drain/reset; priority2 and natural
  completion events. Lookup recursively takes B4. Atomics define native lockless
  shared fields in C++; source thread handles join before storage can die.
- Reverse last-wrapper deletion under B4 precedes334 clear and worker stop,
  before418 record retirement/input-manager shutdown. Stop flags precede1000ms
  native event waits; pending join fails retaining ownership, checked retry works.
  Source exceptional RAII fallback is separate from native fault/timeout behavior.
  Storage B4→socket54/buffers/list4C/list40/vector→registry capacity is preserved.
- Actual raw scratch shares the existing bit kernel, with no Human input clone.
  Cold queue ACKs, ping request/reply, signed deadlines and23-bit ACK sequence
  lifting are source. Ping sends UNDER20; normal ACK sends AFTER builder20 unlock
  and temporary-sentinel retirement. Snapshot timing and actual38 socket identity
  are preserved. Datagram checksum/XOR,500-byte split/paced FIFO and raw OS
  sender/receiver are source. Populated messages and received fragments still fail
  at their missing receivers, preserving earlier mutations; no fake delivery.
- Client prefix keeps actual338, constructs/publishes334 BEFORE port+1/register.
  Completion publishes SAME local00982104 BEFORE334+04; failure5044 byte unmasked.
  Client termination unregisters/deletes/clears334 then5044; caller app11 remains
  required. Prepared-host tail requires actual prior record04, publishesA8/A1/
  wrapper6C/stats under418 and registers AFTER unlock. Abort intentionally leaves
  stale6C; weak source metadata rejects use instead of fake NULL/lifetime extension.
- StartupSession owns ONE worker as LAST member, before any borrowed member dies.
  Win32 platform is concrete, launcher/probe link ws2_32. Cold reset uses SAME owner;
  non7/interface/peer guards and host/join/application-loading boundaries remain.

ASan/UBSan/LSan PASS100 actual source worker/socket/connection/C4/entry/cold-reset
families,200 same-worker start/reset cycles, callbacks on real native C++ threads.
OS/graphics endpoints are explicit doubles. Includes forced early publication,
real paused-callback timeout/retry, throwing pre/post-A4 acquisition, required04,
client identity prefixes, abort weak borrow, ACK/ping lock/deadline/wrap, checksum,
fragment boundaries and failure without false completion. Prior D6a100 regression
PASS on NEW artifact paths. Strict actual StartupSession i386 and final launcher/
probe compile/link PASS; never executed. No TSan, live transport/World or desktop
claim. Receipt freezes11 source files/14 artifacts/145 evidence files, including
separate executable snapshots so later builds do not overwrite this checkpoint.
New temporary audits remain ignored until selected H2/D12 cleanup; never overwrite
frozen dedicated scripts/binaries/logs. Source hashes are historical across commits.

## Continue through dependencies

1. **H3 dependency now committed10bebe9** in sibling H3 worktree. Review its
   contract/receipt and integrate that committed snapshot, preserving any later
   uncommitted edits. H3 source closes seat/mounted/aim/casing receivers including
   004C6620; its own handoff explicitly leaves occupied/non7 Car simulation **D5**
   open. Do not remove that containing frame gate merely because H3 is source-done.
   H2/D4a/D4b and H3 share files; resolve source/contracts/ledger as a union, not a
   blind reset. Keep this H2 handoff and generated current counts.
2. **D6b2** actual DirectPlay admission/name/address/record and callback composition
   with same client04/server00/current application11, plus reached retained message/
   queue/reassembly receivers. Relevant next bodies00747240/00745EA0/00747DD0;
   use index before exports. Current helper prefixes are not a complete live
   00723D20/007260A0/00722180 service. Thread creation/A4/callback414 is not readiness.
3. **D7/D8/D9** containing status9/received0/non7 entry/frame/reset stay required:
   same00698510/006989B0, retained game restored BEFORE menu shutdown, real input
   replacement BEFORE old World retirement and distinct failure deletion. OWN has
   advanced committed application lifecycle; review dependencies before integration.
4. **D10** actual unmounted0077DE30→0074D510→007B0910→007A7470 can advance without
   all live transport. Existing earlier audits still separate containing control/
   selection/geometry endpoints; they do not close real containing-owner composition.
   H4 committedf3788fd common completion/equipment receivers, but now has additional
   uncommitted script-event work: integrate only reviewed committed dependencies.

**D11 genuine closure blockers unchanged:** fresh part004CB2CD reads/toggles
unproduced5F9 BEFORE motion; wheel005FDD10 reads unproduced695 after body/lifetime
mutation at<=500ms/expiry. EXPL8/smoke14 also lack demonstrated producers. No defaults
or fresh success from warm fixtures. [Exact triggers](analysis/human-reaction-containing-boundary.md#source-closure-blockers).
Keep H2 open, advance independent required source, and do not repeat those confirmed
searches without new contradictory evidence. D12 consolidated actual containing
verification/ledger reconciliation/temporary audit cleanup remains required.

## Evidence and commands

WORK=`analysis/work/mission-world-20261103`; original=`analysis/work/hd2.unpacked.exe`,
SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
`python3 -B tools/research_index.py find ADDRESS...` BEFORE new exports;
indexed mentions/absence are not proof. Worker-wire batch's missing007456D0 ERROR
is recorded; bounded raw covers absent helpers with NO frozen credit.
Native g++17/O0/warnings-as-errors/pthread/ASan/UBSan/frame pointers, runtime
`.tools/hd2-sanitizer-runtime/usr/lib64`. i386 compiler
`.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
Ghidra JAVA_HOME=/home/control/.sdkman/candidates/java/25.0.0.0-oracle, project
/home/control/Documents/Projects/HD2ResearchProject/HD2Research, process
hd2.unpacked.exe -readOnly -noanalysis; one local WORK scriptPath, rollback only.
Check ERROR even exit0; never overwrite frozen output dirs. Bounded objdump-d/PE
reads only; objdump-s ignores bounds. No original/Windows execution or desktop claim.
