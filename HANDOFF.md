# P22-HUM handoff —2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll remains OPEN**.
Read `AGENTS.md`, Phase2 section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[Human contracts](analysis/human-ragdoll-source.md), and the relevant linked notes.
Continue dependencies/checkpoints until the FULL selected checkbox completes or
is genuinely blocked. Source progress does not close application/desktop work.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested transferring back here; transfer evidence is ignored WORK
`human-return-main-20261111`. Preceding checkpoint **e6f08b7**. Check current
HEAD/status before acting. Preserve other LIFE/UI/PACK/research worktrees,
`/tmp/hd2-p22-hum-continue` and unrelated changes.

No original EXE/DLL/Proton or Windows fixture execution. Native own audits only;
Windows compile/link only. Heavy compilation/Ghidra **sequentially**. No push.
Commit completed source checkpoints and continue; interactive acceptance requires
the user. Mission loading remains disabled; no mission desktop verification.

## Latest verified source

[Authority destruction receipt](analysis/checkpoints/p2.2-human-car-destruction-20261112.json)
adds3 partials/extends3: **34 complete/2726 partial(2721client+5engine)**, no
promotions. [Full contract](analysis/human-car-destruction-source.md).
Actual004DBF00 RET4 binds SAME Car/DM/body/wheels/controls/Script/Scene2/random/
streams:254A→wire4→late clock→flags/engine/fuel/health→Script→source lookup→reverse
Human kind5→model254C release→forward max6 wheels→experience600→late2548→area.
Car734/core6A4 callback remains coldNULL. Normal hostility observer is VICTIM;
water source24AC and base30 point reread per occupant. Engine RandFloat is
separate from CRT. Clock0073EAC0 timestamp BEFORE enabled. Original failed-prefix
retry may return on254A without replaying tail; DM64 still publishes afterreturn.
Actual004DC580 traverses SAME Scene2/head260, cached centreY+.5/radius25,
**static-only** ray `(start,offset,NULL,NULL,-1,0)` (zero args are output pointers),
post-ray slot reload, actual random bone point and HumanD4kind2. Radius5's small
negative damage is preserved. Human map-key guards precede source dereference.
Native100 repeated authority/area families PASS with explicit endpoint doubles;
strict actual World/control/Script/ray/Car/Human i386 and launcher/probe PASS
`human-car-destruction-*`. **Queued visual/physical reset, Item12 detachment and
full timed/frame paths remain required; no full Car/Human/application claim.**

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

1. Finish **004DC780** presentation and **004E9DB0** physical reset/
   **004DCD10** mesh/occupant transitions, then Item12 detach/positive glass,
   timed-DM and other reached effects. Original WORK already indexed/exported:
   `human-car-destruction-mesh-workers/{004e9db0,004dcd10}.{c,json}`;
   `human-car-damage-presentation-workers/{004dc780,00500da0}.c`;
   `human-car-reset-dependencies-index.txt` for4C8140/4CCC60/4EA760/507A40/4F5840.
   Current default C aliases in4E9DB0 are badly corrupted by engine math ABIs;
   use raw or a rollback ABI-corrected export before implementation. Entire body
   is2474bytes; includes wheel pose/frame publication, SAME body/cache/motion/
   solver/wheel resets, optional4FEEA0/4FE910 and receiver relinks. Reuse actual
   existing owners. Actual native4DC780 first invokes4E9DB0(speed958,true).
   Keep4DBF00/area/packet/DM checkpoints; no kernel re-audit needed. Use research
   index before further exports, and memory-sequential compilation/Ghidra.
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
