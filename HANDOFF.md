# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, feature/p22-frame.
Environment cwd retired H8: explicit workdir. Priority actual P22 game entry,
then FRAME/OWN. No subagents/push/original/DLL/Proton/Windows execution/permanent
tests. Heavy compilation and Ghidra sequential. Preserve OWN dirty files/artifact.
Continue the first concrete loading/frame failure, not broad admin/network work.

LATEST USER: "Crashes after pressing space to continue". Exact cb8b636c exe/logs
archived WORK/loading-first-frame-input-return. Actual log: loaded1/status5
activation succeeds, SAME-World FRAME binds, Scene78 render AND fixed-update
complete. Then handled `Game input requires the actual two mutable64-word
controller views`; clean teardown result1. No access violation/old decoration/
collision-retirement error this run. Startup63 bindings/63/63 output words.
No user-accepted stable world presentation/input/update/gameplay yet.
[Current source/result](analysis/p22-frame-loading-decoration-source.md),
[receipt](analysis/checkpoints/p22-frame-loading-decoration-20261117.json).

NEXT: resolve original input manager allocated storage versus64-word consumers.
Do NOT just weaken guard or append fake binding63: original HUD0049C1D0 and
Player0053BF90 actually write64 words, while primary mode2 settings install0..62.
Native006C7480 appends output words one-by-one with0045CCC0 growth; existing
raw input::client::Vector models size+max(size,1), leaving spare allocated bytes
unwritten. Portable adapter bulk std::vector::resize differs. Need verify helper
raw/export and preserve honest unwritten allocation semantics and SAME buffers.
Index batches WORK/first-frame-input*-index.txt; new growth export is in progress
or completed WORK/first-frame-input-growth-original, log same prefix. Current
main source has NO input edits yet. Check current files/status before acting.

Most recent source: prepared Human direct00611C90->Human.activate bypassed
GameWorldActors' activation journal. Scene2 skipped derived retirement and deleted
reserved Human first; later external composition faulted in command.clear_empty
at rebuilt0051F722. Human dispatcher now journals before required receiver (also
throw), so original mode retirement removes B0/DD0/collision BEFORE actor deletion.
Native61 ASan/UBSan/leaks passes real dispatcher/registry/helper with explicit
lifetime/frame endpoints. Revised user run passed prior fault/retirement errors.

Actual decoration0047BCC0 groups98/9C gate, SAME listener/conditional Player
anchor,0047EB80 rolling grid and0047E2D0 static ray/material selection now source-
bound.0047B500 LIFO model/template pool, genuine failed-load latch, real engine
material/root/transform/opacity and0047D110 mask sampling use actual World.
Tutorial87x87 grid; all10 named4DS geometry/LOD/group/material data decoded.
Native22868 ASan/UBSan/leaks passed actual parsers/grid/pools/mask/failure prefixes
with explicit engine/ground endpoints. Full actual i386 O2/probe PASS; cb8b636c
was stage-only then user-executed. Temporary native fixtures removed/fingerprinted.
Positive Player/vehicle bends and wind mesh capture still fail explicitly; no
fabricated request vector/readiness/success. Do not mark whole decoration complete.

REVIEW: `/home/control/Documents/Projects/HD2-Decomp-p22-frame-review`, DETACHED
edebd81 + preserved OWN interactive overlay/later FRAME source. Mixed staged and
unstaged is expected, not a merge conflict. Interactive app adapter remains
UNCOMMITTED pending actual user desktop acceptance per AGENTS. Source-only main
fixes independently committable; main app still declines activation. Latest full
recoverable overlay WORK/loading-decoration-review-final.patch includes both
untracked collision_slide.h and game_decorations_runtime.h. No accepted game yet.

Desktop command, USER ONLY:
`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`
Inspect NEW host-runtime/steam logs AND actual completion markers. Archive each
run before restaging. Do not ask another retest until next correction is concrete.

Prior source fixes:e3f3a84 exact HUD timer-explosive%02d (0083B280),75 startup keys;
0e69b80 actual sound scene + late SAME-camera RAII loan;6f37c21 #crate clips;
8167e7c sound root/footprint failure status RET. Contracts/receipts remain linked
in analysis/p22-frame-loading-*.md. No need to repeat unchanged audits.
Earlier FRAME9289d4f/edebd81/3be24f5/03da7f3/4da1639/6b6d292 retained source.
Ledger34 complete/3815 partial total(client3810/engine5), frozen18783. No new
counts/promotions. FRAME/HUM/OWN partial; nonoffline app/menu, console catalogue,
general network/history/close/special packets remain external to current path.

OWN branch feature/p22-own HEAD1d54083 dirty sources and older staged exe
SHA d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c
remain unchanged; source manifest WORK/own-review-overlay-inputs.json rechecked.
No stash/reset/overwrite/merge/rebase. OWN runtime differs from review runtime.

WORK=analysis/work/p22-frame-closure-20261117. H8=/home/control/Documents/Projects/
HD2-Decomp-p22-hum-h8; research index before new exports (H8 snapshot misses newer
FRAME). OriginalSHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++;
ASan runtime H8/.tools/hd2-sanitizer-runtime/usr/lib64. Ghidra under H&D2 Decomp4,
Projects/HD2ResearchProject HD2Research, readOnly/noanalysis. Sequential heavy jobs.
