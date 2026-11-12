# P22-HUM handoff —2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll remains OPEN**.
Read `AGENTS.md`, Phase2 section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[Human contracts](analysis/human-ragdoll-source.md) and relevant linked contracts.
Continue dependencies/checkpoints until the FULL selected checkbox completes or
is genuinely blocked. Source progress does not close application/desktop work.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
requested transferring back here; transfer evidence is ignored WORK
`human-return-main-20261111`. Preceding checkpoint **9ebacae**; check current
HEAD/status. Preserve LIFE/UI/PACK/research worktrees, `/tmp/hd2-p22-hum-continue`
and unrelated changes. No agents were spawned. No push. No original EXE/DLL/
Proton or Windows fixture execution; native own audits only, Windows compile/link.
Heavy compilation and Ghidra MUST remain sequential. Mission loading remains
blocked; no mission desktop acceptance. Source-only checkpoint commits authorized.

## Latest verified source

[Item12 receipt](analysis/checkpoints/p2.2-human-car-item12-20261112.json) adds7
partials/extends4: **34 complete/2740 partial(2735client+5engine)**, no promotions.
[Full contract](analysis/human-car-item12-source.md). Actual00479450 clone and
005FDA20/005FDAB0 type12 owner bind SAME runtime registry/body list/buckets;
005FDC00/005FDC50 lifetime→bind→configure→reset→actor ordering and004CD550 wheel
fields are in source. Use EXISTING GameRuntimeActors.field74_, reset both passes,
not a parallel counter. Actual D8 material11/D4 empty/5C AL0 confirmed, absent
leaves ZERO credit. No inventory/Physics substitute. Native400 wheel/300 part
lifetimes and failure/expiry PASS ASan/UBSan/LSan; strict nested actual World/Car/
Item12/Physics/Human and final launcher/probe PASS `human-car-item12-*`.
Engine/frame/math/activation/reset/authority simulation/effect endpoints are
explicit doubles; no original trajectory or containing-frame claim.

**Actual004C9000 body frame still REQUIRED**: GameWorldActors::update_detached_body
throws BEFORE even lifetime reads. No stationary/lifetime-only success.695 is
confirmed unwritten by ctor/alignedmalloc/activation/wheel launcher; only part
launcher writes it. Keep optional produced state; no invented wheel expiry bool.
Do not re-audit this without new evidence. Fade starts final500ms; expiry60 is
00486E90→camera amount0→frame54(1), NOT fade cancellation.64 is existing fade-out.

Preceding checkpoints remain verified; don't reopen unchanged kernels:
- **9ebacae** [glass fragments](analysis/human-car-fragments-source.md): actual
  69C→00485BA0 (absent frozen0credit)→00484F60 FASTCALL/RET40, SAME Physics/type4/
  fade borrower, six ENGINE draws, LOD→mesh→visual factory reference retirement.
- **a29033d** [physical reset/mesh/glass](analysis/human-car-physical-reset-source.md):
  004DC780→004E9DB0/004DCD10, immediate mesh enum, actual retained texture exchange/
  restore, seated Human DD0 opacity. Max6 saved wheel bits; mass*3 impulse at
  MODEL POSITION actor350/core2C0.004066C0 is SCALE, not decompiler subtraction.
- **5f7aa6d** [authority destruction](analysis/human-car-destruction-source.md):
  preserve254A/wire/Script/source/wheel/experience/area ordering, water current
  driver, victim hostility observer, radius5 negative amount, static-only ray ABI.
- **e6f08b7/cfd8aa3** [Car DM](analysis/human-car-dm-presentation-source.md)/
  [blast](analysis/human-car-blast-damage-source.md): SAME part/wheel/embedded24E0
  DM/packets/particle/SOUND. Confirmed unwritten EXPL record8 and unparsed smoke14
  remain honest failures, not repeated-audit targets.
- Earlier [Mine](analysis/human-mine-contact-source.md),
  [Versions](analysis/version-damage-transitions.md), clip/rest/reaction selection/
  damage/session checkpoints remain linked from the Human contract.

## Next work

1. Finish actual **004C9000→004C8EE0→wheel004C94B0/part004CB2B0** simulation/contact
   and sleep004CF440/frame004CEAA0 through SAME RigidBody registry/fields/buckets.
   Read Item12 contract and WORK `human-car-item12-next-motion-index.txt` before
   exporting kernels (they are already indexed but still unexported). Existing
   `human-car-item12-motion-workers/{004c9000,004cdfe0,004cef00}.c` and
   `human-car-item12-motion-additional/{004c8ee0,004c9480,005fdd10,...}.c` are default
   aliases; raw/rollback ABI required before implementation. **004C9480 is only
   37-byte wake helper, not integration kernel**, absent frozen0credit. Initial
   plain export stopped there; rollback-only ExportHumanCarItem12Motion created
   temporary missing boundaries and succeeded. Original table00810700 maps body
   00bind/04CDFE0/08CE5F0/0CCEC60/10CEF00/14C9480/18reset/1Cclose/20C8EE0/24C9000.
   Continue Car event2/timed-DM/deformed mesh and other reached receivers, then
   full Human entry/frame. Keep temporary focused audits ignored until closure.
2. Full status9/received0 entry remains unbound. Read
   [shared world](analysis/multiplayer-world/shared-world.md).00730A40 needs actual
   peer6C/client334 wrappers/worker lifecycle→0072E890→00731610(true). Reuse WORK
   `human-entry-connection-originals`. Worker registration underB4 writes connection08
   =workerD4 before append; swap-last unregister does not delete. Client00723D20
   publishes334 thenport10/register; host007260A0 peer6C register AFTER418 unlock.
3. Preserve publication/failure order: status9 restores retained game BEFORE menu
   shutdown using SAME00698510/006989B0. Received0 manager replacement precedes
   old-game retirement; failed new-game deletion differs from status9's explicit
   shutdown/delete. Voice final ABI-v3 has EIGHT initializer args ending config/
   optional interface; interface18, publish0C before init, client-before-server close.
   Non7 zone decisions/frame and full Human containing frame/failure/re-entry/
   ordered teardown remain required. Actual loading/render/user acceptance remain
   separate. No fake transport/service wrappers.


## Commands

WORK=`analysis/work/mission-world-20261103`; original
`analysis/work/hd2.unpacked.exe` SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
`python3 -B tools/research_index.py find ADDRESS...` before exports; candidates
are not equivalence proof. `neighbors ADDRESS` has no`--count`.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
ExportFunctions refuses existing output dirs; inspect ERROR even on exit0.
Never overlap heavy compilation. `objdump -s` ignores bounds: bounded PE reads only.
