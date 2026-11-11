# P22-HUM handoff — 2026-11-11

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue through dependencies until the full selected checkbox completes or blocks.

## Active checkout

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** The user
explicitly requested returning here. Transfer base was50f9193; inspect current
HEAD/status before acting.22 pending Human files were verified and transferred
from `/tmp/hd2-p22-hum-continue`. Four older Human changes were already present in
683cb7b; the newer versions include those changes. Newer GUARD source/docs remain
intact. Ignored WORK `human-return-main-20261111` retains before/after files,
hashes and HEAD. Temporary checkout is an inactive backup, not the working root.
Do not edit the separate LIFE/UI/PACK worktrees or discard their changes.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra must run **sequentially** for memory. No push.
Source-only subsystem checkpoints may be committed; interactive acceptance still
requires the user. Application mission loading remains disabled. Both P2.2 boxes
and P22-HUM remain open; no mission desktop verification has occurred.

## Latest source checkpoint

[Damage/scoring receipt](analysis/checkpoints/p2.2-human-damage-presentation-20261111.json)
adds32 frozen client partials and extends four existing partials, preserving GUARD
credit. Totals:34 complete/2570 partial of18783; no complete promotions.
Earlier accumulated joint/clip/selection source retains its uncredited status.
[Particle/surface receipt](analysis/checkpoints/p2.2-human-particle-surface-20261111.json)
is the previous dependency checkpoint. Full contracts are in the source note.

Actual authority/received damage, death/drop inventory, kind1/10/16 packets,
Player B088 death/respawn HUD, configuration policy, localized hit/death messages,
SAME process player/team scores/rank and auxiliary death selection are now in
source.0072EE60 is scoring, not packet publication.005A0DA0 uses SOURCE's local
player predicate. Presentation uses88 (00583940/0053BE10), authority uses84.
00518710(false) closes the reached destroyed-Car stop transition; broader Car
start/running/occupied simulation remains separate. No successful missing owner.

ControlStartupRecord3C is a typed borrower, not a DWORD cast or another Human
map. **Its ordinary nonoffline00612A60 writeback producer remains open**. Peer
slot and Scene2 actor identity differ.0072E7E0 resets7C, not B8; team44C/464 fields
are loader-zero-filled in the original .data tail. Rank stats preserve health and
indices3/5. Player writes54 before stats/publication; Remote must use its old rank.

## Immediate next work

1. Reuse existing `feature/p22-pack` source rather than duplicating its23 codecs,
   discard grammars, actual command/packet owners and callback lifetime checks.
   Read its plan/implementation through Git. Local commits2316b73/287c6fc/31b8786
   are NOT merged into master; do not overwrite their work. Their damage/drop/
   rank effect gaps can now bind these actual Human bodies. Preserve distinct
   Player/Remote ordering and copied display values before callbacks retire owners.
2. Continue Player0053BD00 byte550 objective tail: actual nonoffline objectives
   are **GameData kind12**, with native28 selector and18 side. Current shared
   GameDataNode lacks that metadata producer. Reuse P2.8 metadata research before
   implementing; do not reinterpret offline kind4 fields or invent successful
   objective transport. Exact first-objective side quirk is in WORK export.
3. Actual peer removal0072F2F0 reaches application11/0072A0B0 transport; nonrender
   messages reach0069F1E0/00744870 console window/process (NOT networking).
   These reached branches still require real owners. Continue through full
   containing frame/damage failure, repeated entry and ordered retirement.

Existing007B3C80 clip collision/transitions,007A7470 containing ragdoll, actual
rest007B37C0→DD000787110→ShotMgr A0, actual0074D510 DD0+AA0 selection and damage
controller remain preserved and verified as described in the contracts.
Normal184=5/188=NULL is reset, not a missing authored loader:00761690 is saved
DD0 restore via00784310/005AB7A0. Full persistence remains P2.6.

## Verification and research

WORK=`analysis/work/mission-world-20261103`. Focused native ASan/UBSan/LSan logs
PASS: death-inventory, damage-presentation, damage-policy, damage-messages,
damage-score, score-presentation, auxiliary-selection, car-engine-stop and
state-damage (all prefixed `human-`, suffixed `-native.log`). Explicit engine/
Scene/query/platform doubles; Car stop audit seeds real slots with access control
disabled, no resource-loading equivalence. Temporary audit files are ignored,
fingerprinted in receipt and retained only for this ongoing full-checkbox work;
remove them at full completion. No permanent tests.

Strict actual World i386: `human-damage-score-presentation-host-i386.log` PASS.
Latest main launcher compile/link: `human-damage-score-main-application-i386.log`
PASS. Outputs were NOT executed. Tracker render/check and diff checks required
at checkpoint. Do not repeatedly audit unchanged verified solver/particle source.

Before new exports use `python3 -B tools/research_index.py find ADDRESS...`.
`neighbors ADDRESS` has no `--count`. Candidates are not equivalence proof.
Client=`analysis/work/hd2.unpacked.exe`, SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
Latest repaired exports: damage-presentation ABI-v3, damage-policy ABI-v3,
damage-messages ABI-v2, score ABI and rank-presentation ABI. Raw rank FTOL
instructions remain authoritative where C loses x87 arguments. Logs retain hashes.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra JAVA_HOME=`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
Inspect ERROR even on exit0. Never overlap it with heavy compilation.
