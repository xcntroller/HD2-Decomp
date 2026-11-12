# P22-HUM handoff —2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll remains OPEN**.
Read `AGENTS.md`, Phase2 section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[Human contracts](analysis/human-ragdoll-source.md), and the relevant linked notes.
Continue dependencies/checkpoints until the FULL selected checkbox completes or
is genuinely blocked. Source progress does not close application/desktop work.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested transferring back here; transfer evidence is ignored WORK
`human-return-main-20261111`. Preceding checkpoint **5f7aa6d**. Check current
HEAD/status before acting. Preserve other LIFE/UI/PACK/research worktrees,
`/tmp/hd2-p22-hum-continue` and unrelated changes.

No original EXE/DLL/Proton or Windows fixture execution. Native own audits only;
Windows compile/link only. Heavy compilation/Ghidra **sequentially**. No push.
Commit completed source checkpoints and continue; interactive acceptance requires
the user. Mission loading remains disabled; no mission desktop verification.

## Latest verified source

[Queued physical/mesh/glass receipt](analysis/checkpoints/p2.2-human-car-physical-reset-20261112.json)
adds6 partials/extends3: **34 complete/2732 partial(2727client+5engine)**, no
promotions. [Full contract](analysis/human-car-physical-reset-source.md).
Actual event4 now binds004DC780→004E9DB0, controls, authority004F2790,
004DCD10 immediate004DCDE0 mesh traversal,004DD040 mesh refresh, reverse parts,
model254C release and actual seated Human DD0 opacity. Preserve max6 saved wheel
bits, transient speed-dependent engine reset, mass*3 impulse at MODEL POSITION
actor350/core2C0 (not centre), separate ENGINE/CRT draws and failed prefixes.
00500DA0 shares SAME actual retained glass texture exchange with004FE910 restore;
69C=004D4A10 genuinely ignores direction/name but selected name is still read.
Positive mesh fragments00485BA0→00484F60, Item12, deformed/global backup/occupied
restore and timed-DM remain required. Native100 reset/mesh/glass/opacity and100
queued families PASS ASan/UBSan/LSan, strict nested actual World/control/Car/Human
and final launcher/probe compile/link PASS `human-car-physical-reset-*`.
Native engine/math/Scene/bucket/mesh/material/fragment/physical-control endpoints
are doubles as detailed in contract. No full checkbox/application/desktop claim.

Preceding **5f7aa6d** [authority destruction](analysis/human-car-destruction-source.md)
binds004DBF00/004DC580/0073EAC0. Preserve254A/wire/Script/source/wheel/experience/
area order, water current driver, victim hostility observer, radius5 small negative
damage and static-only ray output-pointer ABI. Previously verified kernels stand.

Preceding **e6f08b7** [Car packets/DM lifetimes](analysis/human-car-dm-presentation-source.md)
binds actual004DB360 to SAME parts/wheels/DM/streams/module17/SOUND. Actual
00445740/00445800 signed binary lookup/immediate stable deletion (no pooling),
DM explosion/fire/smoke/SID/clear/reset/dtor and puncture9EC/CBC are in source.
Record8 is genuinely unwritten by004E09D0/malloc: require produced handle or
actual empty-list proof; otherwise fail. Constructor smoke14 is also unwritten
without actual DM parse. **Confirmed omissions; no repeated audit needed.**
Preceding **cfd8aa3** [Car blast](analysis/human-car-blast-damage-source.md) binds
004DBA10 to actual body/parts/wheels/embedded24E0 DM. Car254A differs from seat
engine234F/core22BF transition. Event glass0/part1/puncture3/destruction4/explosion5.
Event writers and004D9A40 absent frozen ZERO credit. Earlier focused audits pass.

Preceding verified dependencies (contracts retain evidence/order details):
- **e861ace** [armed Mine/shared explosion](analysis/human-mine-contact-source.md):
  actual item70/71 type2/radii5/8/power2500/25000/sound2; Mine may self-retire during
  Human source damage. Retained local runtime reference preserves global access.
- **4e96476** [Versions](analysis/version-damage-transitions.md): SAME Master/Part,
  d+model category5 cache open10, effects/Script/Physics/material10, slot retirement.
- **59fee6f** [Mine producers](analysis/human-runtime-mine-source.md): actual model/
  pose/load→create/activate→bind→factory release→inventory→late optional packet.
- Earlier source contracts/checkpoints for solver/clip/rest/reactions, authority,
  policy/death, PACK/session/spawn/map/entry/voice remain valid. Do not reopen
  unchanged kernels. Temporary native audits remain ignored until full closure.

## Next work

1. Continue positive **00485BA0→00484F60** mesh fragments and Item12
   **00479450→005FDC00/005FDC50**, then timed-DM and other required reached effects.
   Read `human-car-physical-reset-source.md` before changing reset/presentation.
   WORK `human-car-glass-fragment-index.txt` locates existing bounded original
   00485BA0 assembly and00484F60 C; both absent/unfinished dependencies have zero
   new credit.69C raw/source is confirmed ignoring extra args; don't invent use.
   New reset export `human-car-physical-reset-abi` still has decompiler alias errors;
   use raw `human-car-physical-reset.asm` and math evidence (004066C0 SCALE, not
   subtraction). Preserve actual model/helper/seats/body/wheel/material owners.
   Focused temporary audits remain ignored until full selected checkbox closure.
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
