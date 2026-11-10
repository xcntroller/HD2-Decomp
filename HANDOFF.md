# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,846 partial
(1,841 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Script runtime/objectives/lifecycle and camera look](analysis/mission-script-runtime-composition.md),
[receipt](analysis/checkpoints/p2.2-script-runtime-objectives-20261110.json).
Actual source Runtime/Module composition, shared clock/advance gates, objective88
and descriptor/Players/Human experience/HUD result ordering, ReturnEnd/EndScript,
active-camera look-target pose. Native focused sanitizers, strict actual-host
i386/app link and27 tracker tests pass. Engine/CRT/file/frame/clock/presentation
and lifecycle-cleanup boundaries explicitly doubled; containing consumers still
declarations.13 new partials/four extended, no complete promotion.12 temporary
artifacts hashed/removed. No application readiness/desktop claim.

006E9EA0 initializes real128-entry scalar process tables with896 shared CRT
draws. Objective stage13 indices8CAB4A/8CB24A are WORDs; compared values are
DWORDs, not bytes. SAME StartupSession owns tables and BSS advance override.
Tables remain uninitialized in app; actual loader/API/integrity observed-value
publication still required. Never force production equality or invent successful
protection. Original-address integrity VM excluded from accepted startup scope.
008692D0 is normally false; only confirmed direct writer00424900 scopes reset.
Native advance is override OR game28. Module loaded10 can latch after bad binding;
containing loader must inspect failed_bindings. Runtime tries base before derived;
unknown required commands/predicates reject. Fade finish uses actual Effects8C.

ReturnEnd sets104false/D4true before kind5 Human cleanup without health gate.
F9 storesD4 first, positive-health Human cleanup/state selection, then directPC++
without can_advance/104 modification. Live Dog/type22 attached-current-player
cleanup remains required. Look288 is a borrower distinct from retained248;
00466970 rotates ACTUAL active scene camera through virtual0C and updates it
before common main240 update. Debug/config8/attached pose remains excluded.

Previousad54f4c [initial controls](analysis/mission-script-initial-controls.md)
recovers same-Human eventEC/Guard queues/five initial controls, delayed suspend,
Sight/SOUND, retail output and view/tactical commands. ContextEC is events[4],
not executingD8 or pending24. Previous1b703b8
[Human/world composition](analysis/human-world-composition.md) owns actual
DD0/E0/230/B0/inventory/activation and precompile notifications. Previous99c8688
[Car activation](analysis/car-activation-loading.md) composes cold type4.

## Next — same entire checkbox

Actual containing Human/inline/FPV/HUD/SOUND/VM/world/session ownership, required
observed-value producer, ordered006CA780 loader/postactivation, first camera/
update/render and failure/repeat, then user desktop acceptance. WorldScene/
WorldActors remain uncalled by application. Consumers.make_human_loading_owner,
presentation and VM consumers must use real containing dependencies. Retire
Human/B0/VM before world actors, before entries/models/tree/session. Do not
expand all P2.3 simulation/opcodes into P2.2.

Static census:145/146 captured programs compile;114/114 initial binding names
compile. First synchronous Block reaches five Human controls, face texture,
three camera commands, tactical and objective status (now source supplied).
NPC1..4 CreateItem scripts are later assignments, not initial114. Actual overlays/
resolver authoritative. Captures script-start-census.json, script-human-command-
labels.json/base jump map and human-suspend-vtable-targets.json remain in WORK.
Human float switch labels decode bits against COMMANDS only, not predicates.

Canonical contracts: [world](analysis/mission-world-loading.md),
[shared ownership](analysis/world-scene-ownership.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md),
[receivers](analysis/mission-script-receivers.md),
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
