# Session handoff — 2026-11-02

## Latest result

User requests resource-layout cleanup and fullscreen before Phase1 closes.
The complete follow-up is implemented, built, statically/native verified and
**user desktop-accepted**: “Nevermind, I got it working. Good, that's done.”
[Contract, original addresses and review](analysis/resource-fullscreen.md),
[dual-image evidence](analysis/resource-fullscreen-map.json),
[portable instructions](docs/STANDALONE.md). Section5 follow-up is checked.
Earlier standalone packaging was accepted (“The build works fine, by the way”);
its separate artifact/acceptance is preserved in analysis/standalone-package.md.

## Current artifacts and implementation

Fresh review folder: dist/HD2-Phase1-clean; ZIP: dist/HD2-Phase1-clean.zip,
493 payload files,2,028,879,915 bytes. Entries HD2FrontEnd.exe, Launch.cmd and
LaunchFullscreen.cmd. Client SHA256 745174a2fc7f3cf6d23253d49c2ffb76faa63a9fe6f0eba57f463a375e926573.
ZIP SHA256 a95a1396230231c035a94be4dd892e74845780d28a54c65bc226f68700be383e.
Seven original engine/codec DLLs and platform dependencies remain as documented.
No Models/Sounds/MenuText/Maps extraction or renamed font atlas is packaged.
Actual model/sound caches and archive setup already existed; normal fonts now
use original Text/Fonts paths. Original Text stays loose as in this installation.
Earlier diagnostic stagers/aliases remain available; old workspaces are not pruned.

Native entry accepts --fullscreen / --windowed alongside existing flags/legacy
CLI. Default800×600 windowed. Fullscreen requests original flags2,32bpp/default
refresh, enables the existing gamma gate and avoids saving fullscreen window.cfg.
Inactive/lost frames wait; DEVICENOTRESET calls original engine Present/reset
before drawing resumes. Native frame/launch policy earns no original-body credit.
No live Alt-Enter/in-menu selector or display-mode persistence was added.

## Verification and preservation

Strict i386 application/preview/startup builds, temporary actual native
ASan/UBSan parser/settings/font/frame guard, Windows fixture compile/link,
temporary Python font/package/ZIP/stage-only audits, existing native suite,
158 Python tests,14 original instruction anchors/three calls/two path references,
dependent presentation map, ledger and whitespace checks pass. Temporary audits
removed; no permanent tests. Every real folder/ZIP payload hash/CRC and static
DLL closure verified;13 original font pairs and clean profiles validated.
All581 recorded original/accepted/writable identities are unchanged, including
the previous accepted ZIP. Logs/source/artifact hashes:
analysis/work/phase1-resource-fullscreen-20261102. No original DLL/game,
Windows fixture or Proton execution; no commit/push requested/performed.

The accepted dist/HD2-Phase1 client remains3547347c…6070b. Since that preservation
audit the user ran the Python launcher: development-staged, compiled and clean-
packaged clients now all match745174a2…26573. Its log records fullscreen2560×1440,
Czech/CE Set3 and normal state-exit/teardown result0. That is script-run evidence;
package acceptance comes from the user's statement. Initial package startup
failure was user-resolved, cause unspecified; no logging/runtime fix was made.
Do not rewrite accepted ZIPs or infer each suggested desktop case was run.

## Next work

The user's current request is a question about function matching and when
existing partial functions will be completed, before moving on. Explain the
strict full-body rules and native ABI/ownership exclusions; do not promote
functions or adopt a new matching policy solely from this question.
When instructed to continue, next checkbox is full Phase2 dependency research
and replacement of the outline with a sectioned plan. Include any subsequently
agreed function-closure/matching gates; no Phase2 implementation started here.

Keep prior deferrals: broader saves.metadata open (only stored Main caption
accepted), loadouts COMPLETELY UNTESTED until accessible, full Single Player/
game/save/results Phase2, ready-server UI/services UNTESTED until Phase3.
Original server must not run. Family catalog30 units/17 open. Frozen ledger
564 partial (559 client/five engine), zero complete /18,783 unchanged.
No subagents/permanent tests. Commit only when requested and interactive changes
user-confirmed; no push.

Build: make -C recovered application-launch engine-window startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Package: python3 -B tools/package_application.py --output dist/NEW-NAME;
existing outputs refused. Checks: make -C recovered test;
python3 -B -m unittest discover -s tools -p 'test_*.py';
python3 -B tools/function_progress.py --check.
