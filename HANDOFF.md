# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,871 partial
(1,866 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Shared world loading consumers](analysis/shared-world-loading-consumers.md),
[receipt](analysis/checkpoints/p2.2-shared-world-consumers-20261110.json).
Three focused native ASan/UBSan/LSan audits, strict actual-host i386 composition,
app link and27 tracker tests pass. Actual frame/engine/containing world/DD0/
Players/camera/SOUND boundaries doubled in native; containing aggregate,
presentation endpoints/animation callbacks/VM/models/nonoffline records remain
required declarations in host compile. Source-only, not complete Human activation.

0049FBF0 actual inventory root E0bit0 hidden RET now routes same presentation→HUD.
Visible004A49F0→004A26D0 interactive inventory explicitly rejects; normal bind hides.
SceneActorLoadingRuntime uses SAME checked module EC lookup/Use event0041D960,
unused pushed state/RET4; actual Switch flags1/group1/entryD8/world position.
Human teardown actual proximity/nonoffline-record endpoints, no substitute map.
WorldSoundLoading calls SAME Startup.emit_game_sound; actual WorldActors checked
composition/DD0 sector, B0 flags50 bit18, camera sector24C and byte270. Camera
virtual7C00465070/virtual8C004650C0 absent frozen, no credit. Getter address
comment corrected from unrelated00465AD0. Camera export had missing-function
error; static PE/vtable/objdump verified getter bodies instead.

HumanWorldLoadingServices supplies common clocks/pools/Look/230/Players/FPV/
faces/animation/music/SOUND callbacks, NOT every derived AI/Sight/speech/area
consumer. It uses Dependencies::Composition through checked WorldActors map.
007855C0 DD0DC4 initialized0/0075B280 same-byte read now owned in DD0, replacing
an unnecessary external human_collision_disabled service. Control process's
SAME00982104 local_peer now signed32 and exposed by Startup; no per-world copy.
006B66B0 config04nonzero AND224==3;006B6700 remote26C first, else offline7 lazy
profile34 else3.0069A4B0 actualFPV58 OR59.0045E920 reverse selection identity
includes zero-ref/dead/timed records until erasure; differs from0045E960.
005A6380 health/nullB0/property19C1 false else B0flags50 bit26.0049B980 actual
HUDflags→compass selected append/retire now has source body.
Six new partials; retracted005A8740/005A86F0 abstract-callback-only prior credits.
No broad audit: targeted discrepancy on next dependency. See linked contract.

## Next — same entire checkbox

Actual containing Human/FPV/HUD/SOUND/VM/session aggregate and reached remaining
Human/Players/Sight receivers, required integrity observed publication, ordered
006CA780 loader/postactivation, first camera/update/render, failure/repeat then
user desktop acceptance. WorldScene/WorldActors remain uncalled by application.
Do not stop at a dependency; do not expand all P2.3 simulation/opcodes/inventory.
New common Human bridge still needs player_add_enemy/player_remove_enemy,
queue_active_ai_speech/queue_player_ai_speech, stance/area and Sight feedback
receivers to instantiate complete actual Human factory. Source existing
HumanSelectionRecords acquire/release owns enemy records, but its show/hide
feedback is not005A8740/005A86F0's body. Loading notification flag startsfalse;
trace reached branches before expanding positive Human fades into P2.2.
New exports human-shared-player-feedback-required/{0049b980,005a8740,005a86f0,
005383d0}.c and human-feedback-consumers-required/{005882f0,005ac0f0,0078b180}.c
are researched ONLY; speech/visibility consumers not implemented/credited.
005383D0 checks B0flags50 bit40000 then group code adjustment/process query and
calls common005882F0. Common null speech_definition is genuine return0; populated
path005AF8E0/005AFD10/00588170 and shared SOUND remain to recover.00542060 export
already in human-player-sight-speech/. Existing confirmed Switch/runtime/portal/
DD0/FPV code needs no repeated broad audit.

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
