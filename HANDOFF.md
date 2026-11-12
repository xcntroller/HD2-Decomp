# P22-HUM H8 handoff — 2026-11-12

Active worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`, branch
`feature/p22-hum-h8`, created at H6fc9d452. All H1–H7 source is present, including
user-merged H4b9d67aa; no reliance on deleted H4 branch/worktree. Preserve other
worktrees/user changes. No push/original EXE/DLL/Proton/Windows/application/desktop
execution. Heavy compilation/Ghidra sequential. No permanent tests.

H8/P22-HUM remain OPEN. Read AGENTS.md, section2 of plan.md,
[H8 plan](analysis/p22-hum-h8-plan.md) and
[frame composition contract](analysis/human-h8-frame-composition-source.md).
[Receipt](analysis/checkpoints/p2.2-human-h8-frame-composition-20261112.json).
H6 remains [source-complete](analysis/human-h6-source-closure.md); do not restart
unchanged H1–H7 kernels/audits. User priority is getting in game.

H8.1: Scene2 Human now uses SAME World→concrete borrowed HumanFrameRuntime→B0/
inline/DD0/Look/equipment/geometry. Recovered walk007646F0/material0075EEE0,
H3 seats, H4 Script/formation/completion, H6 firing and delayed secondary-FOV
are bound. Two Script attachment sites now use actual World. No extra Human,
physics/input/formation/clock owner; required missing receivers stay throwing.
Native scene-frame/material bridge PASS1100 with stated endpoint doubles;
strict actual whole Human World frame and Scene2 bridge FULL LINK with EMPTY
outer Runtime, plus launcher/probe rebuild PASS. Full ledger/check/README PASS,
34 complete/3360 partial; no new function claims. Temporary audits fingerprinted/
removed; original static evidence and logs retained.

NEXT H8.2: actual Active AI frame +24=00525DA0/+28=00526550 at final008109F8.
Research-index receipt and existing exact C are in WORK `human-h8-active-frame-index.txt`,
`frame-human-state-common-updates/00525da0.c`,
`frame-human-script-and-active-workers/00526550.c`. Preserve full completion
boundary/dependencies before implementation, including actual command/task owners.
Bounded Active worker export completed in `human-h8-active-frame-workers`:
00526400 self-treatment cadence,005328A0 actual attachment98,00537690/005376C0
paired mode expiry,00528570→query-update,005282E0 pending-task gate,
00528950 event/decision processing,00536120 follow/formation decision and actual
RET4 005335E0. No bodies from that export are implemented/credited yet. Index
receipt `human-h8-active-prefix-index.txt`; full saved Active raw assembly now
available.00526550 requires these decisions BEFORE ordinary navigation/aim;
do not turn the current guard into an idle/early-success frame.00526400's
positive self-treatment reaches subtype23 command/actual base medicine; its
caller ignores incidental return bits, but preserves skill/health/random/cadence.
Remote +24=00592A60/+28=00592E30 is conditional non7; not an offline detour.
Also required when selected: Script00591550, remaining00751480/00755500 actions,
Player suppressed/delayed command/input transport, contacted actor58, optional
debug visual5D94. Actual Startup0098AF98 debug flag is used, not false stub.

00451900 hazard static C/raw now exported/indexed: native blast request centre,
radius5,power1000,same-sector=false,sourceNULL, initial100 BEFORE ShotMgr packet
borrow/write/return. Packet is ShotMgr00452CA0/00452D00, not assumed to be session
broadcast.005A97C0 C available: offline+174 command5 then HUD0049EF70(2,saved174).
Its command reaches00585A60 special/unattached path; do not just clear174.
Index before export00585A60; latest lookup finds no exact C (677 frozen bytes).
These sources were investigated, not implemented; guards are honest.

H8.3+ still actual async ShotMgr0044B910/0044B400/00454360/00449290/00449030/
0044D260/0044D240/0044D570, actor/material/Dog/type15/20/34, selected Car D4/D5,
OWN D6–D9 and confirmed D11. FRAME/OWN have separate active worktrees. Completed
masterfa94dee render work is not yet imported here; compose as required later.
No full application/render/user acceptance from this source checkpoint.

WORK=`analysis/work/mission-world-20261103`, independently reflink-copied from H6.
Tools copied symlinks to shared existing compiler/Ghidra/sanitizer installation.
OriginalSHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Research index BEFORE new exports. Ghidra `../H&D2 Decomp 4/ghidra/support/analyzeHeadless`,
project `/home/control/Documents/Projects/HD2ResearchProject HD2Research`, process
hd2.unpacked.exe -readOnly -noanalysis. Check ERROR even on exit0. No types changed.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
