# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,833 partial
(1,828 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Initial Human/VM/view controls](analysis/mission-script-initial-controls.md),
[receipt](analysis/checkpoints/p2.2-script-initial-controls-20261110.json): same
Human context/Guard queues, five reached Human commands, real delayed suspension,
Sight toggle/lost callbacks, actual process SOUND registry, retail empty output,
camera/face/tactical receivers. Focused actual-source native sanitizers, strict
actual-host i386 and app link,27 tracker tests pass. Explicit engine/inline/
identity/presentation doubles; required containing consumers still declarations.
22 new partials; absent005786C0/00465B20 earn none.14 temporary artifacts hashed/
removed; captures/exports/logs remain. No application readiness or desktop claim.

ContextEC is events[4], not executingD8 or pending24.005518F0 replays2AC/B4 into
FIRST command queue then nine recognized Script interrupt masks into distinct
processed SECOND queue. Guard mode0058F330 records request8000; do not switch B0
immediately.0058FAA0 only offline7; non7 command16/+FC rejects before mutation.
SetCameraFrame virtual74=00465B20 sets borrowed288 plus radians28C (angle-1 for
this opcode), distinct from retained248 and0046C370.0069BFE0 is single C3 RET.
Containing runtime must try base00416E10 before derived00421C00 receiver.

Previous1b703b8 [Human/world composition](analysis/human-world-composition.md)
owns actual DD0/E0/230/B0/inventory/activation; WorldActors type3 dispatch and
per-Human lookup, shared SOUND routing and pending EC binding notifications.
Previous99c8688 [Car activation](analysis/car-activation-loading.md) owns source
cold type4 composition. These do not enable the application's world path.

## Next — same entire checkbox

Finish actual objective88 first-tick receiver, ReturnEnd/EndScript and reached
initial camera look-target update; then real Human/FPV/HUD/SOUND/VM/module/session
ownership, ordered006CA780 loader/postactivation, first camera/update/render,
failure/repeat and user desktop acceptance. WorldScene/WorldActors still
uncalled by application. Actual Consumers.make_human_loading_owner, presentation
and VM consumers must use real containing dependencies; no successful substitute.
Retire Human/B0/VM before world actors, before entries/models/tree/session.
Do not expand all P2.3 simulation/opcodes into P2.2.

Static census:145/146 captured programs compile;114/114 initial binding names
compile. First synchronous Block reaches five Human controls, face texture,
three camera commands, tactical and objective status. NPC1..4 CreateItem scripts
are later assignments, not initial114. Actual overlays/resolver authoritative.
Captures script-start-census.json / script-human-command-labels.json / base jump
map / human-suspend-vtable-targets.json are in WORK. Human float switch labels
must decode bits against COMMANDS only; predicates overlap opcode numbers.

Objective research already exported cleanly:
WORK/script-initial-suspend-objective-dependencies/0069ba60.c,
script-initial-camera-objective-required/0069bc40.c and
script-initial-camera-target-objective-state/{0069beb0,0069bed0,0069bfd0,0045f590}.c.
0069BFD0 is genuine RET. MissionA8.related is native34 objective vector;
0069BEB0 changes child.word10 and latches child.flag15 for status1. Initial
TUT_Objectives Block sets statuses1..7 to0.0069BA60 first compares process guard
bytes8CA730[index8CAB4A] /8CAE30[index8CB24A] and conditionally draws rand before
status mutation. These arrays/indices are BSS, not file-backed static data.
Do not bypass: recover actual producers. Read-only HD2ObjectiveGuardReferences
job/session46521 is running or complete; check WORK/script-objective-guard-
references.log and completion before another heavy job. Exports include actual
full offline objective completion/failure, HUD result and Players behavior.
Return/End exports: script-flow-pc-required/00416db0.c,
script-human-end-state-required/ and script-cold-item-end-required/.
Detailed pending research WORK/HumanContextPendingNotes.txt contains earlier
hypotheses corrected by the checkpoint contract; prefer the latter.

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
