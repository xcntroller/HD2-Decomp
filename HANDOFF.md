# Session handoff — 2026-11-09

Continue FULL P2.2: first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue after dependencies/checkpoints until the full checkbox is
done or concretely blocked. Larger verified subsystem commits; consolidate
notes/tally there. No push, subagents/goals, original game/DLL/Proton or Windows
fixture execution. Preserve assets/user changes; focused verification only.

GameApplicationState still declines loading. No live world, rendered first world
frame or new mission desktop acceptance. Ledger:34 complete/1,632 partial
(1,627 client/five engine)/18,783 frozen; zero complete promotions.

## Latest verified subsystem

[Script compiler/context source](analysis/mission-script-loading.md), following
camera/local-dispatch commit ceaf501. Actual ordered command/signature/predicate
tables, lexer/pools/expressions/control-flow compiler, retained frame EC word1
and pool references, one-instruction tick/delay/periodic scheduler, events,
Block/exit, signal masks and binding compile/begin/failure/remove lifetimes.

[Receipt](analysis/checkpoints/p2.2-script-compiler-context-20261109.json):75 new
source partials; native ASan/UBSan/LSan, strict actual-host i386 instantiation,
app compile/link and27 tracker tests PASS. All114 captured base Tutorial
bindings compile;145/146 base ISD0 scripts compile (4481 instructions). Unbound
T_EE_Weather.scr safely rejects an unsafe raw-minus/operator fold at byte1443.
These are NOT runtime loose/Patch ISD1 captures or world execution. Ten temporary
fixture sources/binaries/objects hashed and removed; original exports/captures/
logs remain. Source World receivers/factory/notifications are required external
interfaces; boundary doubles and declarations earn no receiver/desktop credit.
00428490 is absent frozen inventory and receives no credit.

## Next — continue the entire checkbox

Implement actual remaining base00416E10 commands and world predicates0041E3F0,
Human00421C00/Dog00421970 derived receivers, asynchronous/deferred operations,
module activation/global variable stores and actual runtime Files selection.
The compiler/context source is not a successful full VM or activated module.
Do not insert successful default receivers. Source VM switch is two stage:
use script-base-vm-jump-map.json, not decompiled case numbers as opcodes.

Original004167B0 executes ONE instruction per tick. Periodic checks are strict
signed accumulator>500 / when.elapsed>period, adding elapsed AFTER checking.
00428490 actual module tick (assembly-validated, absent inventory) traverses
contexts backwards once, optionally byte9 cutscene-only. It first drains deferred
frame restores and a timer.00416CF0 advances only with authority and actual
008692D0 or game28 gate. Pending async stream PC24/28 is explicitly unsupported.

Remaining full checkbox: containing host world/session/Human/FPV/HUD composition;
SOUND portals/subtitles/effects; actor/physics1/3/4/6; missionA8 objectives/zone37
A4 identities; first frame, ordered failures/repeat and user acceptance. Runtime
actors before physics; descriptors before buckets; entries before Items.
Canonical notes: [world](analysis/mission-world-loading.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md).

## Evidence/execution

WORK=analysis/work/mission-world-20261103 (ignored durable). Script original
export directories/unsafe probe exclusions and contracts are listed in the new
script note. New original exports: script-execution-controller-required/,
script-flow-pc-required/,script-module-tick-required/,script-events-required/,
script-signal-flags-required/; all valid exports complete, no process pending.
Lexer/operand/table/group research is already implemented/verified; do not redo it.

Ordinary shell default sandbox. Ghidra config writes and ASan/LSan execution need
escalation; one Ghidra process/read-only project/fresh output; check ERROR even
if exit0. Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Native sanitizer libraries:.tools/hd2-sanitizer-runtime/usr/lib64 (verified,
do not reinstall). i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
