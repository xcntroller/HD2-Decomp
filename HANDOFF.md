# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, feature/p22-frame.
Use explicit workdir; environment cwd retired H8. Goal actual game entry, FRAME
then OWN. Preserve other work. No original/DLL/Proton/Windows execution by agent,
no push/subagents. Heavy compilation/Ghidra sequential. User revoked the earlier
AGENTS instructions; do not invent a new approval requirement from that file.

LATEST USER on c35be793 rejects subtitle overlap, upward Ladder top jitter/stuck
on release and standalone climb's unrelated animation. Actual initial run exits0,
not proof of success. Exact exe/logs/screenshot archived
WORK/gameplay-subtitles-climb-return. Fresher logs during work end with known
Unrecovered application state transition; separate archive
WORK/gameplay-subtitles-climb-return-later (same preceding c35be793).

CURRENT CORRECTION:
- Prior2429e19 climb fix was WRONG: command kind4 does not mean Look action.
  Original00586200 common1F ->005A45F0 ->005ABD90 kind4 ->005ABE30 subtype1E
  requests0077EC00 PRIMARY6A after offline/player/authority gate. Source helper
  request_human_step_motion binds that actual arm to SAME DD0/E0. Existing
  0077DE30/0074A690 then selects actual step8..14. Prior movement30 and action30
  both wrong. The previous native dirty2 test verified the wrong endpoint.
- Prior full-viewport subtitle policy also exceeded retail evidence. Restore
  0048EE50 and006C9260 consumers to process.full_viewport, retaining Script824
  only for accepted HUD suppression and independent skip input. Native ordinary
  subtitle bounds1600x1200 bottom1020/top888,800x600510/444. The prior geometry put
  text in lower black bar outside Scene redraw, consistent with accumulating old
  pixels. That pixel explanation remains inference, not agent-executed engine
  proof. Actual subtitle records correctly replace row0 and expire; no duplicate
  string cache or invented black-bar clear needed.
- LADDER TOP IS STILL UNRESOLVED. Earlier sphere-center fix matches native but
  did not fix actual user exit. Focused compare00768230/0074A690/00777AD0/
  00779E60/00767D40/0075CD50/0075BC80 has not established another concrete cause.
  Need actual engine/geometry/track observation; agent Windows/DLL/Proton
  execution prohibited. Do not force detach/collision success or mark resolved.
- Added observation-only changed-reason logs `Human step probe`, phase=clearance
  or ladder-motion. Reports no-upper-surface/low-surface/upward-obstruction/
  forward-obstruction/headroom/posture deferral/etc, or travel-obstruction/
  top-exit-rejected/accepted/stationary/advancing, plus actual active/queued IDs,
  988/B8/posture/9AC,HD marker,position,first sphere,rise. Per-Control reason dedup
  prevents unchanged per-frame spam. No owner/queue/collider/equipment lifecycle
  changes. These logs are diagnostic, not a Ladder fix or native body credit.

78 focused native ASan/UBSan/leak checks PASS: actual corrected request/Look
masks/authority gates, step14 clear/blocked diagnostic/query restoration and actual
subtitle one-record replacement/timer/native rectangle. Geometry/control endpoints
explicit doubles; no engine execution. Temporary cpp/binary fingerprinted/removed.
Actual REVIEW launcher/probe full sequential O2/Werror rebuild PASS; --stage-only
PASS. NEW staged SHA30ede22548e5e20f9c8e7b416723d2f92efb2ee2d9dde3fd0852982715920eb1.
[Receipt](analysis/checkpoints/p22-frame-subtitle-climb-return-20261117.json),
[consolidated contract](analysis/p22-frame-startup-script-preflight.md).
No new checkbox, whole-function/function-ledger or FRAME/HUM/OWN matrix promotion.
Ledger34 complete/3815 partial, frozen18783 unchanged.

NEXT: user-only retry verifies restored subtitle and corrected standalone climb,
then read fresh `Human step probe` lines to identify the actual remaining Ladder
blocker. Preserve real failure and finish its reached dependency before claiming
climb completion. This specific runtime evidence is unavailable to the agent.
Pause14/retained menu graph stays plan section5; archived separate selector
producer stays OWN/UI. No new broad research/audit cycle.4efed3f5 accepted earlier
camera/HUD,ordinary movement,selected/placeholder and equipment fixes remain
revision-qualified. c35be793 returned controls batch is not accepted; soldier,
prone,skip/chat not independently confirmed by the latest user.

REVIEW `/home/control/Documents/Projects/HD2-Decomp-p22-frame-review`, DETACHED
edebd81 + preserved OWN overlay + later FRAME source. Mixed staged/unstaged and
untracked recovered headers expected; interactive adapter remains uncommitted.
Full final content/untracked recovery patch WORK/subtitle-step-return-review-final.patch;
index separately WORK/subtitle-step-return-review-index.patch. Main still declines
activation; REVIEW is actual user launcher. HumanWorld has a preexisting OWN
focus_loading_command_human callback absent Main: never copy that file whole.
All6 correction batch sources synced except preserved callback. Never reset/stash others.

USER ONLY desktop command:
`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`
Agent may --stage-only, never execute Windows. Archive fresh logs/exe before staging;
current runtime logs refer to precedingc35be793 until user retries.

OWN `/home/control/Documents/Projects/HD2-Decomp-p22-own`, feature/p22-own HEAD1d54083:
six dirty sources match WORK/own-review-overlay-inputs.json, untouched. Latest
preservation record WORK/subtitle-step-return-preservation.json. Older OWN
staged exe SHA d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c.

WORK=analysis/work/p22-frame-closure-20261117. H8 research root
`/home/control/Documents/Projects/HD2-Decomp-p22-hum-h8`; index before new exports:
`python3 -B tools/research_index.py --root H8 --db H8/analysis/work/research-index/c34e7937c6f2f7d6-v4.sqlite3 --no-refresh find ADDRESSES`.
Also rg current source; H8 snapshot omits later FRAME work. Original static PE H8/
analysis/work/hd2.unpacked.exe SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Ghidra under H&D2 Decomp4; HD2ResearchProject/HD2Research readOnly/noanalysis.
No heavy job running. No new export needed for this checkpoint.
