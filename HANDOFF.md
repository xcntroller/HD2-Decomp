# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
agents/goals, original EXE/DLL/Proton or Windows fixture execution. Preserve
assets/user changes. Heavy builds/Ghidra SEQUENTIAL (memory crashes), focused
checks, no permanent tests. Interactive changes need user desktop acceptance;
source-only work needs no invented desktop test.

GameApplicationState still DECLINES loading. No loaded world, first frame or new
desktop acceptance. Ledger34 complete/1,944 partial(1,939 client/five engine)
/18,783 frozen. Zero complete promotions. Both P2.2 checkboxes remain OPEN.

## Latest source checkpoint

[World actor consumers](analysis/world-actor-loading-consumers.md),
[receipt](analysis/checkpoints/p2.2-world-actor-consumers-20261110.json).
WorldActorLoadingConsumers now supplies common formerly-declared methods with
SAME actual Startup/Scene2/Items/Human factory/Players/Script link/SOUND/HUD/
proximity/Car/packet borrowers. SceneActorLoadingRuntime now composes with it.
Actual locked control A2/3C writes, derived script actor lifetime guard, HUD
selection pair/command1C actor cleanup, dynamic-item contact SOUND005B7570.
HUD10B3C is COMMAND10B20+1C, NOT Signals; exact assembly retained. Main roll
2F00+88 cold, positive roll/signal rebuild still rejects/no producer.
Strict actual spawn composition found compass0049F1C0 needs005A78F0 INVENTORY
DEFINITION9 lookup, NOT player action. Source now uses SAME Human/Items lookup.
Offline spawn borrows signed Startup local identity; PreparedSpawnPackets reads
SAME session.words1C, initialized0 by00698510, distinct from session.status04.
Startup lends SAME control records with callers' lock0. Strict actual graph now
constructs real source WorldActors/common Consumers/SceneActorLoadingRuntime/
VM/router AND actual offline presentation/PreparedSpawnPackets/decoder/camera
refresh. The old declared common Consumers/Spawn decoder are removed. Car
factory and containing aggregate/HUD world data remain borrowed, not real
application instances. Native focused ASan/UBSan checks all contracts/failures/
repeat and actual game1C direct/broadcast recursive locks; strict i386/app link/
27 tracker checks PASS. One new partial; existing HUD/context/spawn claims
corrected/extended once. Completed audits hashed/removed. Active next ignored
fixture WORK/checkpoint-car-callback-host.cpp.

Prior21b5c7c [module dispatch](analysis/module-loading-dispatch.md)/
[18 vtables](analysis/module-lifecycle-map.json);6021978
[GEffects](analysis/geffects-module-loading.md);4af20a3
[Versions](analysis/version-module-loading.md);c97d95b
[lifetimes](analysis/module-loading-lifetimes.md);52c5689
[session consumers](analysis/session-loading-consumers.md)/[ShotMgr](analysis/shot-manager-loading.md).
Other Human/VM/FPV/HUD/Players links in RESEARCH.md/loading-modules.md.

## Next — SAME full checkbox

Actual Car callback owner and containing world/session instances, ordered006CA780
loader, first camera/update/render, failure/repeat then user desktop acceptance.
All recovered loading world/VM/router/Consumers are still UNCALLED by application.
GameCarCallbackFrames is abstract: actual actor/item/script/effect/particle
receivers remain to bind. Do not add successful placeholder services.

Ordering: +14/+1C/+18 fresh ASCENDING;0048DB90/+20 fresh REVERSE. Original
00698F80 does presentation finish→dubbing close→authority finish→HUD close.
Reserved Scene2 Humans can die before Script4 contexts exit. Derived actor
access now checks SAME retained-frame EC0 before dereference, rejecting retired
borrowers. Do NOT move VM finish ahead of native module order as a lifetime
substitute. Final aggregate destruction still retracts VM and remaining Human/
FPV/camera/dubbing borrowers before engine scene disposal. Positive Exit blocks
requiring retired B0 remain an explicit required receiver; no successful bypass.

Same EC/roster/grid/pools/cache, process CRT/mode/config/registrations; Human
Frames=GameActorServices sharing GameSceneActors. Script bank persists and
standalone kind1/campaign bank selection follows context begin. Fresh B0 clears
stance74/78, previous-null Area8 skips B4. Positive stance/area/Dog534/nonoffline
friendship reject. Mode0 Fowl transfer/grass/positive particle producers remain
excluded; preserve honest first-frame gates and live overlay precedence.

Guard: Startup tables still UNINITIALIZED; private observed publication unresolved.
006E9EA0 initializer896 SAME CRT draws, WORD indices/DWORD expected+observed.
API payload order00698AAE ticks,00698C09 IsWindowVisible,00698C5D GetCapture is
source. Do NOT put raw API values in observed slots or force equality.
Original-address/per-thread VM outside accepted startup scope; actual meaningful
observations remain required. Initial114 bindings/Block controls/faces/camera/
tactical/objectives source supplied; later NPC CreateItem not initial prerequisite.
Live file overlays authoritative. BASE ISD0 fixtures never establish ISD1 Patch/
language precedence/absence; see WORK/geffects-archive-census.json.

## Execution

WORK=analysis/work/mission-world-20261103 (ignored durable, not/tmp).
Client SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Native sanitizer libs .tools/hd2-sanitizer-runtime/usr/lib64; do not reinstall.
i386 .tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Ghidra JAVA_HOME=/home/control/.sdkman/candidates/java/25.0.0.0-oracle;
project /home/control/Documents/Projects/HD2ResearchProject HD2Research.
Read-only/noanalysis/fresh output. Check ERROR even if exit0. Check frozen entries
FIRST; absent confirmed entries use static instructions/transient definition.
Default sandbox reads/edits; git/Ghidra/native run escalation. Tracker python3
tools/function_progress.py --render/--check; python3 tools/test_function_progress.py.
