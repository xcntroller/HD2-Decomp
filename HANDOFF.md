# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, feature/p22-frame.
Environment cwd retired H8: always explicit workdir. Goal actual game entry,
FRAME then OWN. No subagents/push/original/DLL/Proton/Windows execution/permanent
tests. Heavy compiler and Ghidra sequential. Preserve OWN dirty work and review.

LATEST USER: "Crashed,202 now. Is it possible to check ahead, so we don't end
up fixing50 broken script commands?" Actual staged d60751d1 passes activation,
Scene78/fixed/input/update then handles required202 (result1, no access violation).
Exact exe/logs WORK/first-frame-script202-return. No stable gameplay acceptance.
Do NOT request another one-opcode retest. User explicitly wants check-ahead batch.

CURRENT VERIFIED SOURCE: all13 initially missing command types and three automatic
predicate types now supplied. Human Stand/modes/navigation290 native checks;
actual automatic predicates/inventory/language/alarm and Car fuel27; model clip/
World tick/Script queue/snapshot8 restoration and marker lifetime77; weather9
commands plus actual positive splash/static ray/particles/Player/shelter chain2399.
Commits b17e68f/8ff34af/b188f7f. SAME actual owners, native ordering, honest required
conditional failures. Source boundaries/receipts linked in
[preflight contract](analysis/p22-frame-startup-script-preflight.md).

LATEST OPENING GROUP: actual cutscene00423C70/00423EB0 start/end camera/HUD/context/
Human activity/FPV/dubbing/animation retirement and500ms game6E input timer;
Human named special animation/working mode/nearest-player/Stop; real speech73 VM
delay/FIFO, subtitle geometry gates, camera slide, hint-ID list, secondary music,
Door6 SetActorState.86 cutscene +396 Human +38 VM/presentation native ASan/UBSan/
leak checks PASS; full actual application/probe FINAL O2/Werror build PASS
WORK/startup-opening-final-review-build.log. Latest source checkpoint receipt:
analysis/checkpoints/p22-frame-startup-opening-20261117.json. Native fixtures
fingerprinted/removed. Source copied to review IN PLACE; OWN differences preserved.
Positive command-modal GUI transitions and other95 actor families remain explicit
required boundaries; Tutorial false/hidden path calls actual current-Human refresh.
No desktop/matrix/function-count promotion. No user staging/retest yet.

CHECK-AHEAD: actual CURRENT ResourceStore selected114 binding sources; compiler
inspection uses actual MissionScriptCompiler. Recursive literal ScriptAssign
closure adds17 selected files, **131 programs all compile**, no missing resources/
includes/new predicate types. Initial (before first wait) dispatch gaps0. Full
recursive coverage still has **7 missing types**:
28 HUMAN_CreateItemInStore;100 FRM_TeleportNearCheckpoint;114 ITEM_Teleport;
127 FRM_CreateParticle;145 SetNPCTeamStatus;146 MakeExplosion;
254 HUMAN_RemoveItemsFromStore. These are decimal IDs.
NEXT: finish these concrete dependencies/positive receivers, starting reached
NPC1..4 creation/team status and simple particle packet binding. Do not stage a
known failed startup merely because first-wait coverage is0. Full event-only work
and predicate truth/frame admission remain qualified, not assumed executed.

Evidence already read for NEXT: WORK/startup-opening-command-dispatch.json maps
VM targets; original typed00416E10/00421C00 and decoder0042B650 in H8 mission-world
exports. Index WORK/startup-item-script-index.txt covers007D7810/005A7B30/005B7920.
Raw005AF760 calls SAME Items007D7810(definition,slot,Human,-1), without state5 gate;
actual create_prepared_human_item already source-bound.005A7B30 repeatedly finds
actual definition005A78F0(def,0), removes item identity10 via005A7B70(id,true), then
unconditionally Human178=true.005B7920 calls005B76A0(frame), then non7 publication
required (offline7 returns). Read-only bounded raw was viewed; save evidence before
new edits.127 writer uses8-bit definition then actual slot1 frame NAME, no null gate;
decoder reads8/stringFF, scene findFFFF, real world matrix then SAME process.emit
(def,matrix,frame,true,false). Never replace full matrix with direction-only emit.
No fresh Ghidra needed until indexed dependencies need a new export.

WORK=analysis/work/p22-frame-closure-20261117. Raw compiler inputs/programs:
startup-script-inputs.json / startup-script-programs.json (114), plus
startup-script-recursive-{inputs,programs,coverage}.json (131).
Read-only extraction/compiler inspection utilities remain for ongoing closure;
not a successful dummy gameplay runtime. Original/current reports fingerprinted
at checkpoints. Source/user staging separate. Newer staged exe/run appeared during checkpoint validation (not staged/executed
by this batch): SHA9ecf206f56e2b16b93a7fcac31ef4b19709cb284703c0b3b7c2c15512b758c8b,
mtime17:32UTC. Actual run reaches activation/Scene78/fixed/input/update, then
required28 CreateItemInStore result1. Archive WORK/first-frame-script28-return-
20261117T1732. Preserve this newer staged file; do not overwrite from old d607.

REVIEW `/home/control/Documents/Projects/HD2-Decomp-p22-frame-review`, DETACHED
edebd81 + preserved OWN interactive overlay + later FRAME source. Mixed staged/
unstaged is expected. Interactive app adapter stays UNCOMMITTED until actual user
acceptance per AGENTS. Main app still declines activation; source-only fixes are
independently committable. Latest recoverable overlay:
WORK/startup-opening-review-final.patch (includes untracked recovered headers).
No push. Do not replace whole HumanWorld services from Main onto Review.

User desktop command, USER ONLY, after batch is ready:
`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`
Archive new logs/exe BEFORE restaging. Agent may --stage-only, never run Windows.

OWN `/home/control/Documents/Projects/HD2-Decomp-p22-own`, branch feature/p22-own
HEAD1d54083, six dirty source hashes manifest WORK/own-review-overlay-inputs.json
remain unchanged. Older OWN staged exe SHA
**d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c**.
No stash/reset/overwrite/rebase/merge. OWN runtime differs from review runtime.

Earlier source contracts/receipts preserved: diary/profile7762daa, alarm/Run422d793,
fades/Door skip d8c0f1d; decoration contact8d0d1f2, water e667395, input allocation
74a6dd7, Scene2 Human activation journal e341c06; sound/HUD/crate loading contracts.
Do not re-audit unchanged paths. Positive vehicle/wind decoration, retained water
camera, game64 weather/HUD gate and other unsupported families remain honest
required paths. Ledger34 complete/3815 partial, frozen18783; no new credit/promotion.

H8 `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`; index before exports:
`python3 -B tools/research_index.py --root H8 --db analysis/work/research-index/c34e7937c6f2f7d6-v4.sqlite3 --no-refresh find ADDRESSES`.
Index H8 snapshot misses newer FRAME source: also rg local.
Original client H8/analysis/work/hd2.unpacked.exe SHA
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
ASan runtime H8/.tools/hd2-sanitizer-runtime/usr/lib64. Ghidra under H&D2 Decomp4,
Projects/HD2ResearchProject HD2Research, readOnly/noanalysis. No heavy job running.
