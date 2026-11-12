# P22-HUM handoff —2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll remains OPEN**.
Read `AGENTS.md`, Phase2 section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[Human contracts](analysis/human-ragdoll-source.md), and the relevant linked notes.
Continue dependencies/checkpoints until the FULL selected checkbox completes or
is genuinely blocked. Source progress does not close application/desktop work.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested transferring back here; transfer evidence is ignored WORK
`human-return-main-20261111`. Preceding checkpoint **cfd8aa3**. Check current
HEAD/status before acting. Preserve other LIFE/UI/PACK/research worktrees,
`/tmp/hd2-p22-hum-continue` and unrelated changes.

No original EXE/DLL/Proton or Windows fixture execution. Native own audits only;
Windows compile/link only. Heavy compilation/Ghidra **sequentially**. No push.
Commit completed source checkpoints and continue; interactive acceptance requires
the user. Mission loading remains disabled; no mission desktop verification.

## Latest verified source

[Car packets/DM receipt](analysis/checkpoints/p2.2-human-car-dm-presentation-20261112.json)
adds12 partials/extends3: **34 complete/2723 partial(2718client+5engine)**, no
promotions. [Full contract](analysis/human-car-dm-presentation-source.md).
Actual module13→Car1C004DB360 uses SAME World/Car/DM/parts/wheels/streams.
Kinds0/1 mutation,2 late authority/current-health gate,3 flags→actual puncture
sound9EC/CBC,5 actual explosion particle and4 DM effects BEFORE required004DC780.
Reserved8/9/unassigned kinds are genuine empty;6/7/10/11 still required.
00445740 signed binary lookup and00445800 immediate instance close/delete before
stable erase, NO pooling, use SAME process/module17/points/parent ownership.
DM004E1F70/004E2000 emit through SAME process/SOUND; reverse explosion retirement,
fire→smoke→positive SID; clear/reset/dtor preserve native process ownership.
Missing optional DM reset is genuine success. Native record8 is unwritten by
004E09D0/malloc; use a produced handle or actual empty-list proof, otherwise fail.
Constructor004E08C0 also leaves smoke14 unwritten; missing-model positive smoke
emission fails at that boundary. **These omissions are confirmed; do not re-audit
without contrary evidence.** Native100 repeated packet/DM families plus separate
actual particle retirement and sound-slot audits PASS; endpoint doubles explicit.
Strict full World i386 and final launcher/probe PASS `human-car-dm-presentation-*`,
`human-car-particle-retirement-*`, `human-car-dm-sound-*`. No Windows output run.

Preceding **cfd8aa3** [Car blast receipt](analysis/checkpoints/p2.2-human-car-blast-damage-20261112.json),
[contract](analysis/human-car-blast-damage-source.md): actual004DBA10 RET24 binds
SAME Car/body/wheels/deformables/embedded24E0 DM. Source faction2 policy,254A gate,
24B0 before Scene104 pick, part28/24/WORD38/dirty events, wheel radial Y/Z gates,
DM zone/seat damage and2350=mass_changed. Car feature uses254A; seat engine stop
instead uses234F/core22BF=engine_transition (including runningfalse). Glass event
is0, ordinarypart1, wheelpuncture3, destruction4, explosion5. Event writers and
004D9A40 absent frozen ZERO credit. Terminal destruction/full visuals remain required.

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

1. Finish **004DBF00** authority destruction and004DC580 nearby Human damage,
   then **004DC780** presentation, timed-DM frame and other reached receivers.
   Indexed/new originals are already in WORK:
   `human-car-destruction-dependencies-index.txt`,
   `human-car-destruction-inner-index.txt`,
   `car-actor-dm-water-required/004dbf00.{c,json}`,
   `human-car-damage-presentation-workers` (004DC580/004DC780/004E2000/00500DA0/
   0051A880/004E1F70), `human-car-destruction-mesh-workers` (004E9DB0/004DCD10).
   Raw **`human-car-destruction.asm` now covers FULL004DBF00..004DC580**, and
   `human-car-destruction-human-area.asm` covers004DC580. C aliases remain bad.
   Preserve254A→wire4→clock/flags/control/fuel/health→Script→reverse seat damage→
   model254C release→forward max6 wheel CRT/RandFloat/detach→experience→late2548→
   area damage. Normal source is lookup24B0, water uses driver24AC; normal victim
   is hostility OBSERVER after damage. RandFloat is ENGINE (existing Frames
   volume_random_unit), separate from CRT.734/core6A4 is cold NULL callback.
   004DC780 first calls actual004E9DB0 physical reset, then004DCD10 mesh work;
   positive glass/Item12 detachment remain guards. Inspect these existing originals
   before implementation; use index before any additional exports.004DBEE0/D8 is
   not reached by blastCar4 and needs no speculative recovery.
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
