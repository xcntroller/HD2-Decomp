# P22-HUM H8 handoff — 2026-11-12

Worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`, branch
`feature/p22-hum-h8`. All H1–H7 source is present, including merged H4b9d67aa.
Read AGENTS.md, section2 of plan.md and [H8 plan](analysis/p22-hum-h8-plan.md).
H8/P22-HUM/application remain OPEN. Preserve other worktrees and user changes.
No push, original EXE/DLL/Proton/Windows/application execution or invented desktop
test. Heavy compilation/Ghidra sequential. No permanent tests.

**USER PRIORITY:** finish actual Human effects from PACK's
`../HD2-Decomp-p22-pack/analysis/p22-pack-hum-h8-review.md`, baseline14d59d3,
BEFORE ordinary Active/frame work. Do not edit PACK's worktree. Full H8 closure
is not a PACK prerequisite; PACK retains its own stream/FIFO/module integration.

[Consolidated source contracts](analysis/human-h8-pack-effects-source.md) now
supply **commands2/7/8/9/11/18/20** on SAME actual owners. Latest general inventory
checkpoint binds005A2D50/005A2E10/005A2E90/005A2F50 via new
`human_inventory_packet.h`, `human_inventory_transfer.h`, `human_inventory_swap.h`.
007D8B40 source/destination storage, actual all-slot/cross-Human validation and
mutation;007DA5E0 displaced preflight;007D9DE0 retained return/fallback/drop and
paired equipment/clothing/FPV;007DA3F0 source automatic placement are supplied.
Authority slot lookup and presentation source whole-inventory lookup/ignored
result differ. Existing loading weapon request/kernel APIs retain their scope;
received packets bypass queued request producers. Typed source lookups require
native Human inventory layout; foreign/missing Scene2 borrowers fail honestly.

Raw precision evidence: normal capacity spills FLOAT; pack5 capacity/final sum
stays x87; preflight weight+FLOAT delta stays x87 without epsilon/final FLOAT
spill. Slot6 whole capacity failure precedes absent-pack rejection. Pack6 remains
borrowed, Items owns rejected displaced entries, source detach follows insertion,
retain only changes subtraction. Native B0+68 clothing policy: Player queues
FIRST80, AI/Remote directly apply existing clothing. No extra wrapper/absent
producer credit. Fresh audit PASS84100 assertions/100 lifecycle matrices on actual
ActorBase/storage/catalogue/RuntimeItem and policies with explicit engine/model,
equipment, Players/HUD/FPV/clothing endpoints. Concrete actual host i386 full link
and latest launcher/probe pass; no Windows fixture execution. Temporary audits
fingerprinted/removed. Ledger34 complete/3409 partial (3404 client+5 engine).
[Receipt](analysis/checkpoints/p2.2-human-h8-pack-inventory-20261112.json).

Earlier verified source checkpoints remain:5460739 active clothing and preceding
Active task/follow/cancel/regroup/delayed speech;447d5d8 live Remote20 creation;
cd1473e runtime9 pickup/handoff; e14d9c6 SupplyBox11 effects;f3b9295 named18
actual suffix/category3 loan/fourth-track/shared outgoing process. Their native
and concrete-host receipts remain in the consolidated contract. SupplyBox later
state1 closing/crossfade/respawn/falling frame still requires actual H8 work.

**NEXT PACK:** command12 actual Player0053D5D0/0053D870 and Remote00594110,
then remaining common3/00586200 subtypes, then combined PACK source verification.
Command12 C in WORK `human-h8-pack-effects`; actual mapped action table0083C348
and speech table0083C4A0 are still required. Action33/34 call005467C0(action,
position, direction). Offline other actions call0053D520 (Players virtual2C
setter, raw `human-h8-pack-action-control.asm`), capture Players virtual30 into
PlayerC0, reset520, dispatch actual inline action/speech, native fallback random
speech duration, then copy command24/30..50 to Player4E4/4F0..510. Non7 policy
and presentation differ. Do not replace actions with successful no-ops. Index
lookup `human-h8-pack-action-index.txt` records missing0053D520/005467C0 C; export
needed if raw insufficient. No new exports for these yet. Full checkbox boundary
must be researched before implementing it.

General inventory research: WORK `human-h8-pack-inventory-resume-index.txt`,
`human-h8-pack-inventory-detail-index.txt`, original C dirs
`human-inventory-immediate-transfer`, `human-inventory-weapon-swap`,
`human-inventory-swap-dependencies`, `human-inventory-command-receivers`, plus
`human-h8-pack-effects`. Fresh general transfer/swap/preflight/receivers assembly
records precise argument/ECX ownership; do not rely on decompiler signatures.

After PACK:005282E0/00529F60 pending,00528950 decisions,00536120 follow before
full00526550 authority. Retained first slot survives regroup reset; Human190/
route74 producers and target lifetime remain required. Other offline Script/
Player/DD0/hazard/water, SupplyBox frames, async combat, Car/OWN/D11/absent actors,
completed FRAME/render/application composition and actual user desktop acceptance
remain required. Masterfa94dee render is not imported. Prior contracts:
[orders](analysis/human-h8-order-owner-source.md),
[actions](analysis/human-h8-order-action-source.md),
[frame](analysis/human-h8-frame-composition-source.md),
[presentation](analysis/human-h8-active-presentation-source.md),
[H6](analysis/human-h6-source-closure.md). Do not re-audit unchanged H1–H7.

Accidental deletion recovery preserved committed source31c436e. Tools link to
master; WORK independently reflink-copied from surviving HUM. Lost old ignored
logs are historical receipts, not reruns; latest checkpoints have fresh logs.
Recovery `analysis/work/human-h8-worktree-recovery-20261112.json`.

WORK=`analysis/work/mission-world-20261103`. Index BEFORE exports:
`python3 -B tools/research_index.py find ADDRESS...`. Original SHA
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307. Ghidra
`../H&D2 Decomp 4/ghidra/support/analyzeHeadless`, project
`/home/control/Documents/Projects/HD2ResearchProject HD2Research`, process
hd2.unpacked.exe -readOnly -noanalysis. Check ERROR even exit0; no types changed.
Build: `make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ledger: `python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
