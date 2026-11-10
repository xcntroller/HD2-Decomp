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

[Human factory/world bindings](analysis/human-factory-world-bindings.md),
[receipt](analysis/checkpoints/p2.2-human-factory-bindings-20261110.json).
Actual HumanLoadingOwnerFactory takes SAME scene/collisions/Items/inputs/VM/
profiles/Sight/hearing/Players/camera/audio/presentation dependencies. Startup
owns physical process009832B0 and exposes shared FLOAT0084E80C=1.4. Per-Human
animation markers/player queries now route through their actual DD0/tracks;
shared SOUND50/54, alignment/session34/roster/clothing44, HUD direction and
presentation mission/fallback endpoints supplied. Fixed actual base-actor enemy
replay→Human selection and HUD cursor→actual item-module interface mismatches.
Focused native ASan/UBSan/LSan; strict i386 whole Human construction/bothpasses/
WorldActors dispatch plus real shared animation/hearing/FPV constructors, app
link and27 tracker tests pass. No new ledger credit. Source-only; no loadedworld.

Previous58b1095 [selection/visibility](analysis/human-selection-visibility-loading.md),
4f752cd [speech/dubbing](analysis/human-speech-loading.md),
9621549 [shared world consumers](analysis/shared-world-loading-consumers.md).
Actual005AC0F0 controls same DD0 A9C shadow, NOT SOUND. Human228 queue update
supplied but live world update unintegrated; runtime projection/mark producers
and pools unimplemented, actual cold primary projection list empty.00489180
absent frozen/no credit; static ASM primary when Ghidra has no function.

## Next — same entire checkbox

Continue actual containing VM/Human/FPV/HUD/SOUND/session aggregate, real integrity
observed publication, ordered006CA780 loader/postactivation, first camera/update/
render, failure/repeat then user desktop acceptance. Source WorldScene/WorldActors
remain uncalled by app. Do not stop at a dependency or expand all P2.3.
HumanWorldServices now supplies remaining cold selection/speech/Sight bindings;
00537FB0 populated74/78 and previous Area8 B4 area branches explicitly reject,
fresh state bind clears74/78 and skips previous-area lookup. Dog null534 genuine
RET; populated Dog/nonoffline friendship reject. No credit for those guards.
Source Human factory available, containing actual instance construction remains.
VM Frames must be actual GameActorServices (Human's actor backend), sharing the
same derived GameSceneActors instance; do not invent SceneActor<GameSceneActors>.
Current ScriptContextServices compile check has an external authority/runtime
boundary; replace it with actual MissionScriptLoadingRuntime/Module/Contexts.

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
