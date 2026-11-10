# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,852 partial
(1,847 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Player/first-person loading composition](analysis/first-person-loading-composition.md),
[receipt](analysis/checkpoints/p2.2-player-first-person-composition-20261110.json).
Actual source initial player mode0/secondary/action/zoom receivers; same session
OverlayOwner FPV model/items/SOUND notification/playback/view ownership/reset;
real current-Player/WorldActors/Scene2/inline/B0/sector/target-retirement lookups.
Native focused mode/Look/lookup sanitizers and strict actual-host full FPV
composition/app link,27 tracker tests pass. Full FPV composition/reset is compile-
only; DD0/equipment/containing composition/SOUND/cursor endpoints declarations.
Six new partials/four extended, no complete promotion. Four temporary artifacts
hashed/removed. No application readiness/desktop claim.

Application+0C/008AE908 owns ONE BYTE initialized0, independent of profile
settings and live FPV58. Player005469E0 stores52C before local gate/callbacks;
initial old-1→0 reads that preference, enables ordinary view, resets zoom and530.
Mode1 holster kind14 calls actual inline command17/model; leaving1 calls18.
Static ASM: command kind1 at+8, action17/18 at+C, not genuine default no-ops.
Inline005ABB50 updates optional typed Look04 through0077F710 then scalar48
through0077F050. Null model retains04; D0/D4/B0 and40/C8 flags preserved.
Modes2/3 require actual process stage41 guard match; never force equality.
FirstPersonPlayersLoading uses actual world checked per-Human map; target
retirement captures world/actors and must run before their destruction.
Occupied C4/8C and nonoffline secondary command2 queues remain explicit.

Previous5cf27a0 [script runtime/objectives](analysis/mission-script-runtime-composition.md)
composes actual VM/module, objective88, ReturnEnd/EndScript and camera look.
Tables006E9EA0 use896 CRT draws; objective stage13 indices are WORDs, compared
values DWORDs. Startup tables remain uninitialized; real observed publication
required. Original-address integrity VM excluded from accepted startup scope.
008692D0 normallyfalse; only confirmed direct writer00424900 scopes reset.
Advance is override OR game28. Module loaded10 can latch after bad binding;
containing loader must inspect failed_bindings. F9 uses directPC++, unlike normal
advance. Look288 rotates actual scene15C through0C before main240 update.
Previousad54f4c [initial controls](analysis/mission-script-initial-controls.md)
recovers same-Human eventEC/Guard queues/five controls, delayed suspend, Sight/
SOUND, retail output/views. ContextEC is events[4], not executingD8 or pending24.
Previous1b703b8 [Human ownership](analysis/human-world-composition.md) owns actual
DD0/E0/230/B0/inventory/activation;99c8688 recovers cold Car activation.

## Next — same entire checkbox

Actual containing player presentation/HUD soldier/compass binding/removal,
remaining Human/inline/FPV/HUD/SOUND/VM/world/session ownership, required
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

Fresh required HUD exports in WORK: player-mode-and-soldier-receivers/
0049B7E0/0049B870/004B3C60/0049F1C0/00493D10; soldier-bind-first-frame-required/
004B3890/004B47D0/004B4860/004936C0/00493F40/005A78F0/005A6600;
soldier-human-name-modes/005A8380/005A81C0/005A88B0/005A88D0. Jobs clean/completed.
004B3C60 calls name/rank, photo/weapon caption, VIP/stat7 flags and AI textures.
005A81C0 offline profile user14 else LAST name04 (not005A82C0 short10).
005A8380 profile.field24, absent profile0.005A88B0/D0 return-1 unless B0kind3,
then virtual13C/140 still need actual ActiveAI getter targets.005A78F0 searches
DEFINITION identifier (not item instance identity), holder0/2/3/4/5 then reverse
entries in ascending holster/pack stacks. Rank localization table8AE850 stride16
and actual getters/marker dependencies still need confirmation before coding.

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
