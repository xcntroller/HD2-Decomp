# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, feature/p22-frame.
Use explicit workdir; environment cwd retired H8. Goal actual game entry, FRAME
then OWN. Preserve other work. No original/DLL/Proton/Windows execution by agent,
no push/subagents. Heavy compilation/Ghidra sequential. User revoked the earlier
AGENTS instructions; do not invent a new approval requirement from that file.

LATEST USER: actual Tutorial GAMEPLAY ENTRY confirmed and cutscene HUD now
correctly hidden on ac6c4b78. Camera view still wrong; selected squad overlaps
placeholders, helmet/backpack absent, movement crashes. Do not promote stable
play/control/camera or full FRAME/HUM/OWN acceptance. Exact exe/logs preserved in
WORK/gameplay-movement-crash-return/manifest.json. New crash is actual AV in
LS3DF10018E5B, EBX=4A4F5250 (PROJ) indexed as a frame kind, client0045F795.

CURRENT FIX BATCH:
- ShotSurface004546E0 used driver54 frame factory for PROJ; original calls50
  visual factory. Same correction for shell00454190 OBJ_ and trace0044B030 BBRD.
  No remaining literal FourCC driver_.create uses; owners/lifetimes unchanged.
- PreparedHumanLoading notified spawn context use(false), confusing native extra0
  with source authority. Native0041D960 reads process bit1. Reread SAME actual
  mode bit1 at the existing late event boundary; preserve activation/publication.
  Tutorial OnUse assigns real NPC scripts, avoiding placeholder fallback; NPCs
  grant80 helmet/101 pack. Actual assets confirm both definitions/models.
- Check-ahead: Player_2 OnUse then reaches Human-only1C on BASE SpawnPoint context.
  Native opcode28 dispatch selector118→0041861C returnsfalse without advancing.
  Preserve this known decline in Context; do not abort or invent item work.
  Human-derived1C still reaches actual inventory receiver.
- Cutscene FOV read unwritten session.words[BC]=0 while actual006CC760 root3010
  lives in scene.documents().fields().field_of_view. Script now borrows that SAME
  field through World, as normal camera/FPV already do. Tutorial FOV1.39626336.
  No second projection copy, guessed camera pose or new loadout default.

34 focused native ASan/UBSan/leak checks PASS actual notifier/Player2..4 VM/frame
commands, real NPC assignments, base1C decline, Human NPC helmet/pack requests,
mode gate/context mismatch/frame retirement and dynamic FOV borrowing. Engine/
items/binding endpoints explicit doubles. No Windows/DLL/Proton execution.
Actual REVIEW launcher/probe sequential O2/Werror rebuild and stage-only receipt:
analysis/checkpoints/p22-frame-gameplay-spawn-20261117.json. Staged launcher SHA
4efed3f51f6afe8aec859f7de34a8c4378564cc06f88fe4208b537f70bde0d26. Source complete
for these concrete errors; actual camera/movement/squad/equipment remain USER
verification, not presumed fixed. Main remains a source branch, REVIEW the actual
interactive overlay. No heavy Ghidra work or new research export was needed.

PREVIOUS abf6f6d: LOD64 is FLAGS, not stride; removed five incorrect flags==32
predicates. All four actual Tutorial flags confirm224 LOD0 vertices/32-byte base
rows; native allocation/lock/flags evidence in prior receipt. New user run now
gets through to gameplay. Existing count/capacity/lock checks remain intact.

PREVIOUS FIX e9da171: original006CD940 tag22 WXYZ→engineXYZW conversion was
missing. Fixed only serialized boundary; runtime setters stay XYZW. Actual camera
records/provenance and22 native ASan/UBSan/leak checks retained. UI HUD prepass/
draw now suppresses on SAME Script824 cutscene state while text/effects remain.
This is explicit user-requested integration policy, not native visibility body
credit; latest user now confirms HUD hidden. Camera view still needed the FOV
fix above. Diagnostic camera changes log actual name/world pose/forward. Previous
user41bb65eb cutscene/screenshot/exe/logs preserved WORK/cutscene-vertex-failure.
HUD acceptance confirmed; camera remains pending the new projection correction.

SOURCE CHECK-AHEAD263fa18: actual ResourceStore and actual MissionScriptCompiler
cover114 initial Tutorial bindings plus17 recursive literal ScriptAssign targets:
131 compile, no missing resources/includes/new predicate types, full static
command-type gaps0. Completed NPC/items/team/particles/explosion and both teleport
chains, plus actual scene-targeted Pike28 Script/packet/movement/obstruction.
713 focused native checks; no engine execution. Previous opening/camera/weather/
Human navigation/predicate checkpoints remain separately revision-qualified.
[Consolidated contract](analysis/p22-frame-startup-script-preflight.md) links the
source groups, original addresses, static coverage, receipts and limits. Do not
repeat unchanged audits or inflate plan checkboxes. Ledger remains34 complete/
3815 partial, frozen18783; no new function/desktop/matrix promotion.

REVIEW `/home/control/Documents/Projects/HD2-Decomp-p22-frame-review`, DETACHED
edebd81 + preserved OWN overlay + later FRAME source. Mixed staged/unstaged and
untracked recovered headers expected; interactive adapter remains uncommitted.
Latest full recovery patch WORK/gameplay-spawn-review-final.patch. Main still declines
activation; actual user launcher is REVIEW. HumanWorld has a preexisting OWN
focus_loading_command_human callback absent from Main: never copy that file whole.
Other updated source copied in place. Do not stash/reset/merge/rebase others' work.

USER ONLY desktop command:
`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`
Agent may --stage-only, never execute Windows. Archive fresh logs/exe before staging;
old runtime logs refer to the preceding executable until user retries.

OWN `/home/control/Documents/Projects/HD2-Decomp-p22-own`, feature/p22-own HEAD1d54083:
six dirty sources manifest WORK/own-review-overlay-inputs.json preserved. Older OWN
staged exe SHA d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c.

WORK=analysis/work/p22-frame-closure-20261117. H8 research root
`/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`; index before new exports:
`python3 -B tools/research_index.py --root H8 --db analysis/work/research-index/c34e7937c6f2f7d6-v4.sqlite3 --no-refresh find ADDRESSES`.
Also rg current source; H8 snapshot omits later FRAME work. Original static PE H8/
analysis/work/hd2.unpacked.exe SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Ghidra under H&D2 Decomp4; HD2ResearchProject/HD2Research readOnly/noanalysis.
No heavy job running. User gameplay entry/HUD confirmation only; no stable-play or full FRAME/HUM/OWN acceptance.
