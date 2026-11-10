# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,900 partial
(1,895 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Session loading consumers](analysis/session-loading-consumers.md),
[ShotMgr](analysis/shot-manager-loading.md),
[receipt](analysis/checkpoints/p2.2-session-loading-consumers-20261110.json).
Actual activation GetTickCount/IsWindowVisible/GetCapture order with real host
GetIGraph/window endpoints. Mission effects use actual selected files, shared
process definition cache/scene borrower; both captured roots empty201, valid
placed particle producer still rejects. ShotMgr14 now owns actual cold16+16
pools, humanzones/WeaponOdds/models/16textures and ordered retirement. Startup
owns SAME process state; registration14/mode borrowed. No projectile/damage/
rendering or populated record cleanup offered. Focused three native sanitizer
fixtures (ShotMgr ALL40 endpoint failure prefixes), strict i386 actual source
graph/app link and27 tracker checks pass. Six new partials, no completes.
0044DDE0/0044E110 absent frozen: no credit; static ASM used for absent cleanup.
Original exports/failed guesses documented honestly in note/receipt. Temporary
checkpoint fixtures hashed/removed; logs/research retained. Source-only.

Previousde81817 [Human/VM world](analysis/human-vm-world-composition.md),
11fe11e [Human factory](analysis/human-factory-world-bindings.md),
58b1095 [visibility](analysis/human-selection-visibility-loading.md),
4f752cd [speech](analysis/human-speech-loading.md),
9621549 [shared consumers](analysis/shared-world-loading-consumers.md).
Human/VM WorldLink binds actual module/context/authority after construction,
finishes before unlink before Human retirement. Frames=GameActorServices sharing
actual derived GameSceneActors; do not invent SceneActor<GameSceneActors>.
Actual005AC0F0 uses SAME DD0 A9C shadow, NOT SOUND. Startup physical009832B0 and
FLOAT0084E80C=1.4 shared. Cold primary projections empty; runtime producer absent.

## Next — same entire checkbox

Continue actual registered module dispatch/remaining real receivers, containing
world/session, ordered006CA780 loader, first camera/update/render, failure/repeat
then desktop acceptance. App still declines loading. Source WorldScene/WorldActors/
VM are uncalled by application; strict checks borrow WorldActors/Car runtime and
session boundaries, not actual containing instances. Do not stop at dependencies
or expand all P2.3. Active temporary full type graph for further integration:
WORK/checkpoint-module-dispatch-host.cpp (audit not permanent test).

Use loading-modules.md dispatch table and WORK/module-load-activation-map.json:
all19 original registrations. Existing source Track/Physics/Items/VM/Sight/SOUND/
Vertex/Players/actors/camera owners should supply their actual methods. Shot14
now source. GEffects1/Versions7/Particles17 and other remaining reached receivers
still need actual ownership/activation where not yet supplied.8/10 verifiedRET,
18 coldnull. Never add a successful default handler for a missing module.

Activation original API mapping in WORK/session-activation-api-resolution.json:
00698AAE GetTickCount beforeprogress5=.05;00698C09 IsWindowVisible after5=.8;
00698C5D GetCapture beforestatus5/spawn.006E9EC2 seed alsoGetTickCount.
Private expected/observed side effects remain unresolved; tables uninitialized.
Do NOT store raw API results in observed slots or force equality. Original-address/
per-thread VM stays outside accepted startup scope. Actual observations still
required; no fabricated guard or loaded-world readiness.
Fresh B0 clears stance74/78; previous-null Area8 skips B4. Positive stance/area,
Dog534/nonoffline friendship reject; guards earn no credit. Clear FPV/camera/
dubbing before Human/VM/Items and scene as detailed ownership notes require.

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
