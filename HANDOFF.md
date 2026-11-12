# P22-HUM handoff —2026-11-12

**Worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum`, branch `feature/p22-hum`.**
Created from clean master **2c96a7d** at the user's request. Latest
source checkpoint follows below; initial source baseline b132e6d plus merged UI415e850.
Check HEAD/status; preserve all unrelated changes,
LIFE/UI/PACK/research worktrees and `/tmp/hd2-p22-hum-continue`. Transfer evidence
is ignored WORK `human-return-main-20261111`.
Ignored research is a local copy-on-write snapshot for safe evidence validation;
.tools/ghidra entries link the existing installations. No tracked main edits.

FULL P2.2 / **Human H2 reactions/damage/ragdoll remains OPEN**. User requested a
plan revision because the old checkbox hid many dependencies. The
[revised Human plan](analysis/p22-hum-plan.md) is now the current execution plan:
S01–S14 are completed bounded source checkpoints; D0–D12 are explicit remaining
producer/composition dependencies with exit tests. H3–H6/H8 and external
application/desktop delivery remain separate, visible gates. Do not treat old
“continue” paragraphs in research history as the current task list.

Read `AGENTS.md`, Phase2 section2 of `plan.md`, the revised Human plan and
[Human contracts](analysis/human-ragdoll-source.md). Continue reached dependencies
and verified checkpoints until H2 completes or a concrete blocker prevents it.
No mission application/desktop acceptance is claimed. No agents spawned or push.
No original EXE/DLL/Proton or Windows output execution; native own audits only,
Windows compile/link. Heavy compilation and Ghidra MUST run sequentially.

## Latest verified result

[D2/D3 receipt](analysis/checkpoints/p2.2-human-car-positive-deformation-20261112.json),
[contract](analysis/human-car-positive-deformation-source.md): actual004FF8B0
positive light/part/variant/glass/wheel damage now binds SAME CarColdReceivers,
initializer/body/geometry/paired flags and694/698. Hit58 deformation invokes the
actual loading receiver AFTER existing velocity mutation; no parallel graph.
004FEFD0 RET12 is index/optional impulse/allow-detach;694/698 are FASTCALL/RET8.
00500CB0 preserves mask/texture/update/power order. New frozen partials4FEFD0/
500CB0 plus5 extensions: **34 complete/2999 partial(2994client+5engine)**.
Initial worktree index through external links lacked inventory entries, NOT
proof of absence. Local copy-on-write research snapshot now validates all evidence.

**Ownership correction:** FA8+28 is the existing seat-associated Door, not an
occupying Human.004E2E30 discovery/seat association/part72 binding already produces
it; mutable visual20 changes AFTER selected frame24, and004FE910 restores it
BEFORE variants/visibility even for unoccupied Cars. D3 has no H3 seat-entry
prerequisite. Actual occupied Human/control work remains H3/D5.

**D1 correction:** retirement used the keyed mesh restore, but physical reset's
lambda still omitted resource. Both004EC5C0/004E9DB0 now share ONE geometry/
resource/Door restoration helper. Previous603dd1b [D1 source](analysis/human-car-mesh-backup-source.md)
remains linked. Raw mesh hit sum association is now(X²+Z²)+Y²; lights/wheels
use(Z²+Y²)+X². Light amount is FLOAT-spilled ONCE before the loop; wheel health
compares the unspilled result and detaches only when strictly NEGATIVE.

Native100 D1 regression+100 positive families PASS ASan/UBSan/LSan, actual Door
producer/variant/restore, child/glass/callback/paired state and failure prefixes.
Explicit engine/frame/physical coefficient/glass-name/hide/detachment doubles.
Decimal100 light427F7084 versus wheel427F7083 and previous XYZ golden PASS.
Strict actual nested World/Car/Human/module13/loading hooks and final new-worktree
launcher/probe PASS. No application/desktop claim. Temporary audits stay ignored
until H2 closure. S01–S14 and D0 remain verified; don't restart unchanged kernels.

## Next work: D4, then reached D5/D10–D12

Follow the [Human plan](analysis/p22-hum-plan.md) and [D0 path map](analysis/human-reaction-containing-boundary.md).
D0–D3 are source-complete. Continue dependencies/checkpoints until full H2 closes
or a concrete blocker prevents it; D11 failures must remain honest. No fake
transport, owner, initialization or successful unknown callee.

- D4 authority frame is **004D2790**, NOT presentation004D3110. Its initial
  0073EA90 gate publishes event6, conditionally calls004DB2A0 when !mode&2,
  then clears actor2478. Pending target calls004DEC10 BEFORE water/timed DM.
  Presentation004D3110 owns other non7 gates/seats/navigation/queued voice.
- Reuse WORK `human-car-frame-events-index.txt`,
  `human-car-authority-frame-abi-index.txt`, `human-car-frame-consumers-index.txt`.
  Existing originals `car-activation-required/004d2790.c`/`004d3110.c` and
  `human-car-damage-event-readers/004db360.c`. Case6 calls004DB2A0 without a packet
  argument; case7 invokes004DE1F0(packet,~mode&1). Resolve actual grammar/owners
  before implementation. No new D4 Ghidra export yet.
- D2 continuation evidence is preserved: rollback `ExportHumanCarVariantFinalABI.java`
  →`human-car-variant-final-abi` is current. Earlier impact-damage-abi's004FEFD0
  one-arg/RET4 and initial restore-impact-abi's694/698 thiscall are WRONG.
  Use raw `human-car-positive-variant-worker.asm`, `human-car-positive-light-worker.asm`,
  `human-car-deformation.asm`, `human-car-part-restore.asm`.
- D5 occupied/non7 Car loading/driver/mounted input remains required. D10
  unmounted reaction composition can advance independently of full transport.
  UI source is merged/source-closed; actual Human/Actor producers/frame callers
  and application activation remain external.
- D6–D9 remain actual entry-specific work. Reuse `human-entry-connection-originals`
  and [shared entry contract](analysis/multiplayer-world/shared-world.md).00730A40
  needs peer6C/client334 wrappers/worker lifecycle. Registration underB4 stores
  connection08=workerD4 before append; swap-last unregister doesn't delete.
  Client00723D20 publishes334 before port/register; host007260A0 registers after418
  unlock. Status9 restores retained game BEFORE menu shutdown; received0 replaces
  input manager BEFORE old-world retirement; failure deletion differs. Voice final
  ABI-v3 has EIGHT args/interface18/publish0C before init/client before server close.
  Live synchronized multiplayer acceptance remains Phase3.

D11 confirmed unproduced states remain honest failures: part5F9, wheel695,
EXPL record8 and smoke14. Part004CB2CD reads/toggles5F9 before movement; ctor/
reset/launchers leave it unproduced. Native warm checks explicitly seed prior
state. Do not invent defaults or repeat completed audits without new evidence.
A reached required failure blocks closure even when its failure behavior is known.

## Commands

WORK=`analysis/work/mission-world-20261103`; original
`analysis/work/hd2.unpacked.exe`, SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
Use `python3 -B tools/research_index.py find ADDRESS...` before exports;
`neighbors ADDRESS` has no`--count`; candidates aren't equivalence proof.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject`/HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
ExportFunctions refuses existing output dirs; inspect ERROR even on exit0.
No heavy compilation overlap. `objdump -s` ignores bounds: bounded PE reads only.
