# P22-HUM H8 handoff — 2026-11-12

Worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`, branch
`feature/p22-hum-h8`. All H1–H7 source is present, including merged H4b9d67aa.
H8/P22-HUM remain OPEN. Read AGENTS.md, section2 of plan.md and
[H8 dependency plan](analysis/p22-hum-h8-plan.md). Preserve other worktrees.
No push/original EXE/DLL/Proton/Windows/application/desktop execution. Heavy
compilation/Ghidra sequential. No permanent tests. User priority: getting in game.

Latest [order/action/marker source checkpoint](analysis/human-h8-order-action-source.md),
[receipt](analysis/checkpoints/p2.2-human-h8-order-action-20261112.json): actual
Active0052A1E0→SAME process action7/8/**12** leases/resources/vehicle path and
SAME HUD command model748/free750 acquire/update/return are implemented.
00582920 creates action12, NOT4. Native order manager+18 remains separate.
Driver lookahead resolves NEXT target then freshly CURRENT; native texture8000
creation/update difference is preserved. Native ASan/UBSan/LSan PASS5900 actions,
PASS23500 markers, each100 lifetimes with explicit endpoints; strict actual
whole Human/Scene2/Active/HUD i386 FULL LINK and launcher/probe rebuild pass.
Eleven new partial claims, no complete promotions;34 complete/3373 partial.
Temporary audit sources/binaries fingerprinted/removed; fresh logs retained.
Actual pending-order production and full Active authority are NOT enabled.

NEXT H8.2: implement order owner005825C0/0058C5A0 and its complete marker EC/
World-root/dynamic descriptor→0058D6C0/0058C8E0/00582640 retirement; typed Active
250 sequence insert/update/remove/reset0052EE60/0052EFA0/0052EF20, hold00531F60.
Use actual Startup process order pool and existing HUD command marker methods;
no per-Human replacement pool or missing-marker successful placeholder.
Native descriptor removal BEFORE EC clear/model return; route allocation free
BEFORE process free-list return. Resolve actual Human190 borrower/ownership when
its producer is reached. HUD command close must follow retirement of live loans.
Then005282E0/00529F60 pending processing,00528950 decisions and00536120 follow
BEFORE ordinary navigation/aim/movement/trigger/command retirement/self-treatment.
Do not turn required00526550 into an idle success. Full dependency table is in
H8 plan; original helper/ABI/raw evidence is in the checkpoint contract.

Earlier source checkpoints: [frame composition](analysis/human-h8-frame-composition-source.md)
Scene2→SAME World→borrowed Runtime→B0/inline/DD0/geometry with required guards;
[presentation](analysis/human-h8-active-presentation-source.md)00525DA0 and actual
Car004D9E10. Do not re-audit unchanged H1–H7 kernels. [H6 closure](analysis/human-h6-source-closure.md).

Other required H8.2: Script00591550, remaining00751480/00755500, reached Player
camera/delayed commands, hazard00451900 and water005A97C0/00585A60. Hazard's
blast5/1000/sourceNULL/initial100 precedes ShotMgr packet00452CA0/00452D00;
not session-broadcast by assumption. Water174's exact typed producer and command
retirement need recovery; do not convert a raw scalar to an assumed actor loan
or just clear it. Non7 Remote/input/visibility is conditional, not an offline
detour. H8.3+ async combat, Car/OWN/D11/absent actors, completed FRAME/render
composition and user desktop acceptance remain open. Other branches stay owned
by their agents; completed masterfa94dee render is not imported here yet.

Accidental worktree deletion recovery: committed source survived at31c436e.
Tools now link to shared master .tools; WORK independently reflink-copied from
surviving HUM. Some ignored H6/H8 audit logs were lost; old receipts remain
historical verification, not reruns. Read-only Ghidra regenerated recent H8
exports/static bytes, recorded in local `analysis/work/human-h8-worktree-recovery-20261112.json`.
New checkpoint above has fresh verification logs.

WORK=`analysis/work/mission-world-20261103`. Index BEFORE new exports:
`python3 -B tools/research_index.py find ADDRESS...`. Existing H8 order lifecycle,
proxy/marker-return/action-boundary C and raw assembly are retained. OriginalSHA
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307. Ghidra
`../H&D2 Decomp 4/ghidra/support/analyzeHeadless`, project
`/home/control/Documents/Projects/HD2ResearchProject HD2Research`, process
hd2.unpacked.exe -readOnly -noanalysis. Check ERROR even on exit0. No types changed.
Build: `make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ledger: `python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
