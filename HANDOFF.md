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

[D1 mesh receipt](analysis/checkpoints/p2.2-human-car-mesh-backup-20261112.json),
[contract](analysis/human-car-mesh-backup-source.md): actual004FF8B0 first-hit mesh
capture/deformation and004FE910 reverse restore now share SAME physical part/
resource/mesh owners. Native654/658 is an owned per-physical-object vector, not
process global (004E2900 zero initialization,004FC320 ECX=this+654 append).
Record/count/flag4 ordering, LOD0/1 locks/XYZ-only writes, original-relative clamp,
shared CRT, callback/dirty/rebuild and flag-clear/count/release/erase are preserved.
Resource mismatch genuinely skips mesh restore and retains4. Later positive
light/health/variant/glass/wheel damage still fails honestly at D2; FA8+28 occupied
attachment repair remains D3. No successful full positive-impact claim.

Native100 focused families PASS ASan/UBSan/LSan: producer→backup→restore, both
LOD capture branches, repeated/shared-key hits, reverse partial restore, retry/
release and lock/factory/notification/rebuild failure prefixes. Engine/frame/
geometry coefficients doubled; access checks disabled only to seed known physical
fields, no manually seeded backup. Decimal100 golden matches XYZ3FA55876/
C14A137E/42F7D5D2. Strict actual nested World/Car/Human/module13 i386 and NEW-worktree
launcher/probe PASS. ONLY extends existing004FF8B0/004FE910 partials: merged total
34 complete/2997 partial(2992client+5engine), no additions/promotions.

Earlier b132e6d fuel/impulse, S01–S14 and D0 boundary map remain verified. Source
progress is not application loading/desktop acceptance. No pending launcher rebuild
for D1. Preserve confirmed unproduced5F9/695/EXPL8/smoke14 failures.

## Next work: D0 completed; implement the reached dependencies

[D0 boundary map](analysis/human-reaction-containing-boundary.md) records T1–T10,
actual state producers/retirement and mode/occupancy/entry-specific dependencies.
Direct ragdoll→Car004F2790 does not require004FF8B0 mesh deformation; positive
Car→Car/passenger004F2B60 does. Status9/received0 are not callees of every offline
reaction. UI source is merged/source-closed; Human producers/frame callers remain.
D1 is source-complete. Next implement reached D2 positive damage, then D3–D5; D10 unmounted
composition can advance independently of full transport. Check
and commit each verified checkpoint, then continue. Add newly discovered required
dependencies to the plan with a caller and exit test before following them.

D2 continuation evidence:

- D2:004FE910 original is in WORK `car-initializer-mass-required/004fe910.c`;
  default C has broken indirect-call/register aliases.004FF8B0 producer default C
  is `car-impact-typed-required/004ff8b0.c`. Current raw
  `human-car-part-restore.asm`/`human-car-deformation.asm` is bounded disassembly.
- Index receipts: `human-car-restore-fuel-checkpoint-index.txt`,
  `human-car-global-mesh-backup-index.txt`,
  `human-car-deformation-producer-index.txt`,
  `human-car-backup-container-index.txt`. Rollback `ExportHumanCarRestoreABI.java`
  →`human-car-restore-impact-abi`; later `ExportHumanCarImpactDamageABI.java`
  →`human-car-impact-damage-abi` corrects694/698 to FASTCALL ECXcontext/EDXindex
  plus TWO stack args(RET8), not initial invented thiscall extra argument.
  Workers004FEFD0/00500CB0 are indexed in `human-car-positive-deformation-workers-index.txt`;
  their new export is research only until implemented.
- D3: actual FA8+28 mesh+20 repair precedes variant/visibility restore. Preserve
  native reverse matching, LOD lock/unlock, flag4/refcount/release/erase order.
- D4/D5: Car6/7 grammar, periodic/pending damage/SOUND, occupied driver/mounted
  state and non7 loading still have genuine required guards.
- D6–D9: reuse WORK `human-entry-connection-originals` and
  [shared entry contract](analysis/multiplayer-world/shared-world.md).00730A40
  needs actual peer6C/client334 wrappers/worker lifecycle. Worker registration
  underB4 writes connection08=workerD4 before append; swap-last unregister doesn't
  delete. Client00723D20 publishes334 before port/register; host007260A0 registers
  after418 unlock. Status9 restore retained game BEFORE menu shutdown; received0
  manager replacement BEFORE old-game retirement; failure deletion differs.
  Voice final ABI-v3: EIGHT args, interface18, publish0C before init and client
  before server close. Live multiplayer acceptance is not a new Phase2 gate.

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
