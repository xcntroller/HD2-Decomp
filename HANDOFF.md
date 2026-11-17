# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, feature/p22-frame.
Use explicit workdir; environment cwd retired H8. Goal actual game entry, FRAME
then OWN. Preserve other work. No original/DLL/Proton/Windows execution by agent,
no push/subagents. Heavy compilation/Ghidra sequential. User revoked the earlier
AGENTS instructions; do not invent a new approval requirement from that file.

LATEST USER accepts preceding4efed3f5 fixes: cutscene projection/HUD, movement
crash, selected/placeholder overlap and starting equipment all fixed. New reports:
stationary cutscene/barracks soldiers, skip unavailable, possibly high subtitles,
prone teleport, climb mapped to prone, Ladder upward top jitter and chat Enter
failure. Downward Ladder exit works. Exact clarified error: Console command
requires its actual Human/application/packet producer. Fresh logs confirm it:
WORK/gameplay-ladder-chat-return/manifest.json. Earlier controls archive contains
a separate missing HUD selector producer; do not conflate them.

CURRENT SOURCE CHECKPOINT fixes all those new concrete gameplay paths:
- HUMAN_Move005AF210/0058E8A0 uses actual Human state.navigation() FIRST, not
  vehicle SECOND. All46 actual Tutorial soldier route names FIRST only; existing
  navigation/AI/completion remains owner. Vehicle lookup remains SECOND.
-00586200 climb action1F sends action1E via E0 kind4/0077F050. Old movement
  kind1/30 requested prone. Correct actual Look48/dirty2 producer now used.
- Prone bone22: original007AB589 reads world coordinates then engine virtual08
  world SetPos1001C9F0, which applies inverse parent. Old localB0 setter caused
  moving-parent accumulation. Use actual world setter, leaving local writers.
- Ladder top0074A690 upward sphere originY must be baseY+radius, not baseY.
  Preserve XZ Ladder shift, masks/radii, collision rejection, descriptor14 and
  actual occupant/equipment/root-motion lifecycle. Source/geometry-double verified;
  actual user's upward exit still needs desktop retry.
- Skip0069A560, subtitle geometry and text viewport now use SAME Script824 live
  cutscene flag as accepted HUD policy, through checked existing module link.
  Process full_viewport has no source writer; retail0049B7C0 RET4 is not one.
  Explicit containing integration policy, no setter body credit.800x600 subtitle
  bounds bottom594/top528 vs prior normal-layout510/444. Escape reaches real skip.
- Chat default ConsoleQueries borrows real StartupSession producer. Actual
  request3→0072B1C0→authority00727800/3→0072A0B0/4→presentation00728450 now source-bound.
  Owning bounded text, real peerA8 clock/418 ordering,32/32/1/string grammar,
  both payload FREEs before packet finish, node unlink after finish, team/alive/
  ADMIN/byte-font policies and existing HUD output. Native unwritten108 is an
  ignored wire word, source initialized0; no fabricated routing/target.
  Offline005A9B30 genuinely RET; required nonoffline text/timer still guarded.

237 focused native ASan/UBSan/leak checks PASS (159 controls +78 Ladder/chat).
Actual Tutorial table/codecs/control/Script/step/FIFO helpers used; engine rotation,
geometry, clocks and final HUD endpoints explicit doubles. Temporary cpp/binaries
fingerprinted and removed; logs retained. Actual REVIEW launcher/probe full
sequential O2/Werror rebuild PASS and --stage-only PASS. New compiled/staged SHA
c35be79387debd4074d3e1d37e2dda12cbe859d650b32f8947bb6594af9dffb7.
[Receipt](analysis/checkpoints/p22-frame-gameplay-controls-20261117.json),
[consolidated contract](analysis/p22-frame-startup-script-preflight.md).
No Windows/original/DLL/Proton execution; new fixes await desktop acceptance.
No full FRAME/HUM/OWN, stable-play, function-ledger or matrix promotion.
Ledger34 complete/3815 partial, frozen18783 unchanged.

NEXT: user retry cutscene skip/soldier motion/subtitle baseline, prone/climb,
upward Ladder exit and chat Enter. Source/build verification cannot establish
actual rendered success. Pause still reaches unimplemented application status14:
retained menu/resume graph is plan section5; do not swallow/reset status. The
separate archived selector-producer failure remains OWN/UI work. General network,
console actions and nonoffline completion remain explicit dependencies. Keep
next work focused on actual gameplay blockers/FRAME then OWN, not new broad audits.

REVIEW `/home/control/Documents/Projects/HD2-Decomp-p22-frame-review`, DETACHED
edebd81 + preserved OWN overlay + later FRAME source. Mixed staged/unstaged and
untracked recovered headers expected; interactive adapter remains uncommitted.
Full final content/untracked recovery patch WORK/gameplay-controls-review-final.patch;
index separately WORK/gameplay-controls-review-index.patch. Main still declines
activation; REVIEW is actual user launcher. HumanWorld has a preexisting OWN
focus_loading_command_human callback absent Main: never copy that file whole.
All18 batch sources synced except preserved callback. Never reset/stash others.

USER ONLY desktop command:
`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`
Agent may --stage-only, never execute Windows. Archive fresh logs/exe before staging;
current runtime logs refer to preceding4efed3f5 until user retries.

OWN `/home/control/Documents/Projects/HD2-Decomp-p22-own`, feature/p22-own HEAD1d54083:
six dirty sources match WORK/own-review-overlay-inputs.json, untouched. Latest
preservation record WORK/gameplay-controls-final-preservation.json. Older OWN
staged exe SHA d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c.

WORK=analysis/work/p22-frame-closure-20261117. H8 research root
`/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`; index before new exports:
`python3 -B tools/research_index.py --root H8 --db H8/analysis/work/research-index/c34e7937c6f2f7d6-v4.sqlite3 --no-refresh find ADDRESSES`.
Also rg current source; H8 snapshot omits later FRAME work. Original static PE H8/
analysis/work/hd2.unpacked.exe SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Ghidra under H&D2 Decomp4; HD2ResearchProject/HD2Research readOnly/noanalysis.
No heavy job running. No new export needed for this checkpoint.
