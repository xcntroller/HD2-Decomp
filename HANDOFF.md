# Session handoff — 2026-11-09

Continue FULL P2.2: first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue after dependencies/checkpoints until the full checkbox is
done or concretely blocked. Larger verified subsystem commits; consolidate
notes/tally there. No push, subagents/goals, original game/DLL/Proton or Windows
fixture execution. Preserve assets/user changes; focused verification only.

GameApplicationState still declines loading. No live world, first rendered world
frame or new mission desktop acceptance. Ledger:34 complete/1,651 partial
(1,646 client/five engine)/18,783 frozen; zero complete promotions.

## Latest verified subsystem

[Script module/visibility receivers](analysis/mission-script-receivers.md), after
compiler/context commit d49cb63: frame lookup/child/signal/ScriptAssign, selected
player range/dead/live predicates, script variables; real broadcast SetOn/FIFO/
decoder; retained frame/light snapshots and single-frame restore; SAME Human
DD0/230/FPV visibility orchestration, GEffects recursive visibility/fade source;
animation BODYCOLB aligned descriptors with actual dynamic buckets and dummy or
driver aggregate bounds; module activation/reverse tick/finish source subset.

[Receipt](analysis/checkpoints/p2.2-script-receivers-20261109.json):19 new source
partials/five extended; native ASan/UBSan/LSan, strict actual-host i386
instantiation, app compile/link and27 tracker tests PASS. Temporary audit sources,
binary/object hashed and removed; original exports/logs retained. Boundaries in
fixtures include engine/world and Human inline/FPV doubles/declarations, not
implemented containing services.004270D0/00427B80/00428490 absent frozen: no credit.

## Next — continue the entire checkbox

Implement remaining actual base00416E10/world0041E3F0 and Human00421C00/
Dog00421970 derived receivers; async/deferred/rotation/animation owners and live
runtime Files selection. Do not invent successful default receivers. Latest
research exports script-frame-transform-required/ contain004274A0 rotation
producer (dedup frame, axis, signed int speed/angle times0080F6E8) and00423B40
single-snapshot restore (now implemented). Rotation's actual tick/retirement
owner00428CC0/00428D00/consumers remains to research before implementation.
FRM_Move77/GetPosition79 are UNHANDLED base cases; require derived receivers.
Use script-base-vm-jump-map.json: decompiled cases are not original opcodes.

Original004167B0 executes ONE instruction per tick. Periodic checks are strict
signed accumulator>500 / when.elapsed>period, adding elapsed AFTER checking.
Begin executes no instruction. Module binds even on compilerfalse but containing
world must inspect failed_bindings before required initialization success.
SetOn decoder lookup is scene BEFORE driver (compiler lookup reverses it).
Snapshots support20/30 only; transform/animation masks explicitly reject.
Self ScriptAssign rejects unsafe retirement of the executing context.

Remaining full checkbox: containing host world/session/Human/FPV/HUD composition;
SOUND portals/subtitles/effects; actor/physics1/3/4/6; missionA8 objectives/zone37
A4 identities; first frame, ordered failures/repeat and user acceptance. Runtime
actors before physics; descriptors before buckets; entries before Items.
Canonical notes: [world](analysis/mission-world-loading.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md),
[compiler/context](analysis/mission-script-loading.md).

## Evidence/execution

WORK=analysis/work/mission-world-20261103 (ignored durable). Latest script
exports/ASM and contract in script receiver note. All exports complete; no
pending process. Do not repeat confirmed compiler/table/snapshot evidence.
Captured146 Tutorial scripts are BASE ISD0:145 compile and all114 base bindings
have source; not runtimePatch ISD1 or runtime selection. One unbound weather
script safely rejects the raw-minus/operator-list issue, not original-run proof.

Ordinary shell default sandbox. Ghidra config writes and ASan/LSan execution need
escalation; one Ghidra process/read-only project/fresh output; check ERROR even
if exit0. Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Native sanitizer libraries:.tools/hd2-sanitizer-runtime/usr/lib64 (verified,
do not reinstall). i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
