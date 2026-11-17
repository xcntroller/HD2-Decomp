# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, feature/p22-frame.
Use explicit workdir; environment cwd retired H8. Goal actual game entry, FRAME
then OWN. Preserve other work. No original/DLL/Proton/Windows execution by agent,
no push/subagents. Heavy compilation/Ghidra sequential. User revoked the earlier
AGENTS instructions; do not invent a new approval requirement from that file.

LATEST USER on30ede225: standalone climb now works but appears to require camera
up; Ladder traces present; original subtitles belong in black bars. Pause crash
is confirmed by user and explicitly EXCLUDED from this work. Exact executable/
logs archived WORK/gameplay-ladder-pitch-return/manifest.json. Actual terminal
error remains pause's Unrecovered application state transition; do not pursue it.

CURRENT CHECKPOINT fixes concrete returned dependencies:
- Trace Ladder headroom rejection at actor0(45.7659,.1648,14.4145),active106/
  queued206,9882,B81,posture0,9AC-10000,HD present. Source0074A690 read raw
  scalar744, which has NO producer. Actual0075B280 already computes vector740
  (standing sphere2 minus Human position) in physical.offset740_. All three
  headroom uses now borrow its Y. Old scalar0 made the probe sweep DOWN-.12
  into landing floor instead of UP by loaded head height-.12. This explains
  rejection and can explain pitch sensitivity without a deliberate pitch gate.
  No collision bypass, forced detach, new height copy or occupant/clip changes.
- Original subtitles DO use black bars. The missing dependency is native
  006DCAE0 outer full-viewport clear BEFORE rendering, omitted by our frame loop.
  Original006DCF05..006DCFA6 copies six viewport words, queries actual dimensions,
  stores full Graph7C viewport and Graph64(0,1.f,3) clears, THEN006C3190/Scene.
  Engine1005EDC0 clears only Scene viewport. Restoring text viewport after scene
  without outer clear allowed old caption pixels to persist outside world redraw.
  Added actual outer branch/Graph endpoints and restored bar subtitle geometry.
- Full-viewport consumers share process flag OR SAME live Script824 via new
  checked WorldViews RAII loan. Lifecycle binds it over actual Script link and
  retires before World destruction, including constructor unwinding. No copied
  flag/second Script owner. Missing live-game query still fails. This is explicit
  integration policy already selected for HUD/skip, NOT native RET0049B7C0 credit.
  Original subtitle bounds1600x1200 bottom1188/top1056 (800x600594/528).

78 focused ASan/UBSan/leak checks PASS using ACTUAL HumanPhysics initialization,
not a fake physical field seed: offset740.Y1.44, scalar7440, actual step worker and
real overhead rectangle sweep at three pitches with clear descriptor14 and real
ceiling rejection. Isolated OLD-header scalar744 mutation FAILS same fixture
check4, corrected source passes. Prior fixtures wrote744 directly and masked this
owner mismatch. Actual outer frame clear→scene→bar text→fixed/update/present,
depth-plane preservation, dynamic cutscene loan and retirement/missing-owner/
failed-clear behavior and software caption replacement also checked. Engine/frame/
registration/sound/other query/scene/framebuffer leaves explicit doubles.

Actual REVIEW full sequential O2/Werror launcher/probe rebuild PASS, stage-only
PASS; NEW staged SHAa9e84d72e1075de2b967840cb37a54dad463193ab74d694f07e39bc83aae6c64.
Temporary native cpp/binaries and old-header overlay fingerprinted then removed.
[Receipt](analysis/checkpoints/p22-frame-ladder-height-bars-20261117.json),
[consolidated contract](analysis/p22-frame-startup-script-preflight.md).
No Windows/original/DLL/Proton execution. No new checkbox, whole-function, ledger
or full FRAME/HUM/OWN matrix promotion. Ledger34/3815, frozen18783 unchanged.

NEXT user-only desktop retry: actual Ladder top exit, camera-independent climb
and black-bar subtitle replacement. These now have concrete source corrections
but no actual engine/user acceptance yet. Existing `Human step probe` diagnostics
include loaded head height for remaining discrepancies. Pause14 explicitly out
of scope. Distinct selector producer remains OWN/UI. Continue actual gameplay
blockers/FRAME then OWN, keeping audits focused. Prior4efed3f5 camera/HUD/movement/
spawn/equipment acceptance remains revision-qualified; soldier/prone/skip/chat
are not newly confirmed by this user message.

REVIEW `/home/control/Documents/Projects/HD2-Decomp-p22-frame-review`, DETACHED
edebd81 + preserved OWN overlay + later FRAME source. Mixed staged/unstaged and
untracked recovered headers expected; interactive adapter remains uncommitted.
Full final content/untracked recovery patch WORK/ladder-height-bars-review-final.patch;
index separately WORK/ladder-height-bars-review-index.patch. Main still declines
activation; REVIEW is actual user launcher. HumanWorld has a preexisting OWN
focus_loading_command_human callback absent Main: never copy that file whole.
All9 batch sources synced except preserved OWN focus and const World callbacks. Never reset/stash others.

USER ONLY desktop command:
`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`
Agent may --stage-only, never execute Windows. Archive fresh logs/exe before staging;
current runtime logs refer to preceding30ede225 until user retries.

OWN `/home/control/Documents/Projects/HD2-Decomp-p22-own`, feature/p22-own HEAD1d54083:
six dirty sources match WORK/own-review-overlay-inputs.json, untouched. Latest
preservation record WORK/ladder-height-bars-preservation.json. Older OWN
staged exe SHA d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c.

WORK=analysis/work/p22-frame-closure-20261117. H8 research root
`/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`; index before new exports:
`python3 -B tools/research_index.py --root H8 --db H8/analysis/work/research-index/c34e7937c6f2f7d6-v4.sqlite3 --no-refresh find ADDRESSES`.
Also rg current source; H8 snapshot omits later FRAME work. Original static PE H8/
analysis/work/hd2.unpacked.exe SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Ghidra under H&D2 Decomp4; HD2ResearchProject/HD2Research readOnly/noanalysis.
No heavy job running. No new export needed for this checkpoint.
