# P22-HUM H8 handoff — 2026-11-17

Actual worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`, branch
`feature/p22-hum-h8`. The environment cwd may instead point at retired
`.codex/worktrees/3472/HD2-Decomp-p22-hum-h8` / `feature/p22-hum-h8-4`: always
check and use the actual H8 worktree explicitly. H8.4 through4708ff9 was merged
at906dc1b. Preserve that source and all other worktrees. PACK is finished.

USER: unavailable/asleep; no questions or stopping except task completion or a
hard block on any progress. Continue ALL H8, including unfinished merged H8.4.
After H8 source completion run a P22-HUM verification sweep BEFORE matrix
resolved_source. If still away, continue P22-FRAME on its existing branch/worktree
`../HD2-Decomp-p22-frame`. Check its checkout before acting. No push; never run
original EXE/DLL/Proton or Windows fixtures. Heavy compile/Ghidra sequential.
Source-only work may commit; actual interactive changes require user desktop
acceptance. No desktop test has been performed for H8.

## Latest result

[Projectile source](analysis/human-h8-projectile-source.md),
[receipt](analysis/checkpoints/p2.2-human-h8-projectile-20261117.json): SAME World
all-hit006FF140/static00700BC0/00701350; shell00454360 motion/query/sound;
grenade0044D260/0044D240→0044D290 Body/blast/effects and0044D570 contact;
material00451DB0→0044FBC0 native80 owned projection pool; picked-frame Versions
0043C660 single-Part receiver and004511E0 parser. Native33,408/32 query,
9,901/100 motion/contact/projection and800/100 actual two-Part Versions checks
PASS with explicit endpoint doubles as qualified in receipt. Strict actual
World/factory/frame/contact/effects i386 and launcher/probe compile/link PASS;
exact tracker and27 tests PASS. Counts34 complete/3563 partial
(client3558+engine5), frozen18783. New claims partial only. Temporary audits
fingerprinted/removed; original evidence retained. H8.3 and full H8 remain OPEN.

Important: DOUBLE0080FAC8 is9.99999993922529e-09, not .0001. Shell uses ray
layers1/flags0, .1 speed gate, reflection1.2 and NEGATE helper00454680, gravity3.8;
SOUND sees old position. Grenade common worker runs from presentation in offline7,
Body update before timer/state reads, strict fuse<age and close-before-effects.
Material2C is UNSIGNED integer threshold, not FLOAT. Native80 projections have
no tick; exact maximum recycles BEFORE draws; explicit sectorE0(point,1).
First Ghidra projectile batch failed on absent0044D240 AFTER exporting two workers;
kept failure log, reused raw wrapper and completed separate exports. No absent credit.

Prior latest5738cca fixed actual Active BC dispatch0052EAF0→0052E7C0 (Remote
RET24 still genuine), and bound Players98 reaction00459500/00456860 on actual
process/roster/Sight/current-B0 order owners. Fixed100 fifth-hit logic, distinct
helmet and hand drops: [authority contract](analysis/human-h8-authority-body-source.md),
[receipt active_damage_follow_up](analysis/checkpoints/p2.2-human-h8-authority-body-20261116.json).
Existing on-foot authority through00526550 return, navigation, rifle/cautious/
launcher/vector/close target, task25/26, group3/18/21, item6/loot23/handoff24/supply33,
cover/hold completion32/40/41/42 and producers4/19/32/37 are source-bound there.
No unconditional ordinary-authority tail failure remains. Reuse those owners.
Task6 nativefalse propagates; handoff24 producer rejects Human type3; do not repair
native behavior. Positive holder reload reaches native invalid NULL3C, an honest
invalid-native failure. Actor lifetime metadata does not pin B0; reread current B0
across callbacks. CRT and engine RandFloat remain distinct owners.

## Next work — entire H8, no helper checklist expansion

Continue H8.3 **bullet0044B910/0044B400 and capsule00449290/00449030** combat,
visual/effect/damage/camera frames and original initial100 orchestration.
Existing World stubs remain explicit. Reuse all-hit query and new shell/grenade/
material effects instead of new substitutes. Selected variants remain required.
Raw visual exports now available but not implemented:
WORK/human-h8-bullet-visual.asm and human-h8-capsule-visual.asm. Capsule90 ordinary
producer is false; raw special branch is command9 after strict age>300, not the
misleading earlier inference. Both presentations orient from interpolated motion,
not reconstructed C locals. New impact projection uses explicit sector flag1;
existing one-argument human_scene_sector wrapper needs a targeted ABI check when
reached, not a broad speculative replacement.

Then remaining reached H8.2 specialized seat8..13/action16/Car22/retreat40 pending
and actual runtime callbacks (Script alarm, Human look/control motion, suppressed
Player camera/inline commands, ladder contact), positive sound36 and singleton34.
Do not assume existing World methods override Runtime's still-explicit stubs.
No successful empty actor registry, no marking H8.2 complete from first frame alone.

H8.4 now ours: [Dog contract](analysis/human-h8-dog-source.md) covers actual
23-row GameAnimations table/stride, seven-bone geometry and D4 policy kernels,
NOT live activation/frame. Reuse `h8-4-dog-live-originals` for actual
005D5290→005D4DB0→005C8370→005C7D30 and005D5390 retirement/controller4/5.
005C0F50 takes ONE pointer, pushed earlier at005C8069. Kind4+04 is005D5B00→
005CE5D0, not005CE110; do not repeat wrong-vtable/null-input inference.
[H8 plan](analysis/p22-hum-h8-plan.md) links merged occupied/Car/PhotoPlace/D8
contracts. D11 is source-resolved under FOUR explicitly user-authorized defaults;
do not reopen it. Actual occupied Human/Player/mounted, reached Aircraft/type34/
entry/selected non7 and application/FRAME/OWN/render/desktop gates remain open.

## Commands and evidence

WORK=`analysis/work/mission-world-20261103`. Research index BEFORE new exports:
`python3 -B tools/research_index.py find ADDRESS...`; use existing contracts and
raw before more exports. Original SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Read-only/noanalysis Ghidra: `../H&D2 Decomp 4/ghidra/support/analyzeHeadless`,
project `/home/control/Documents/Projects/HD2ResearchProject HD2Research`, process
hd2.unpacked.exe. Check ERROR despite exit0. No project/type mutation.

Build `make -C recovered -j1 application-launch startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ledger `python3 -B tools/function_progress.py --render`/`--check` and
`python3 -B tools/test_function_progress.py`. Native g++ ASan/UBSan/LSan uses
`.tools/hd2-sanitizer-runtime/usr/lib64` link/rpath, strict warnings; temporary
audits only. Consolidate notes/ledger/handoff at larger verified commits.
Do not repeat unchanged H1–H7/PACK audits. Priority: finish Human and get in game.
