# P22-HUM H8 handoff — 2026-11-17

Worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`, branch
`feature/p22-hum-h8`. Always explicit workdir. No subagents, permanent tests,
push, original EXE/DLL/Proton or Windows fixture execution. Source-only checkpoints
need no invented desktop acceptance; interactive changes need user confirmation.

## Latest source integration

Imported committed OWN `1d54083` (which already contains H8 `24c7310`) into H8.
This brings existing verified RENDER/LIFE/OWN source, actual mutable Player sender
005A3DE0/0053BF90, HUD order queries, inactive Script cutscene end and ordinary
Players selection/handover. Preserve the newer H8 World/Runtime/Car/Dog bodies.
Contracts: [Player input](analysis/human-player-live-input-source.md),
[selection](analysis/human-player-selection-source.md),
[OWN integration](analysis/p22-own-frame-import.md).
Forced strict O2 launcher/startup-probe compile/link PASS; outputs not run.
[Import receipt](analysis/checkpoints/p22-h8-own-import-20261117.json).
The OWN worktree and its uncommitted interactive candidate are untouched.
This H8 checkout retains explicit missing_frames admission after actual activation;
it does not claim installed application frames or desktop delivery.

## Next work

Finish [H8 plan](analysis/p22-hum-h8-plan.md): H8.1–.3 source checked; H8.4–.6
and parent HUM remain open. Prioritize actual Human/Player containing dependencies;
do not resume conditional Dog research without actual path evidence. The merged
Player input exposes positive drop005A2C50/0053E9D0 and nonCar Actor90 requirements.
OWN has a separate uncommitted application/Runtime/UI adapter pending desktop
review; do not import or commit that interactive candidate without confirmation.
Conditional nonHuman EC/B0/19C, type34/36, Aircraft20, Actor58 types14/5 and Dog
activation/perception/authority remain precise H8.4 requirements when selected.
No matrix/resolved_source/full-HUM promotion is justified by these imports.

Before new exports use `python3 -B tools/research_index.py find ADDRESS...`.
WORK=`analysis/work/mission-world-20261103`; original client SHA256
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Build actual launcher/probe with local `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Heavy compile/Ghidra sequential; native sanitizer audits temporary and removed.
