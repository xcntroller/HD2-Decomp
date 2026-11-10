# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,894 partial
(1,889 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Human/VM world composition](analysis/human-vm-world-composition.md),
[receipt](analysis/checkpoints/p2.2-human-vm-world-20261110.json).
MissionScriptWorldLoading owns actual bindings/controls/lifecycle/views/player
predicates/objectives/visibility/runtime/module; borrows SAME Human/Players/
FPV/HUD/camera/Sight/hearing/SOUND/session/process owners. Requires existing
process Script registration, Startup00982100/008692D0 refs. WorldLink supplies
actual context lookup/authority AFTER module construction, rejects unbound calls,
finishes module before unlink, before Human retirement. HumanScriptContextServices
uses link for both Contexts/Runtime types, removing external VM declaration.
Actor Frames must be GameActorServices, sharing SAME derived GameSceneActors;
SceneActor<GameSceneActors> is not Human's actor backend. HUD hint signed-ID
wrapper uses actual small-panel owner, no flags gate. Focused native link
ASan/UBSan/LSan, full actual i386 Human/VM/module/FPV/SOUND construction, app
link and27 tracker checks pass. No new tally credit. Source-only, no loadedworld.

Previous11fe11e [Human factory](analysis/human-factory-world-bindings.md),
58b1095 [selection/visibility](analysis/human-selection-visibility-loading.md),
4f752cd [speech/dubbing](analysis/human-speech-loading.md),
9621549 [shared consumers](analysis/shared-world-loading-consumers.md).
Actual005AC0F0 controls SAME DD0 A9C shadow, NOT SOUND. Startup owns physical
009832B0/shared FLOAT0084E80C=1.4. Cold primary projections empty; producers
unimplemented. Human228 queue update supplied, live world update unintegrated.
Fresh B0 clears stance74/78; previous-null Area8 skips B4. Positive stance/area,
Dog534/nonoffline friendship reject; guards earn no credit.

## Next — same entire checkbox

Continue actual activation APIs/observed publication and containing session/world,
ordered006CA780 loading/postactivation, first camera/update/render, failure/repeat
then user desktop acceptance. App still declines loading; WorldScene/WorldActors
and source VM world remain uncalled. Do not stop at dependency or expand all P2.3.

NEW targeted static evidence in WORK: session-activation-api-resolution.json,
session-activation-api-boundaries.asm, script-guard-slot13-direct-refs.asm and
script-guard-slot13-api-resolution.json.006989B0 real API payloads:
00698AAE→GetTickCount beforeprogress5=.05;00698C09→IsWindowVisible after5=.8,
GetIGraph virtual48 supplies window;00698C5D→GetCapture beforestatus5/spawn.
006E9EC2 initializer seed also GetTickCount. Original006989B0 export remains in
loader-module-composition/. Existing source activate_offline_session currently
omits those activation API queries; next needs actual payload receivers/order.
Private expected/observed side effects remain unresolved; do NOT assign raw API
values or force equality. Original-address/per-thread VM outside accepted startup
scope. Actual observations still required; no fabricated guard readiness.
Current active temporary session-world host fixture retains full verified type
graph for continuing integration; old checkpoint fixtures removed/hashed.

Previous069a96f [HUD/player](analysis/hud-player-presentation-loading.md),
d6b81e9 [FPV/player](analysis/first-person-loading-composition.md),
5cf27a0 [VM/objectives/camera look](analysis/mission-script-runtime-composition.md),
ad54f4c [initial controls](analysis/mission-script-initial-controls.md),
1b703b8 [Human ownership](analysis/human-world-composition.md),
99c8688 [Car activation](analysis/car-activation-loading.md).
Retire Human/B0/VM before world actors/Items, before entries/models/tree/session;
clear FPV/camera targets before actor retirement. No extra process/roster/cache.
Guard006E9EA0 uses896 CRT draws, WORD indices/DWORD values; real observed producer
required, never production equality. Original-address integrity VM outside
accepted startup scope.008692D0 defaultsfalse, advance=override OR game28;
confirmed scoped writer00424900 reset. Module loaded10 can latch on bad bindings;
actual loader must inspect failed_bindings. Static census145/146 programs and
114/114 initial binding names compile; initial Tutorial Block required controls/
face/camera/tactical/objective bodies source-supplied. Later NPC CreateItem is
not an initial114 prerequisite. Actual Files overlays authoritative.

Canonical [world](analysis/mission-world-loading.md),
[ownership](analysis/world-scene-ownership.md), [modules](analysis/loading-modules.md),
[Human](analysis/human-state-loading.md), [HUD](analysis/hud-loading.md),
[activation](analysis/session-activation-loading.md),
[camera](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md), [receivers](analysis/mission-script-receivers.md),
[sound](analysis/sound-world-loading.md).

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored durable, not /tmp).
Client SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Default sandbox normal reads/edits; git/Ghidra/native sanitizers need escalation.
One read-only Ghidra process/fresh output; check ERROR even exit0.
Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Ghidra project:/home/control/Documents/Projects/HD2ResearchProject HD2Research.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64; do not reinstall.
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Tracker:python3 tools/function_progress.py --render / --check;
python3 tools/test_function_progress.py (not unittest package mode).
