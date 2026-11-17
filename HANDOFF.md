# P22-FRAME handoff — 2026-11-17

Active `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, branch
feature/p22-frame. Environment cwd is retired H8: use explicit workdir. Priority
is actual game entry, then FRAME/OWN. No subagents, push, original/DLL/Proton/
Windows execution or permanent tests. Heavy compilation and Ghidra sequential.
Preserve OWN dirty source and its earlier staged artifact. Continue the first
actual loading/frame failure, not unrelated admin/network research.

LATEST USER TEST:0f5fd6a8 passed SupplyBox clips and spatial sound, loaded
humanzones/gameinterface/ingame_info, then returned to Main on
`Required HUD explosive timer artwork is missing`. Actual executable and three
logs archived WORK/loading-hud-timer-return. No activation/first gameplay frame.
[Current correction](analysis/p22-frame-loading-hud-source.md),
[receipt](analysis/checkpoints/p22-frame-loading-hud-20261117.json).

Original004AF630 format0083B280 is timer-explosive%02d, not the sanitized Ghidra
symbol's underscore. Corrected exact key, retained failure/ownership/prefix
ordering. Actual installed gameinterface4DS602 frames has all10 exact direct
children. Focused75 remaining HUD startup keys corroborated:53 static original
literals plus native formats; down_panel model presence only. Original Border/
TMCamera case remains, supported by actual original bytes. No placeholders.
Full actual review i386 O2 application/probe compile/link PASS; stage-only done.
Candidate SHA256ef21bbe0316897e97702e5457262ef13f18c1ab7a06e691369b33441cbaa1dfe.

NEXT: user run SAME revised review command below, inspect first actual exception
AND activation/first-completed-stage markers. Clean result0 can still be handled
mission failure. Agent NEVER executes Windows/original/DLL/Proton. Archive actual
run before restaging. No first frame, presentation or gameplay acceptance yet.

`cd /home/control/Documents/Projects/HD2-Decomp-p22-frame-review && python3 -B tools/run_application_launch.py --game-dir '/home/control/Documents/Projects/H&D2 Decomp 4/Hidden and Dangerous 2' --work-dir analysis/work/frame-own-review-runtime`

Review tree is DETACHED edebd81 plus preserved OWN interactive overlay and later
FRAME source fixes. Mixed staged/unstaged files are expected, not an active merge.
Actual application adapter remains UNCOMMITTED pending successful user desktop
acceptance per AGENTS. Source-only fixes on FRAME are independently committable;
main application still declines activation. Full recoverable review patch now
WORK/loading-hud-review-final.patch, fingerprinted in receipt.

PRIOR SOURCE FIXES (do not repeat unchanged audits):
-0e69b80: removed never-published WorldViews.sound snapshot; scene emission,
tracking/attenuation/portals use actual constructed game scene. Environment reads
late actual camera sector through SAME-World RAII loan. Loan retained through
failed unload, withdrawn before camera/World destruction. Listener and transform
ordering preserves original00433FA0/00432710/004368D0. Native63 ASan/UBSan/leaks
PASS, actual O2 link. [Contract](analysis/p22-frame-loading-spatial-source.md).
-6f37c21: original #cratefly/#crateopen/#crateclosed/#cratedrop strings corrected
from sanitized symbol underscores; installed5DS bytes decoded. Actual user passed.
[Contract](analysis/checkpoints/p22-frame-loading-supply-20261117.json).
-8167e7c: loading sound root query uses actual constructed World; later actual
user run passed old null crash. Original0078A4F0 stops on first failed texture
status and retains acquired prefix;00785BE0 continues/shared count increments.
Actual absent d_l/d_r remains honest; no fake textures. Native4803 status/ownership
checks and earlier sound38 passed. [Footprint contract](analysis/p22-frame-loading-footprints-source.md),
[sound contract](analysis/p22-frame-loading-sound-failure.md).

Prior containing FRAME source:9289d4f delayed exit;edebd81 warmup/countdown/manual/
side;3be24f5 config/result/vote;03da7f3 Runtime snapshot/stats;4da1639 balance/spawn;
6b6d292 Human/Actor/all18 retained reset modules. Receipts revision-qualified.
Ledger34 complete/3815 partial total(client3810/engine5), frozen18783; no new
counts/promotions for this correction. FRAME remains partially_resolved for
nonoffline application/menu provider, console catalogue, general network/history/
close and specialized packets. Selected offline7 source and actual adapter are
under desktop review; HUM/OWN/application acceptance remains separate.

OWN feature/p22-own HEAD1d54083 dirty app/camera/collision/lifecycle sources and
collision_slide.h remain unchanged; no stash/reset/overwrite/merge/rebase.
Captured source hashes WORK/own-review-overlay-inputs.json and prior OWN runtime
artifact d1243e24c723ad5105f3877deb18a896d3c52b27f3baacc80d7f4dcc8c44035c
verified preserved. OWN runtime is distinct from revised isolated review runtime.
Review-only focus fixes and one-time AFTER-real-call activation/frame diagnostics
remain in patch. They do not publish readiness or prove rendering/presentation.

WORK=analysis/work/p22-frame-closure-20261117. H8=/home/control/Documents/Projects/
HD2-Decomp-p22-hum-h8. Research index BEFORE new exports; H8 snapshot misses newer
FRAME source/WORK. OriginalSHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Original HUD exports/raw already in H8 WORK/mission-world-20261103: reuse.
Compiler H8/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++;
native sanitizer lib H8/.tools/hd2-sanitizer-runtime/usr/lib64. Ghidra readOnly,
noanalysis; sequential with compilation. Windows fixtures compile/link only.
