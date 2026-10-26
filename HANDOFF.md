# Session handoff - 2026-10-26

## Resume here

The user is stopping for the day, not requesting more implementation now.
The latest desktop `--menu-ui` run **works**: original artwork, cursor and static
English bitmap text render, and the bounded Exit adapter closes the host.
**Known issue: text runs past button borders.** The user accepts this for now;
the next session should investigate layout fidelity before expanding the UI.
Do not claim the original menu dispatcher or complete game has been recovered.

Read [README](README.md), [the text/Exit notes](RESEARCH.md#static-text-and-exit-slice)
and [the UI evidence map](analysis/menu-ui-map.json) next.

## Project scope and working agreement

- Reconstruct the base Steam Hidden & Dangerous 2 client and original LS3DF.dll.
  Sabre Squadron is excluded from current work and function counts.
- Prioritize the original menu over deeper ALPS1 mission research.
- Use our independently written i386 Windows host with the original engine and
  locally installed assets through Proton, not a lookalike replacement UI.
- Preserve original game files and the original Ghidra project. Generated assets,
  binaries, logs, toolchains and pseudocode remain ignored and local.
- Sensible local commits are authorized. Never push without explicit approval.
- Function baseline: 18,783 internal non-thunk functions; 9 partial, 0 complete.
  Integration glue and research do not receive reconstruction credit.

## Last successful desktop evidence

The user reported: "Works. The text is a bit off though - it runs off the button
border. But I'd say it's good for now".

The direct [runtime log](analysis/work/engine-window/run/host-runtime.log)
contains 37 model frames, all 13 text bindings, bitmap glyph/cursor submissions,
`Exit control activated`, 308 submitted frames and driver/graphics close.
Logs are ignored and overwritten by later launches; the bounded observation is
also preserved in the tracked UI map. Exit activation is log-confirmed; exact
hover/cancellation behavior and visual fidelity are not fully verified.

The first text attempt shut down cleanly at `Missing original menu frame:
text_screen`. Commit `54a7c67` corrected the frame-name pointer from auxiliary
`+0xF0` to name `+0xE8`. Native regression distinguishes both fields. Do not
reintroduce the wrong offset or normalize literal model-name spaces.
Earlier milestones: `6e559ce` static text/Exit; `5864005` original cursor;
`394a9e2` menu-model preview; `de944a2` camera/update diagnostics.

## Restart commands

Run from the workspace root:

```sh
make -C recovered engine-window \
  WINDOWS_CXX="$PWD/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-clang++"
python3 -B tools/run_engine_window.py --menu-ui --frames 0
```

The launch must run from the user's desktop terminal: the agent environment has
no usable Proton window driver. Do not repeatedly retry that known limitation.
`--menu-model` preserves the artwork/cursor-only mode; omit both menu flags for
the blue graphics bootstrap. `--frames 0` waits for Exit, Escape or window close.
The launcher uses an isolated ignored prefix/runtime, not the user's game prefix.
Read the direct runtime log before interpreting unrelated Wine diagnostics;
an empty log can mean the desktop process has not finished yet.

Relevant checks (choose the smallest set covering the next change):

```sh
make -C recovered test
TMPDIR="$PWD/analysis/work" python3 -B -m unittest discover -s tools -p 'test_*.py'
python3 -B tools/validate_engine_host.py --map analysis/menu-ui-map.json
python3 -B tools/validate_frontend.py
python3 -B tools/function_progress.py --check
git diff --check
```

Before the successful desktop retry: 158 Python tests passed, native tests passed,
UI evidence validated 21 instruction anchors / 3 vtable slots, strict i386 build
passed. Graphics, scene and cursor also have separate host-validator maps.

## Code and evidence for the next investigation

- [Text geometry](recovered/host/menu_text_geometry.h): literal bindings,
  styles, model-bounds projection, glyph sizing/advance/alignment and Exit state.
- [Text renderer](recovered/host/menu_text.h): fonts, 13 labels and explicit
  D3D8 state bridge.
- [Scene integration](recovered/host/menu_scene.h): exact-name/subtree lookup,
  visual world bounds and hierarchy diagnostics.
- [Name reader](recovered/host/frame_name.h): the corrected `+E8` field.
- [Geometry tests](recovered/tests/menu_text_geometry_tests.cpp): alignment,
  rounding thresholds, bounds, name regression and bounded input.
- [Font parser](recovered/host/bitmap_font.cpp) and
  [stager](tools/stage_menu_text.py): original TBLs and selected English labels.
- [Original font research](analysis/menu-text-map.json),
  [UI ABI map](analysis/menu-ui-map.json) and
  [front-end map](analysis/frontend-map.json): original addresses and boundaries.

No overflow fix has been attempted or established. Compare actual original
layout/style behavior against our geometry; do not paper over the symptom with
guessed offsets, clipping or smaller fonts. Current mode is fixed 800x600,
ANSI Set2. Button containers use their own `normal00` through `normal08` visual
children for bounds. Literal names include `bresume game`, `bchange profile`
and `text_last save00`. Hierarchy/bounds are now logged.

Important ABI/ownership pitfalls:

- Engine methods use stdcall with self on the stack, not C++ thiscall.
- Keep explicit camera/model updates: omitting them produced a green blank menu.
- Preserve graphics/driver/resource teardown order.
- The borrowed D3D8 device bridge is new integration glue, not an original font API.
  Do not Release the borrowed device or replace the explicit state snapshot with
  an all-state block: restoring FVF behind IGraph's cache desynchronizes drawing.
- Original text arithmetic includes promoted rounding-sensitive operations.
  Preserve threshold tests instead of simplifying float expressions.
- Use section-mapped PE reads for addresses; VA minus image base is not a general
  file offset. Verify Ghidra pseudocode against assembly/calling conventions.
- Research Ghidra project: `/home/control/Documents/Projects/HD2ResearchProject/HD2Research.gpr`.
  Original baseline: `/home/control/Documents/Projects/GhidraHD2Project/HD2Decomp.gpr`.
  Use read-only/no-analysis exports into new directories and verify each result;
  headless exit code zero does not prove every export succeeded.

## Deliberately deferred

Only Exit is enabled and closes our host directly. Original confirmation,
tagged-event dispatch, button animation, other screens and real dynamic
profile/date/save/version values are absent. Logos are original
`Gamemovie/Logo1.bik`, `Logo2.bik`, `Logo3.bik`; playback, audio, gameplay,
Sabre Squadron and deeper ALPS1 work remain deferred.
