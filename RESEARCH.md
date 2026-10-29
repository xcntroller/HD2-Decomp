# Technical research guide

For the plain-English overview and milestone tracker, see [the README](README.md).
This guide preserves the detailed evidence, limitations, and tool instructions.

An early-stage effort to understand Hidden & Dangerous 2 and its LS3D engine.
The reference installation is the Steam release with Sabre Squadron, running
on Linux through Proton.

**Current result:** the outer UPX packing layer has been removed from copies of
both client executables. Static analysis of the base-game client has identified
startup, the main loop, and a mission-loading pipeline.
Selected ALPS1 mission resources have also been extracted locally, and the
`scene2.bin` container structure and selected object fields have been matched
to their loader and engine consumers.
The normal intro-to-front-end transition, menu-state dispatch, and original
main-menu/control definitions have now also been located.

**This is not a playable source reconstruction.** We have not run the unpacked
copies, verified these findings under a debugger, or established that every
remaining startup/protection mechanism is understood.

## What is actually happening?

The game executable contains machine instructions, not the original source.
Ghidra translates those instructions into an assembly listing and approximate
C-like code. We follow calls, filenames, and engine interfaces to work out what
the code does, then test those interpretations.

- **Unpacking:** recovering code compressed inside an executable.
- **Disassembly:** displaying machine instructions as readable assembly.
- **Decompilation:** estimating higher-level code from those instructions.
  This does not recover the original names, types, or comments.
- **Static evidence:** what we can establish from files without running the game.
- **Runtime verification:** checking that an interpretation matches the running
  game, for example by stopping at a function while a mission loads.
- **Address:** a location in the loaded program. Addresses below are preferred-base
  virtual addresses for one exact binary, not file offsets.

Getting readable pseudocode is a foothold, not proof that the interpretation is
correct. There is no meaningful overall completion percentage yet.

## Progress tracker

Checked items are bounded milestones, not claims that an entire subsystem has
been reconstructed.

### Completed

- [x] Fingerprint the original client binaries and inspect their PE structure.
- [x] Reproduce stock UPX's rejection of both clients.
- [x] Restore erased UPX identifiers on separate copies using a reproducible tool.
- [x] Pass UPX integrity tests and unpack both clients.
- [x] Establish headless Ghidra access without running game code.
- [x] Explain the initial entry-point decompiler failure: inline strings were
  mistaken for instructions.
- [x] Find the startup wrapper's on-disk jump target and decompile runtime startup.
- [x] Create a separate research project so the GUI project can remain open.
- [x] Identify base-game application startup, graphics initialization, and main loop.
- [x] Identify a mission-load orchestrator and concrete scene-resource references.
- [x] Record a map with 18 functions, 17 direct-call edges, and 8 resource anchors;
  validate those references against local exports and the binary fingerprint.
- [x] Extract ALPS1 `scene.4ds`, `loader.4ds`, and `scene2.bin` from the base archive.
- [x] Validate the scene root, object records, and immediate field boundaries.
- [x] Fix the recognized stack-probe model in the separate research project.
- [x] Add tested native archive extraction and scene-structure inspection tools.
- [x] Repair three archive import signatures and three chunk helpers using
  assembly-supported argument counts and calling conventions.
- [x] Decode object names, creation-kind requests, position/rotation/scale,
  fallback position, and parent-name containers.
- [x] Verify exact sample counts and selected field-consumer call sites with
  a repeatable evidence validator.
- [x] Start readable C++ source with a read-only chunk-reader reconstruction,
  tested independently against synthetic scenes and the real ALPS1 sample.
- [x] Trace model-resource names and reconstruct the backward last-dot name rule.

### Next: a rebuilt executable reaching the original menu

- [x] Trace the normal startup/video transition to the front-end state.
- [x] Locate menu-specific setup, screen definitions, model binding and shared
  update/render paths.
- [ ] Recover the minimal window/graphics/engine initialization contract.
- [x] Recover a bounded graphics ABI and compile an independently written window host.
- [x] Build a 32-bit Windows host and demonstrate an engine window through Proton.
- [ ] Integrate archive/resource registration, menu models, text/fonts and cursor.
- [ ] Wire mouse/keyboard input and clean Exit/teardown.
- [ ] Compare the resulting interactive menu with the original game.

The working assumption is **Proton first**, retaining the original LS3DF.dll and
locally installed assets while reconstructing the necessary client code. A
native Linux engine rewrite is not the immediate target. A graphics-only host
now compiles and the user has verified a blue engine window on the desktop. Initial video
playback is not required for this
milestone; the rebuilt host can eventually enter the recovered menu path
directly once its prerequisites are established, without patching the original
game.

Deeper ALPS1 format work is paused. Existing mission/model results remain useful
supporting evidence, not a prerequisite to understanding every mission before
attempting the menu.

### Later

- [ ] Recover relevant engine interface/vtable layouts.
- [ ] Investigate unresolved opaque calls through `00A59000`.
- [ ] Finish the menu's other screens, then connect mission selection and loading
  to gameplay.
- [ ] Resume object-field decoding where the next runnable milestone needs it.
- [ ] Compare base-game findings with Sabre Squadron; do not reuse addresses blindly.
- [ ] Choose and investigate a bounded gameplay system, such as squad orders.
- [ ] Build tested parsers or behavioral reconstructions where evidence supports them.

Full playable reconstruction remains a long-term possibility, not a promised
deliverable.

## The map so far

Names here describe inferred roles; they are not original developer symbols.

| Address | Interpretation | Confidence |
| --- | --- | --- |
| `007E8EFD` | C/C++ runtime startup | High |
| `006DD550` | WinMain-like application entry | High |
| `006DCAE0` | Application lifecycle and main loop | High |
| `006DBFC0` | Platform and engine setup orchestration | High |
| `006DB0F0` | Graphics interface initialization | High |
| `006DC950` | Frame interval calculation and limiting | High |
| `006C20F0` | Controller update and state transitions | High |
| `006989B0` | Gameplay activation / mission-load caller | Medium |
| `006CA780` | Mission-loading orchestration | High |

The mission-load orchestrator calls preparation code, a stage that loads
`missions\%s\Scene` through an engine interface, and another stage referencing
`missions\%s\scene2.bin`. It also references `tree.klz` and invokes effects loading.
The exact virtual-dispatch connection from the frontend to this path is still
unresolved.

See [the machine-readable map](analysis/initialization-map.json) for call sites,
evidence, confidence, and limitations. Its addresses apply to the unpacked
base-game client with SHA-256:

```text
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307
```

High confidence means strong static support for a role, not runtime confirmation.
Some loader pseudocode still has poor stack recovery. Ghidra's recognized function
count is not a count of understood functions.

## Menu-first integration boundary

See [the front-end evidence map](analysis/frontend-map.json). Everything in this
section is **static evidence**, not a runtime demonstration or additional
reconstructed-function credit.

The normal path starts `Logo1.bik`, progresses through the other logos and
`Intro.bik`, and reaches `006C0EF0(0, 0)` through `006C20F0`'s video-completion
branch. The null-descriptor path allocates `0x240` bytes, installs vtable
`00815E30`, stores the active state at controller `+0x14`, and invokes initializer
slot `+0x04`, resolving to `0066AD10`.

The front-end's type getter `006C1650` returns **2**; the video's `006C0DB0`
returns **10**. These type IDs, transition status **13**, and the video sequence
index **4** are different fields/contracts, not a single enum.

| Boundary | Static interpretation |
| --- | --- |
| `0066AD10` | Shared front-end setup: scene, camera, light, sound, derived initialization, content and cursor |
| Menu vtable `+0x40` → `00626340` | Menu-specific setup, including an unresolved `00A59000` call |
| `0066C5B0` → menu vtable `+0x38` → `00627060` | Cursor/confirmation resources, then menu model/control binding |
| `00623DD0` | Builds screen/control definitions, including `MAIN MENU` / `main_menu` |
| `0066BFA0` → `004290A0` | Creates kind-9 model and enters the existing client resource-loading boundary |
| Menu vtable `+0x34` → `00626B20` | Menu event dispatch |
| `0066BEA0`, `0066DE00` | Shared update and scene-render paths; the latter calls cursor drawing |
| `0066A4E0` | Cursor setup referencing `CURZOR` and `x_kursor.tga` |

Main-menu control bindings include `bresume game`, `bchange profile`,
`bsingleplayer`, `boptions` and `bexit`. These are internal object/control names,
not necessarily the localized captions shown to the player. Other anchors
include cursor model key `kurzor_menu`, `button.wav`, and sound-frame name
`sound_click`. These references do not establish exact archive members or
resource-overlay precedence.

The menu is not simply an engine feature that can be switched on: client-side
definitions, control classes, text/localization, input dispatch and resource
services must also be reconstructed or integrated. Reusing the original engine
initially avoids rebuilding its renderer first; it does not remove those client
dependencies. Calling the existing game's entry point would not demonstrate our
own reconstructed menu.

Next implementation gate: establish window/graphics/engine ABI and lifecycle,
then a minimal independently built host. The graphics-only host below is now
built and desktop-tested; only launch within the agent environment is blocked.
Several
indirect-call signatures still corrupt Ghidra's stack recovery; generated C is
not safe to compile as recovered source. Opaque startup/menu calls must not be
silently omitted. There is no claim of runtime equivalence or a running menu.

### Repeatable front-end checks and exports

From the workspace root:

```sh
python3 -B tools/validate_frontend.py
python3 -B -m unittest discover -s tools -p 'test_frontend.py'
```

The validator fingerprints the unpacked base client and checks 12 instruction
anchors, 25 vtable slots, 10 direct calls and 14 resource/control-name references
through section-mapped PE reads. Its synthetic tests also reject corrupted
instructions, pointers, calls, strings and terminators without commercial files.

Some indirectly referenced menu methods were absent from Ghidra's function
list. [The guarded seed script](tools/DefineFrontendFunctions.java) checks the
client fingerprint, exact vtable prefixes and callback reference, then defines
missing targets without clearing overlapping functions. Use it transiently:

```sh
ghidra/support/analyzeHeadless \
  /home/control/Documents/Projects/HD2ResearchProject HD2Research \
  -process hd2.unpacked.exe -readOnly -noanalysis -max-cpu 2 \
  -scriptPath "$PWD/tools" -preScript DefineFrontendFunctions.java \
  -postScript ExportFunctions.java "$PWD/analysis/work/frontend-inspection" \
  0066ad10 00626340 00627060 00623dd0 00626b20 0066b140 \
  0066bea0 0066de00 0066bfa0 0066c5b0 0066a4e0 0066e0f0
```

Choose a **new output directory** each time. Check the log and every requested
JSON/C pair, including `decompiled: true`: headless Ghidra can return zero after
a script failure. `-readOnly` discards the temporary definitions, preserving the
research database and frozen function census. Generated exports stay under the
already ignored `analysis/work/`; neither the user baseline project nor game
files are edited.

## Graphics-only host

[The host](recovered/host/engine_window.cpp) is **new integration code**, not a
reconstruction of the whole client initializer. Its bounded
[settings layout](recovered/host/graphics_settings.h) and
[ABI evidence](analysis/engine-host-map.json) are based on both original
binaries. It receives no original-function reconstruction credit.

The graphics singleton comes from exported `GetIGraph` (`10068F90`), constructed
with vtable `1009D840`. Unlike ordinary client C++ methods, the used graphics
methods pass **self on the stack with stdcall cleanup**:

| Slot | Target | Host use |
| --- | --- | --- |
| `+00` | `1006C2D0` | Initialize from a 56-byte record; `RET 8` |
| `+0C` | `1006DA20` | Initialized flag returned in **AL**, not a 32-bit BOOL |
| `+10` | `1006DA40` | Close resources and clear initialized flag; `RET 4` |
| `+48` | `1006A240` | Main window handle; `RET 4` |
| `+64` | `1006A6C0` | Clear color/depth, four stack slots; `RET 16` |
| `+68`, `+6C` | `1006B380`, `1006B3B0` | Begin/end scene via device tail dispatch |
| `+70` | `1006B3E0` | Present, including original device-loss handling |

The initialization record contains instance/adapter, main and child window
coordinates, packed color-depth/refresh bytes, flags, menu handle and optional
notification callback. The host selects an 800x600 window with child-window flag
`0x80`, no fullscreen flag, no menu handle and a null notification callback.
The original notification dispatcher explicitly permits null. The full display
configuration policy and callback contract are not reconstructed.

By default this program requests an original-engine window and plain clear-color frames.
It loads **no menu, mission, archive, font or movie**. Its title says it is an
engine bootstrap, not the menu. Its own Win32 message loop handles Escape/Close;
bounded runs post a real `WM_CLOSE` after the requested frames, restore the
window procedure and call the original graphics cleanup before unloading.
The user has observed the engine windows, blue clear-color surface and subsequent
shutdown in a desktop run. Exact completed frame count, process exit status and
full resource cleanup have not been independently confirmed.

### Build and desktop launch

An i386 MinGW-compatible C++17 toolchain is required only for the optional host:

```sh
make -C recovered engine-window
```

The initial build failed because `i686-w64-mingw32-g++` was missing. A portable
official LLVM-MinGW toolchain was then installed under the already ignored
`.tools/`, without changing system packages:

- Release: `mstorsjo/llvm-mingw`, `20261020`, UCRT Ubuntu 22.04 x86-64 archive.
- Download SHA-256, checked against the release's published digest:
  `55064b60b991a1e1e8090b8c2f84e82b22cfe5903d12c159d65064cf5e8ae335`.
- Compiler target: `i686-w64-windows-gnu`; the resulting host is PE32/i386.

With that local compiler, from the workspace root:

```sh
make -C recovered engine-window \
  WINDOWS_CXX="$PWD/.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-clang++"
python3 -B tools/validate_engine_host.py
python3 -B tools/run_engine_window.py --frames 30
```

The [launcher](tools/run_engine_window.py) fingerprints the original engine
**before execution**, rejects non-i386 host files, and creates a separate prefix,
working directory, shader path and logs under `analysis/work/engine-window/`.
It does not use the original game's Steam prefix or run from the game directory.
Runtime path symlinks escaping that workspace are rejected. The host loads the
DLL by absolute path with its sibling dependency search and verifies used
method RVAs relative to the **actual loaded module**, rather than assuming its
preferred load address. Original game files remain untouched.

Defaults use the Steam installation at `~/.local/share/Steam` and its Proton
Experimental launcher. Override `--steam-root`, `--proton`, `--game-dir` or
`--executable` if needed. The default run requests Close after 180 frames;
`--frames 0` keeps it interactive until Escape/Close. A successful bounded run
should print host entry, graphics initialization, `WM_CLOSE received`,
`Submitted 30 frames` and `Graphics closed`, then exit zero.

**Initial agent-environment launch result:** Proton Experimental 11.0 returned 1
before entering the host, reporting process-creation error **731** and
`nodrv_CreateWindow` / no window driver. This session has no `DISPLAY`; neither
an engine ABI failure nor a successful window can be inferred from that launch.
No environment restrictions were bypassed. Retry the same command from a normal
desktop terminal and inspect `analysis/work/engine-window/run/steam-0.log` if it
fails. Compilation and static/native tests are not substitutes for this check.

**Desktop result:** the user subsequently ran the host and reported two windows,
one showing the blue test surface, followed by shutdown. The updated Proton log
corroborates loading our i386 host and original LS3DF.dll; its recorded frame
argument is `3000`. No host process remained at inspection. This verifies the
visible graphics-bootstrap milestone, not the menu, exact frame completion,
exit status or leak-free cleanup. The main/child window arrangement is expected
from flag `0x80`; it is not two reconstructed menu screens.

The user confirmed, and the local installation contains,
`Gamemovie/Logo1.bik`, `Logo2.bik` and `Logo3.bik`. We keep those files unchanged
and defer playback. Their directory location is installation/user evidence, not
a newly recovered movie-search algorithm.

## Original menu-model preview

The optional `--menu-model` mode now builds a scene around the actual
`MODELS\main_menu.4ds` asset. It is a **static artwork experiment**, not yet the
original interactive front end. The first desktop test showed a plain green
background, **not the menu artwork**. That Proton log confirmed that this
run included `--menu-model --frames 300`; this time it was not the old blue-only
mode. After adding the explicit camera/model updates described below, the user
confirmed the **original menu artwork appeared**. Missing text was expected.
The [scene host](recovered/host/menu_scene.h) is independently written glue and
receives no original-function reconstruction credit.

After the build above, run from the desktop terminal:

```sh
python3 -B tools/run_engine_window.py --menu-model --frames 0
```

Escape or closing the window ends the test. The main-menu Exit artwork is not
wired up. For a bounded run, use `--frames 300` instead. Omit `--menu-model` to
keep using the previous blue-surface bootstrap.

The [staging tool](tools/stage_menu_resources.py) reads the original Models and
Maps archives without changing them. It parses the menu model's first table,
extracts its 13 unique textures, the scene initializer's implicit snow
texture and the original cursor texture, and stages **16 assets** under the
ignored runtime working directory.
It rejects missing dependencies, ambiguous names and unsafe output paths before
writing. The local `menu-resources.json` manifest records source-archive and
member hashes. This is a deliberately bounded **base-archive-only** slice:
Patch/Maps_U precedence, full archive registration and client resource caching
are not reproduced. No commercial files are added to Git.

The [ABI map](analysis/menu-scene-map.json) captures the checked engine/client
instructions, method slots and limitations. Validate it separately:

```sh
python3 -B tools/validate_engine_host.py --map analysis/menu-scene-map.json
```

The host initializes graphics, then the driver, and creates scene (13), camera
(3), ambient light (2) and model (9). It uses stack-self/stdcall public methods
for the original camera position `(0.5, -0.375, -15.5)`, forward direction,
near/far `0.2/20`, field of view and orthographic projection with extent `2`.
The primary sector is the one remaining direct layout dependency: scene+`1F8`,
also read by the original client. Model LinkTo takes **three** stack arguments,
including self; a nearby saved register in the caller is not a fourth argument.

The engine's scene render method already brackets BeginScene/EndScene; the
host must not wrap it again. Negative load/render statuses are reported instead
of falling back to a success-looking blue window. Local frame references and
scene are released before driver close and graphics close, including on reported
initialization errors.

Successful logs should include `3D driver initialized`, the exact model path,
`Original menu model opened`, a nonzero submitted-frame count, `3D driver closed`
and `Graphics closed`. These logs alone do not prove the artwork is correct.
The existing desktop/window-driver restriction prevents a visual check in the
agent environment; a user desktop run is still needed. Fonts, localized text,
button binding, audio, logos and the original scene callback remain outside
this preview. Cursor rendering is now implemented separately as described below,
but has not yet been visually checked.

### Plain-green preview investigation

The green matches the scene initializer's default RGB `(0, 0.5, 0.4)` rather
than the bootstrap clear color. This supports reaching scene rendering, but
does not establish that any menu geometry was drawn or textures loaded.
The host now explicitly performs the original camera's post-configuration
update and refreshes the model after reparenting. These changes are statically
supported. The subsequent user desktop test confirmed visible artwork, so this
is now the working integration baseline. That report does not establish which
individual update was decisive or prove full lifecycle correctness.

To avoid relying on screenshots or manually copied output, the launcher now
relays combined stdout/stderr while saving it to the ignored runtime
`host-output.log`. It includes model-open failures, a bounded tree of loaded
child frames/vtable RVAs, frame counts and cleanup messages. A model with no
attached children is rejected rather than reporting it ready.
The successful artwork run left that combined output file empty. Standard
handle forwarding under this Proton configuration therefore cannot be relied
on. The host now opens `host-runtime.log` itself, reopens both CRT streams and
shares the output descriptor, before writing unbuffered diagnostics. The
launcher checks both log paths for unsafe links, clears stale direct output,
and appends direct host diagnostics to the relayed log after process exit.
Direct logging is built and synthetic relay tests pass; its Windows runtime
behavior still needs to be observed.

### Original cursor integration

The [cursor host](recovered/host/menu_cursor.h) loads the real
`MAPS\x_kursor.tga` (32x32 RGBA TGA, 4,140 bytes), with the original load flags
`0x402`. It does not substitute a system arrow or draw a lookalike. The
[cursor ABI map](analysis/menu-cursor-map.json) checks both binary fingerprints
and the exact load, bind, viewport, vertex-format and draw contracts.

The original caller `0066A870` draws a four-vertex screen-space triangle fan
after scene rendering, using an independent BeginScene/EndScene pair. Its
32-byte vertices contain XYZRHW, diffuse/specular colors and one UV pair.
The unusual UV order `(0,0),(0,1),(1,1),(1,0)` is preserved, as are the exact
depth/reciprocal-W bits. At 1024x768 the quad is 30x30 pixels; each axis scales
independently with resolution. The [geometry helper](recovered/host/cursor_geometry.h)
is native-testable and checks layout, original bit patterns, scaling and bounds.

The loaded texture is the **graphics wrapper** with vtable `1009DC34`, whose
slot zero is Release (`10073760`); it is not the raw texture interface with
vtable `1009D9C0`. The cursor releases that wrapper before driver/graphics
cleanup and reports negative viewport/draw results or a false bind result.

Pointer tracking uses Win32 rather than the original client input subsystem.
It converts desktop coordinates into the verified engine child HWND, then
scales to render coordinates. The cursor is drawn only inside the foreground
host's client area. No pointer capture, confinement, menu click behavior or
button highlighting is claimed. The OS cursor is hidden over the client only
while our cursor is active.

Build and run the same `--menu-model --frames 300` command for a bounded test,
or `--frames 0` to move the cursor until Escape/Close. The expected addition is
the game's own cursor following the mouse over the existing artwork. Text is
still absent in that mode. **The user has now confirmed cursor visibility and
mouse tracking on the desktop.** The direct Windows log also contains the cursor
submission and normal driver/graphics close messages for that run; this is not
a debugger or leak check.

```sh
python3 -B tools/validate_engine_host.py --map analysis/menu-cursor-map.json
```

### Original text/font contract

The [text research map](analysis/menu-text-map.json) records the original contract
behind the new bounded implementation. Text is **client-owned**, not another engine singleton:
the client resolves localization IDs, allocates text objects, reads glyph
metrics, lays out glyphs and builds triangles. LS3DF only supplies the texture
and primitive interfaces already used for the cursor.

The main-menu definitions bind the title to localization ID `2101`, captions
to `2102`-`2104`, and static button labels to `2105`-`2112` and `2115`/`2116`
(not every intervening ID is a main-menu button). Initially empty dynamic fields
use `2002`. These IDs come from original definitions, not newly hardcoded
English labels. Ghidra's symbol spellings misleadingly use underscores for
embedded spaces: the literal names are `bresume game`, `bchange profile` and
`text_last save00`. No underscore-to-space normalization is applied.

The local resources are already loose: `Text/english/TEXTY.txt` and
`Text/Fonts/ANSI/SetN/{BIG,SMALL}.{tbl,tga}`. Font set 1 is selected below
800 pixels wide, set 2 at 800-1023, and set 3 at 1024+. The original initializer
also loads CE and SYMBOLS resources. Each TBL contains 256 records of five
little-endian floats (5,120 bytes total): UV rectangle and width/height ratio.
The original atlases use the same `0x402` texture-load flags as the cursor.

### Static text and Exit slice

The new `--menu-ui` mode preserves the earlier artwork/cursor test as a separate
mode. Build the Windows host as above, then run from a desktop terminal:

```sh
python3 -B "/home/control/Documents/Projects/H&D2 Decomp 4/tools/run_engine_window.py" --menu-ui --frames 0
```

Expected addition: 13 original static labels in the original bitmap fonts.
Only Exit is enabled. Hover selects its original active artwork and makes its
caption white; press and release inside
its visual rectangle closes the reconstruction host. Dragging out or losing
focus cancels the press. Escape and the window close button still quit.
This is a **host-owned interaction adapter**, not the original control dispatcher
or quit-confirmation flow; other screens remain disabled. **The 2026-10-26 desktop
test confirms visible text and working Exit.** On 2026-10-27 the user also
confirmed the original horizontal-inset correction works as expected.
The user also confirmed the original button-visual switching works as remembered;
pixel-perfect visual fidelity is not claimed.

[Text staging](tools/stage_menu_text.py) reads the loose English localization and
validates the selected printable-ASCII records, both original TBLs and the
256x256 32-bit TGA atlases. Five additional local assets are staged beside the
16 artwork/cursor assets, with a separate source/output SHA256 manifest.
The `H2MT` label bundle is our own bounded runtime format, not an original game
file format. Assets and localized content remain ignored and local.
This slice explicitly requires 800x600/ANSI Set2; other language/font sets,
wrapping, dynamic profile/date/save/version fields and patch overlays are not
implemented.

The [UI ABI map](analysis/menu-ui-map.json) validates the world-bound accessor,
the eight-DWORD bounds record and its XY components, projection constants, character rounding and
triangle submission. Button containers are not visuals: each binding finds
its numbered `normal00`-`normal08` child under the actual named button, and
rejects missing/ambiguous nodes. No guessed screen-coordinate hit areas are used.
Styles 11/13 are left/vertically centered; style 10 is right/bottom aligned.
The original raw visual projection is `x = width * world_x`,
`top = 1 - height * max_y * 4/3`,
`bottom = -height * min_y * 4/3 - 1`.

#### Horizontal text inset correction (2026-10-27)

Cached exports revealed that the initial adapter omitted an original layout
step; section-mapped disassembly confirms it. The text-object constructor
`00661B30` sets byte `+90` to 10 (`00661C52`). Type-14 button initialization
(`006550E0`, vtable `00814CB0` slot `+2C`) calls that constructor directly and
uses the normal visual at control `+70` for layout. Plain type-13 static text
instead uses `0065C680` (vtable `00813CC0` slot `+2C`), which overrides the byte
to 1 at `0065C6E7`. The main-menu definitions contain no inset override event.

At `00644AE1`, layout computes `inset_px = trunc(width * (1/1024) * byte_90)`.
The exact float32 scale at `008109C8` is `0x3A800000`. It adds this integer to
projected minimum X and subtracts it from maximum X, retaining promoted
arithmetic until each resulting edge is stored as float32. At width 800 this
is **7 pixels per button edge**, but **0 pixels for plain static labels**.
The host now reproduces this step without changing fonts, glyph height,
vertical placement or artwork. Raw visual rectangles remain the host-owned
Exit hover/click areas; only text layout uses inset rectangles. Runtime binding
diagnostics now print both `bounds=` and `text=` rectangles.

An actual C++ geometry audit used the cached staged original fonts/labels and
the last runtime log's rounded model bounds. All 13 labels fit horizontally in
their corrected text rectangles: button text width is approximately 309.4 pixels
and the widest button caption is 204 pixels. This also means simple caption
width alone did not explain the reported border overflow. The user subsequently
confirmed that the inset correction works as expected. The direct runtime log
shows the corrected text rectangles, Exit activation, 971 submitted frames and
normal cleanup. No guessed offsets, clipping or smaller fonts were introduced.
No new Ghidra export or unit tests were needed.

The strict i386 host build, existing native geometry checks and UI evidence
validator pass (30 instruction/data anchors, plus 3 engine and 2 client vtable
slots). No agent-environment Proton retry was attempted.

#### Main-menu quit-confirmation contract (2026-10-27, experimental integration)

Cached definitions and section-mapped original assembly distinguish two paths
using the same `confirm` model. The fourth main-menu screen in `00623DD0`
is `CONFIRM` / `confirm` (definition at `00625B7E`). Its prompt uses style **12**,
localization 2004; Yes/No use style 13 and localizations 2001/2000; `bok` is
hidden. The literal definition names are `byes*` and `bno*`, including their
asterisks; these are not proven exact runtime frame names.

At stack depth one, shared router `0066C950` handles Exit `06001003` by queuing
`02000002` with screen number 4, then deferred quit `22000004`. Yes emits
`14001100`: the router's bit-100 reply branch moves deferred events to the main
queue with bit `20000000` cleared. No emits `14001200`: the bit-200 branch
discards the deferred queue. Both then emit `06001005`, whose masked handler
`0066D043` removes the current screen. Released quit `02000004` reaches
`0066D33B` via table `0066D68C`, invokes an application callback and stores 4 in
front-end state `+4`. These are binary-checked research anchors in the
[UI map](analysis/menu-ui-map.json), not a host implementation of the dispatcher.

Separately, common resource loader `0066C1D0` creates a 0x90-byte confirmation
object at front-end `+100`, vtable `008155D8`. Initializer `00668790` uses prompt
style **24** and callback `006693A0`, not the main-menu definition's style 12 and
queued reply events. Independently, main-menu screen setup `00627060` compares
the fourth definition's identifier against `10000000` at `0062711F`, and sets
that model's position to `(0.5, -0.375, -0.5)` at `0062712F..00627147`.

The original archive model is 4095 bytes, SHA256
`4d4844844947665c98725422d69a081556d27abb01efe4b2ff09d338b58d37ac`.
Its first-table dependencies are `X_B_DNLF.BMP`, `X_B_DNMD.BMP`,
`X_B_DNRG.BMP`, `X_NMNBTN.BMP`, `X_NMSHD.TGA`; only `X_NMNBTN.BMP` is already
in the main-menu slice. All were located and decoded from the base archives.
[Resource staging](tools/stage_menu_resources.py) now supports opt-in collection:

```sh
python3 -B tools/stage_menu_resources.py \
  --game-dir "Hidden and Dangerous 2" \
  --output analysis/work/engine-window/run --include-confirm
```

Default fresh staging is unchanged. Upgraded manifests retain the optional
model and dependencies during later default launcher calls rather than
orphaning their provenance. Staging alone does not display a confirmation:
the host still exits directly. The opt-in diagnostic below opens that model
alone at the verified position and logs its original runtime hierarchy; it
does not claim to reproduce modal composition, text or input:

```sh
python3 -B tools/run_engine_window.py --confirm-model --frames 0
```

Run it from the user's desktop terminal and exit with Escape or window close.
The first run revealed `byes?` (`normal02`/`actived03`), `bno?`
(`normal01`/`actived01`), `bok` (`normal03`/`actived04`) and `text`.
The updated diagnostic initializes these three original visual pairs and logs
the projected prompt and Yes/No bounds; it does not yet hide the disabled `bok`
or display text. The user observed that diagnostic, and the later interactive
confirmation described below. The host log is overwritten each run.
No new unit tests or exports were added for that diagnostic.

The original definition's `01000004` for `bok` is a **hide** event: control
dispatcher `0065CDA4` and jump-table entry `0065CEDC` call vtable `+14`, button
callback `0065C790`, which clears visibility and hides/updates the bound frame.
The [UI map](analysis/menu-ui-map.json) anchors this chain. The experimental
`--menu-confirm` mode now loads the original confirmation model and labels,
hides `bok`, and uses host-owned Yes/No clicks. Exit opens it, Yes closes the
host, No and Escape return to the menu. It is not the original event dispatcher;
keyboard/default-focus behavior remains incomplete:

```sh
python3 -B tools/run_engine_window.py --menu-confirm --frames 0
```

Default `--menu-ui` still exits directly. The real original labels fit their
projected inset rectangles (98, 23 and 19 pixels), but this is geometry evidence,
not rendering proof. No new unit tests.

Desktop confirmation showed working No and Yes, but menu captions appeared over
the popup and the prompt had a grey box behind it. Original render `0066DE00`
draws the previous screen, disables driver state 5, switches model visibility
and draws the current screen, then restores state 5. The host now uses these
two passes and places menu glyphs before the confirmation pass, not after it.
The user confirmed the corrected layering on the desktop.
Static text initializer `0065C680` sets its visual direction `(0,0,-1)` through
vtable `+0C` before calculating bounds. The experimental host now reproduces
this using `1001C8D0` for the prompt frame; the user confirmed the grey box is gone.

#### Original button visual selection (2026-10-27, desktop-confirmed)

The original type-14 button initializer (`006550E0`) binds `normal*` and
`actived*` model children at control `+70` and `+74`. If active frame flag
`+E0 & 0x80` is clear, `0065513B` copies the normal visual's local XYZ at
`+B0/+B4/+B8` into the active visual, clears dirty-mask bits `0x120` and sets
`0x40000000`. It then hides active through frame vtable `+30` and explicitly
updates it through `+24`. These are cached-binary findings, not new exports.

The focus callbacks at button vtable `+30/+34` (`00655250`/`006552A0`) show
active/hide normal on focus and reverse that selection on focus loss. Both
visuals receive explicit updates after each switch. The engine's visibility
method `1001CFE0` takes stack self plus a byte in the next stack word, changes
visibility bit zero at frame `+E0`, invalidates ancestor visibility and cleans
eight stack bytes.

The new [button adapter](recovered/host/menu_button_visuals.h) uses the existing
bounded exact-name lookup, checking both visual vtables and method addresses.
It initializes all nine pairs in normal state only in `--menu-ui`. Only Exit
changes state on hover. Its local-position/dirty-flag bridge reproduces the
original conditional copy directly rather than incorrectly applying a
world-space position setter. All pointers are borrowed from the menu model;
they are neither independently released nor retained beyond that model.
One cursor sample drives both artwork and text before the scene is rendered.
Cursor absence (including focus loss or leaving the client) restores normal.
The Exit click rectangle and text inset are unchanged.

This is original model-visual selection driven by host-owned focus detection,
**not** the complete original button dispatcher or an invented animation.
Original callbacks also emit tagged focus events `0x04003000`/`0x04004000`;
their event/sound routing and the original confirmation flow remain deferred.
The artwork-only preview does not initialize or switch button pairs.

The focused and unfocused callback sequences are now represented in
[client reconstruction code](recovered/frontend/button_focus.h), including
the byte `+64` focus-entry gate, ordered frame calls, event values and return
values. The existing host visual adapter invokes them through an operations
interface. Its controls are treated as unblocked; event dispatch is currently
limited to the portable subset described below, and the original object ABI
is not reconstructed. Both callback functions are counted as partial, not
complete. The host's preexisting normal-state initialization is unchanged and
does not manufacture a focus-loss event. The user confirmed the previous wiring
works on the desktop; the log records Exit focus transitions, No and Yes and
clean teardown. New dispatcher wiring described below needs another check.

#### Control notification and tagged routing (2026-10-27)

In `0065C900`, control `+40 == 04021000` selects the override callback at
`+48`, with receiver `+44` and a zero first stack argument. Otherwise it scans
12-byte observer entries at `+30` in order, compares the full tag, skips null
callbacks, and invokes the first match with its stored context. No match returns
true. The callback's result is ignored by its caller `0065CF50`.

That caller subsequently scans 24-byte event records at `+1C`, selects records
where `(code & 040FF000) == tag`, copies each record and clears those selector
bits with `FBF00FFF`. Target `FFFFFFFF` calls the emitting control's vtable
`+6C`; other targets enqueue the copied record. The original copy retains and
releases the payload at `+14`. The portable
[routing core](recovered/frontend/control_events.cpp) implements this ordering
and filtering with explicit callback and destination interfaces, not the
original object layout, intrusive reference counting or queue. Its two
functions are partial.

The base constructor `0064DCC0` initializes event and observer vectors empty.
Control creation `0065D150` calls button vtable `+78` (a no-op at `0065CD30`)
with definition `+20`, then `+70` (`0065CF00`) with its `+10` initialization
list. That function passes each record to vtable `+6C`; the button and text
handlers forward tagged records to `0065CD40`, whose tagged branch appends
the record to the control vector. Exit defines `(0, 06001003, 04000000)`,
Yes defines `(0, 14001100, 0)` then `(0, 06001005, 0)`, and No defines
`(0, 14001200, 1)` then `(0, 06001005, 0)`. All have activation selector
`04001000`, so none match focus entry/leave selectors.

The host registers these three controls' known tagged records and sends
their focus callbacks through the portable routing core. Confirmation focus
changes now invoke leave only for the
previously hovered control and enter only for the newly hovered control,
avoiding the prior redundant focus-loss emissions. The user confirmed the
focus-filter wiring works. The tagged branch of `0065CD40` is counted partial.

The button activation callback `006552F0` checks the same `+64` gate and emits
`04001000` with argument zero. Its matching records pass through `0065CF50`,
which clears selector bits before queuing them. Original FIFO pump `0066C770`
removes the front record, calls `0066C950`, and repeats while records remain.
At depth one the Exit event queues screen-four push and deferred quit. Yes's
reply releases the deferred quit onto the back of the main queue, **after** the
already queued pop; No discards it. Screen-four Escape in `00626B20` queues
`14001200` then `06001005` directly.

The [bounded portable queue](recovered/frontend/menu_event_queue.cpp) now
implements those paths and drives host visibility and quit effects in
`--menu-confirm`. Unsupported local, targeted, screen and event cases report
errors. The host still detects mouse presses itself and pumps immediately;
original object layout, intrusive ownership, general routing, full screen
callbacks and application quit callback remain unreconstructed. The default
`--menu-ui` direct-Exit path is unchanged. The user confirmed the queue
integration works. The latest host log independently records Exit, No, Exit,
Yes, pop before quit and clean teardown; it does not independently show Escape.
The associated functions are tracked partial.
`006552F0` and `00626B20` are absent from the frozen inventory, so they receive
no function-count credit; the inventoried queue pump and router are partial.

The original input update `0066B990` obtains a picked frame via `0066A7C0`,
updates mouse state through `0066A220`, and routes input to screen callbacks.
In `0066A220`, a selector-zero press transition stores the current picked target
at input state `+4C`; on release, it compares that stored target with the
current target, writes the result at `+49`, then clears `+4C`. It separately
tracks double-click timing with a 400-unit threshold. The bounded
[pointer click reconstruction](recovered/frontend/pointer_click.h) implements
only that target capture, equality and clear, using portable integer identities.
The `--menu-confirm` host supplies its rectangle-based identities and retains
its existing cancellation on pointer departure, deactivation and Escape.
Complete input polling, double-click dispatch, exact timing, screen callback
routing and the original object ABI are not reconstructed. In particular, original
`0066A220` does not itself clear the captured target merely on pointer departure.
The user confirmed the click integration works. A later log also records Escape;
the original-versus-host drag-off behavior remains unverified.

For the menu screen vtable at `00815540`, the pointer-focus handler `00656C30`
looks up the target control and checks the index. An invalid target invokes
screen focus-leave `00656B90`; a target matching the current index invokes
the control's `+40` method. For a different valid target it first invokes
focus-leave; a zero leave result causes `00656C30` to return one without entering
the new control. Otherwise it calls focus-entry `00656B20`. The ordinary
`00656B20` branch checks the resolved control's `+08` eligibility, stores its
index and calls its `+30` method. `00656B90` clears the focused index only after
the old control's `+34` method succeeds. For these buttons, the control's
`+40` method is `0064DAE0`, which emits `04005000` and returns seven.

The portable [screen focus subset](recovered/frontend/screen_focus.h) implements
this routing and the ordinary entry branch. In `--menu-confirm`, host adapters
assume eligibility and invoke the router only when sampled hover changes.
Special target 15, complete input-update timing and screen/control object ABI
remain excluded. The user confirmed this integration works; the latest log
independently records No and Yes and clean teardown, but not Escape.

Screen target lookup `0065C830` scans controls in order and returns the first
matching identifier field, or -1. If screen `+60` has bits `000FF000`, it
compares bits `00000FF0`; otherwise, if it has bits `03F00000`, it compares
`000FF000`; otherwise it compares `03F00000`. Screen initialization `00668470`
copies definition `+40` to screen `+60`. `00615360` constructs a child
definition ID by ORing the parent ID with `((count + 1) & 3F) << 20`, and
`0064DCC0` copies that ID to control `+60`. Button initialization `006550E0`
copies the control ID to both normal and active frames at `+EC`, the value
picked by `0066A7C0`.

The confirmation definition order therefore yields screen `10000000`, text
`10100000`, Yes `10200000`, No `10300000` and OK `10400000`, assuming all
controls are constructed in definition order. The
[portable lookup](recovered/frontend/screen_target.h) implements the original
mask precedence and ordered comparison. The host maps rectangle hits to the
two button IDs rather than performing original frame picking; other controls
are not reconstructed as host objects. `00656B20` and `0065C830` are absent
from the frozen inventory and receive no function-count credit. The user
confirmed the lookup integration works; the latest log independently records
No and Yes and clean teardown, but not Escape.

Cursor coordinate helper `0066A120` computes `x / width`,
`-(y / height) * 0.75`, and `z = -1`. `0066A7C0` adds the original `0.1`
offset to z, supplies direction `(0,0,1)`, and calls scene vtable `+104`
with the current screen model and flags `00140000`. The scene slot points to
engine `100110F0`, whose `ret 24` confirms nine stack words including self.
On a hit the client reads frame `+EC` into input state `+38`; otherwise it
stores zero. The [portable ray and target subset](recovered/frontend/pointer_pick.h)
and a guarded host adapter now implement this sequence for an optional
comparison probe. The host supplies cursor pixels instead of original input
globals, initializes vector padding and a 64-byte scratch result, and tags the
two Yes/No visual pairs. The probe logs engine versus rectangle identifiers
only when they change. The user confirmed the diagnostic works; the desktop
log first showed matching Yes and No samples, plus one sample with engine No
and an empty rectangle. A later log also showed an engine Yes/empty rectangle
sample. Positions were not recorded, so the exact
locations and broader edge behavior remain unknown.

The separately gated `--use-confirm-picking` experiment now routes
confirmation hover and press/release target selection through the original
engine picker. The comparison probe can be enabled simultaneously without
deciding interaction. The default `--menu-confirm` remains rectangle-driven.
Even in the new mode, the original input-state layout, polling, per-update
timing, complete screen construction, other control objects and the original
pointer-departure policy remain unsupported. The user confirmed interactive
picking works; the latest log shows its startup message, No, Escape, Yes, 741
frames and clean teardown. It does not establish exact edge locations.

Mouse-motion predicate `00669FF0` returns true when either float delta at input
`+14` or `+18` differs from zero. The ordinary `0066B990` movement branch
checks that predicate and its drag state before calling screen vtable `+40`;
if the target is already focused, `00656C30` calls button vtable `+40`, which
emits `04005000` and returns seven. The opt-in interactive picker now forwards
sampled host cursor movement through this path. It logs the first such button
event per button but dispatches every sampled event. Absolute host-position
differences replace original clamped relative input, and drag gating, screen
stack, exact update ordering and timing are not reconstructed. A changed
target still triggers host focus routing even without sampled movement. The
latest desktop log records a motion event for each button.

Screen click callback `00656DD0` invokes the focused control's vtable `+5C`,
or returns seven for an invalid focused index. For these buttons, `0064DBC0`
emits `04012000` with zero argument, forwards the pointer target to virtual
activation `+38`, and returns its result. Button activation `006552F0` ignores
the target, checks its `+64` gate and emits `04001000` on success. The
[portable click wrapper](recovered/frontend/button_input.h) now performs
that event/activation order for the experimental Exit and Yes/No buttons.
The latest log confirms it for Exit, No and Yes.

The [portable screen click subset](recovered/frontend/screen_input.h) now
validates focused index and delegates its click callback, or returns seven for
invalid focus. The confirmation host uses it when sampled focus equals the
clicked hit. If sampling is stale or absent, the host explicitly logs the
mismatch and retains the previous hit-based choice, avoiding a wrong-button
activation. This fallback and the incomplete input update order are not the
original dispatcher. The portable button activation omits the unused target.
The user confirmed the screen-click integration works; the latest desktop log
shows it for No and Yes, with no logged fallback.

On release `0066A220` sets the double-click flag only if captured and current
targets match, the previous clicked target matches, and the elapsed counter is
strictly below 400. Each release resets that counter and capture; a double
click clears the previous clicked target. `0066B990` invokes screen `+60`
before ordinary click `+5C`; `00656E00` forwards `+60` to the focused control,
whose button implementation `0064DBE0` emits `04013000`. The ordinary button
click `0064DBC0` emits `04012000` and invokes activation. The original counter receives a bounded, potentially scaled millisecond frame
delta (see below). Complete polling and screen-update integration remain absent,
so double-click events are not dispatched by the interactive host.

`0066A050` treats selector zero as any nonzero button state; a nonzero selector
tests for any overlapping bit. `0066A080` and `0066A0A0` apply this to previous
and current state, while `0066A0C0` and `0066A0F0` detect press and release
transitions. Thus `0066A220` captures on transition from no buttons to any,
not specifically a left-button transition; `0066B990` uses selector 3 for
its press/release callbacks. Engine `1006FB30`, at graphics vtable `+104`,
combines five high-bit button bytes into bits 1, 2, 4, 8 and 10 hex.
Constructor `0066A4B0` initializes the captured target, previous clicked target
and click timer to zero.

The [portable button state](recovered/frontend/pointer_buttons.h) and
[click-timing shadow](recovered/frontend/pointer_click_timing.h) implement
these predicates and the click/double-click bookkeeping, including the signed,
strict `<400` comparison. The existing comparison probe now observes them with
Win32 mouse-message button snapshots and picked IDs. It uses message time in
milliseconds as an approximation, not a verified original clock, resets shadow
state between confirmation sessions or on focus loss, and does not dispatch
its results. The probe does not replace original frame polling or interactive
host cancellation. The user reported it works; the latest log records matched
left-button captures and `click=1 double=0` for No and Yes. It does not record
right-button transitions or `double=1`, so those cases are not desktop-verified.

Original frame timing and bounded input sequencing (2026-10-29): cached exports
and section-mapped assembly establish the chain `006DC950 -> 006DCAE0 ->
006C20F0 -> state +14 / 0066BEA0 -> 0066B990 -> 0066A220`.
`006DC950` obtains the performance counter/frequency, computes integer
microseconds, takes the wrapping unsigned low-word difference and divides by
1000. The main loop requests a 10 ms minimum and 100 ms maximum. Both limiting
branches resample the baseline; the minimum branch waits on a multimedia timer
and returns the requested minimum rather than the measured post-wait interval.
Before dispatch, states with status 1/6/5 can apply an externally supplied time
scale with integer remainder carry. Thus the 400 threshold measures accumulated
update milliseconds, potentially scaled, rather than raw message-time distance.
The scale provider's full semantics are unresolved. The front-end vtable +14 is
already verified in `analysis/frontend-map.json`; the new call/argument and
arithmetic anchors are in `analysis/menu-ui-map.json`.

`0066BEA0` processes queued events before input and updates bound screen objects
after input. `0066B990` first picks with the existing cursor state, then invokes
`0066A220`; it does not move the cursor and then pick. The input update copies
previous buttons, polls current buttons, advances both counters, clears output
flags and reads relative XYZ. A positive signed +44 countdown subtracts the
frame delta and returns immediately, even if subtraction crosses zero. Otherwise
unchanged XYZ/buttons/target returns early. Processed input resets the idle
counter, adopts the picked target, integrates/clamps relative motion and, unless
byte +48 inhibits it, performs any-button capture/release bookkeeping. This
ordering also means suppressed button transitions are consumed, not postponed.

The [bounded input update](recovered/frontend/pointer_update.h) reconstructs
snapshot/click-counter ordering, signed suppression, unchanged-sample gating,
target adoption and click inhibition around the existing predicates. It now
includes cursor integration/clamping and idle bookkeeping; engine frame updates
remain omitted. Only the
observational probe uses it, with host-picked IDs, approximate host XY deltas and
Win32 message-time differences; it still samples messages rather than original
frames. Logs now include `delta_ms` and `processed`. The host leaves suppression
and inhibition clear. Their code paths are assembly-backed, not desktop-tested.
No interactive double-click routing or cancellation change is implied.

The next [pointer dispatcher subset](recovered/frontend/pointer_dispatch.h)
reconstructs the non-wheel sequencing of `0066B990` through portable callbacks.
It preserves the +24 gate before pick/update, initial previous target `FFFFFFFF`,
processed-or-target-change gate, and alternate +2D motion branch. Ordinary
mask-3 press calls screen +48; when a captured screen control has another ID,
its +64 callback must return exactly one before capture is cleared and dispatch
continues. This capture is distinct from the input object's press-target capture.
On mask-3 release, an active drag is cleared before screen +54, focus +40 and
release +44. The +4A double-click flag triggers +60 before +49 ordinary click
triggers +5C; ordinary click is gated by captured-control identity. Return one
from +5C invokes `0066B110`, which checks front-end +3C, calls its engine +78
with zero, and on false calls +30 with one. That helper remains delegated;
its purpose is not assumed to be sound playback.

After those callbacks, XY motion invokes +40 when not dragging. Held mask 3
then invokes +58 before setting the drag flag, invokes +50 and returns before
wheel processing. The wheel path's float conversion, +4C and fresh pick/focus
remain an explicitly delegated hook. Each screen callback requires a fresh
stack lookup; caching a screen across callbacks could use a screen that has
already been replaced. Input flags are likewise queried after prior callbacks.
The alternate +2D path only clears drag and calls +40 on XY motion; it skips
press/release, clicks and wheel. This recovers call order and gates, not the
original objects, stack storage or callbacks themselves.

The [host dispatch probe](recovered/host/pointer_dispatch_probe.h) exercises
that function only through logging. Host cursor-position differences now supply
approximate XY motion to the shadow, so logs can show press +48, drag +58/+50,
end +54, focus +40, release +44 and click +5C in sequence. Consecutive identical
focus/drag logs are suppressed; dispatch still occurs. No actual UI callback,
activation or accepted-click side effect runs. Portable screen callbacks now route through simulated focus and Yes/No controls;
invalid-focus and release-target results follow the original screen helpers.
Simulated child callbacks return seven rather than activating buttons. Captured screen
control state stays absent and no wheel samples are supplied. New logging and
these branches have not been desktop-verified; interactive input is unchanged.

The [cursor accumulator](recovered/frontend/pointer_cursor.h) now reconstructs
`0066A220`'s XY and wheel arithmetic. It first rounds each position sum to a
float, then clips XY inclusively to `[0,width]`/`[0,height]`. For a negative
position it subtracts that stored position from the sampled delta before setting
position to zero. For upper overflow it subtracts `(stored_position - limit)`
from the delta without an intervening float store of the overflow. The code uses
`long double` for those x87 intermediates (the i386 toolchain reports a 64-bit
mantissa), with explicit float stores at original boundaries. It does not model
the original FPU control word or claim bit equivalence for all inputs. Replacing
the delta calculation with `clamped_position - previous_position` would change
the rounding path. At an edge, outward motion can become zero effective motion;
the dispatcher now reads this corrected delta. Wheel accumulates without XY
clipping. Idle +40 advances before early returns and clears on processed input,
including a picked-target change. Suppression leaves sampled deltas intact and
does not integrate the cursor. The shadow seeds XY/ranges from the first
available host point on entry/reentry; this remains approximate host sampling.

Six additional [screen input helpers](recovered/frontend/screen_input.h)
reconstruct `00656CA0/CD0/D10/D40/D70/E00`. Press (+48), drag (+50), drag end
(+54) and drag begin (+58) validate focus and forward to the corresponding
control slot, returning zero for invalid focus. Release (+44) additionally
resolves the target and requires its control index to equal the focused index.
Double-click (+60) forwards to focus or returns seven for invalid focus. The
confirmation screen vtable sequence at `00815584` verifies these mappings,
including the intervening scroll slot +4C (not implemented here).

The shadow now routes through these helpers and the existing focus/click
helpers, maintaining a separate simulated focus. Only Yes/No are eligible;
enter/leave always succeed and child pointer/click callbacks return seven.
Logs include focus, callback result, accumulated cursor and corrected XY motion.
No actual control events, activation, artwork changes or live double-click
routing are introduced. The six screen functions are counted as partial because
their object ABI, virtual storage and child callbacks remain adapters. Totals
are now **35 partial / 0 complete**, with the denominator unchanged at 18,783.

Assembly also resolves the engine polling contract: graphics +E8 maps to
`1006F8A0`, while +EC/+F0/+F4 map to `1006FA10/20/30`. The poll requires the
input device at `100D2148`, reads a 20-byte record into `100D1EC8` via device
+24, attempts acquisition via +1C and one retry on failure, and clears all five
DWORDs with error `80000005` after a second failed read. A missing device
returns `80000009`. The three getters return stored signed XYZ without consuming
or repolling them. Thus the frontend reads one previously polled frame snapshot.
Successful-path cursor warping, accumulated absolute coordinates, swapped-button
handling and full device/focus setup still need reconstruction. No host calls
to these input slots were introduced.

Functional engine-button confirmation bridge (2026-10-29): the next step toward
a rebuilt menu is the engine boundary its input actually needs, rather than
reconstructing unrelated rendering/gameplay systems first. The new
[mouse poll slice](recovered/input/mouse_poll.h) reconstructs `1006F8A0`'s
device-read branch and `1006FB30`'s button-mask packing. Its exact 20-byte state
has signed XYZ followed by eight button bytes; only the first five high bits
form the mask. Missing device returns `80000009` without clearing state. Failed
read ignores the result of one Acquire attempt, retries once, then clears all
20 bytes and returns `80000005` on another failure. Successful reads clear Z
when capabilities report fewer than three axes and swap button bytes zero/one
when the engine's swap setting is enabled.

The [engine mouse adapter](recovered/host/engine_mouse.h) checks original
setup/enabled/shutdown methods at graphics +DC/+E0/+E4, creates a mouse with
argument zero (foreground nonexclusive cooperative flags six) and borrows the
device pointer at module+`D2148`. Its +24 reads the original state format and
+1C acquires it. Teardown calls the engine's +E4, which owns Unacquire/Release
and clears the pointer/state/enabled flag. Constructor failures clean up after
a successful setup. The rebuilt poll stores state separately; it does not patch
the engine or publish replacement methods into its vtables. Nonexclusive-mode
unused Win32 cursor/window metric queries are omitted. Exclusive-mode wrapping
absolute accumulation is excluded; no cursor warp, clip or exclusive acquisition
was added. Original DirectInput creation/data format/capability setup and device
shutdown still run in LS3DF.dll, so this is not a fully rebuilt input subsystem.

`--menu-confirm --use-confirm-picking --use-engine-mouse` now bypasses
message-based confirmation button handling and runs the reconstructed pointer
update, dispatcher and screen helpers once per frame using those device buttons.
The main menu's Exit remains message-driven. Host cursor coordinates provide
hover/picking and motion so they stay aligned with the displayed cursor; the
read raw engine XYZ is not yet used for live cursor movement. Hover and click
share a single frame sample. Host focus routing precedes dispatch and the queue
pumps after dispatch; these are explicit integration differences from the full
original frame loop. QPC supplies wrapping microsecond differences converted to
clamped 10..100 ms; original minimum wait and time scaling remain absent.

Button +44/+48/+50/+54/+58 emit `04007000`/`04006000`/`04008000`/`04009000`/
`0400A000`, respectively, then return seven. The
[button pointer wrapper](recovered/frontend/button_input.h) now reproduces that
sequence and the separately gated +60 `04013000` event. Live double-click and
wheel dispatch remain disabled. Tagged dispatch uses original event records;
none of these new press/release/drag events match the bounded activation list.
Click still emits `04012000` followed by activation `04001000`. Repeated pointer
event log lines are limited per type per button; event routing still occurs.

Leaving a button cancels its captured press. Outside-window/focus loss, polling
failure and closed confirmation clear frame input; reentry must first observe
all buttons up. These host safeguards remain deliberate deviations. The engine
bridge emits `Rebuilt engine mouse polling enabled`, poll status transitions and
`Engine mouse confirmation click` logs. The accepted-click helper and original
captured screen-control objects remain absent. No new desktop claim is made.

Desktop terminal command (the agent must not launch Proton):

```sh
python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0
```

Check Exit, No, Escape and Yes, plus focus loss and press-drag-off-release. The
old command omitting `--use-engine-mouse` restores message-driven confirmation.
Desktop follow-up (2026-10-29): user reports it works well and right-click also
activates buttons. This matches original mask-three screen transitions and
selector-zero (any-button) click capture/release; no left-only restriction was
added. Additional original filtering elsewhere remains unverified. The direct
log confirms rebuilt polling, one `80000005` failure followed by recovery,
No three times, Escape, Yes, pointer events, 807 frames and clean teardown. Raw
button masks are not logged in this mode, so right-click identity is based on
the user report. Focus-loss and drag-off results are not established by this log.
The new flag requires both confirmation and engine picking in launcher and host;
all existing modes retain their prior paths. Function accounting stays at
35 partial / 0 complete: engine `1006F8A0`/`1006FB30` and these tiny button
callbacks are absent from the frozen inventory and receive no ledger credit.

Wheel continuation (2026-10-29): the engine-input mode now feeds polled Z into
the reconstructed update and dispatcher. `0066A020` reads the stored float delta
and `0066A030` tests it against zero. There is no division by 120 in this path.
At `0066BDCB`, the dispatcher calls `007E5E00`, which temporarily selects x87
truncation, converts to signed int64, restores the control word and returns
EDX:EAX. The caller passes only EAX to the screen. The
[wheel helper](recovered/frontend/pointer_wheel.h) preserves truncation and
low-word semantics without undefined out-of-range C++ casts; masked-invalid
integer-indefinite has low word zero. Original FPU flags and unmasked traps are
not reproduced, and that helper is not claimed as a complete runtime rebuild.

The [screen wheel subset](recovered/frontend/screen_input.h), `00656DA0`,
validates focus and forwards both target and wheel word to control +4C, or
returns seven. Button `0064DB40` ignores those parameters, emits `04011000`
with argument zero and returns seven. It does not activate. After screen +4C,
`0066B990` resolves the current screen/model again, repicks and invokes +40.
The portable dispatcher now expresses this order directly, with original stack
storage/lookup supplied by context adapters. Existing drag early returns and
alternate-motion gating still skip wheel. The functional adapter repicks the
original confirmation model using the same frame's host cursor point, so a
second asynchronous OS cursor sample cannot change the target mid-dispatch.
The message shadow still receives no wheel messages, but its context implements
the same helper interface. Live double-click dispatch remains disabled.

No new flags are required: run the existing `--use-engine-mouse` command and
scroll over Yes/No and outside them. Expected visible behavior is no activation
or dismissal; the log should show `Engine mouse wheel` and, for a focused
button, `04011000`. New wheel behavior has not been desktop-verified. Three
additional tracked functions are partial (`00656DA0`, `0066A020`, `0066A030`),
bringing totals to 38 partial / 0 complete; frozen denominator stays 18,783.

Cursor-frame investigation also identified a prerequisite: `0066C5B0` pushes
`kurzor_menu` and `CURZOR` at `0066C63C/41`, then calls model loader `0066BFA0`.
The input initializer later finds that frame and updates its +08/+24 transform
before hiding it through +30. The host currently stages neither that model nor
its scene binding. Recover/stage the original resource and ownership path before
calling frame setters on an invented substitute. The working visible bitmap
cursor and its projection remain unchanged in this slice.

The strict i386 build, existing native geometry checks and expanded UI evidence
map pass (40 instruction/data anchors, 5 engine and 4 client vtable slots).
No new unit tests were added. The latest desktop run logged
`Original button visuals ready: 9 pairs; Exit hover only`, repeated
`Exit visual state: focused`/`normal` transitions, Exit activation, 575 frames
and clean teardown. The user confirmed it works as remembered. This is bounded
desktop confirmation, not exhaustive pixel-perfect or keyboard behavior proof.

The overlay is independent integration glue. It borrows the original D3D8
device through a fingerprint/relocation-guarded field, snapshots and restores
each render/stage state it changes, the stage-zero texture and pixel shader,
and restores the viewport through IGraph. Glyphs still use the verified original
texture wrapper and IGraph primitive method. An all-state block is deliberately
avoided: restoring FVF directly would invalidate IGraph's cached vertex format.
This explicit setup is not claimed to be the original client state-save protocol.
Unsupported controls use the original disabled text alpha; Exit's hover uses
the original focused-text white and the model-visual selection described above.

Validation:

```sh
make -C recovered test
TMPDIR="$PWD/analysis/work" python3 -B -m unittest discover -s tools -p 'test_*.py'
python3 -B tools/validate_engine_host.py --map analysis/menu-ui-map.json
recovered/build/bitmap_font_tests "Hidden and Dangerous 2" analysis/work/engine-window/run/MenuText/labels.bin
```

The direct Windows log should list all 13 label bindings/bounds, then
`Original menu bitmap glyphs submitted`. Activating Exit should log
`Exit control activated`, followed by driver and graphics cleanup. These
diagnostics were observed in the successful run: all 13 bindings, glyph/cursor
submissions, Exit activation, 308 submitted frames and normal driver/graphics
close. This does not establish every hover, drag or focus-cancellation edge case.

The first desktop text attempt stopped before rendering with
`Missing original menu frame: text_screen`, then closed driver/graphics normally.
The host had confused the separate auxiliary string at frame `+F0` with the name.
The actual name setter (`1001D330`, vtable `+34`) reads/writes `+E8` and copies
the supplied NUL-terminated name there. The corrected, native-tested
[name reader](recovered/host/frame_name.h) distinguishes both fields, including
an unnamed frame with a nonempty auxiliary string. Hierarchy diagnostics now
print names as well as types/counts. The correction was committed as `54a7c67`;
the subsequent desktop run succeeded. The later inset correction and its
desktop confirmation are recorded above.
See [the session handoff](HANDOFF.md) before continuing in a fresh session.

## First mission-format milestone

See [the mission findings](analysis/mission-format.json) for sample fingerprints,
static anchors, counts, and limitations.

The ALPS1 sample comes specifically from the **base `missions.dta` archive**,
not an established runtime/patch overlay. The native reader extracted three
members without using game DLLs or executing game code:

| Resource | Bytes | Observation |
| --- | ---: | --- |
| `scene2.bin` | 6,727,229 | Root size exactly matches the file |
| `scene.4ds` | 1,380,788 | `4DS\0` header, version `0x29` |
| `loader.4ds` | 3,901 | Same header/version |

`scene2.bin` uses six-byte little-endian headers: a two-byte ID and four-byte
**total size including the header**. Its root ID is `0x4C53`, with 15 direct
children. The `0x4000` container at file offset `0xFA` contains exactly 4,008
`0x4010` records. Every record's immediate children also consume its payload
exactly; all have one `0x0010` field containing a NUL-terminated name-like string.
This is not an enemy count: records include scene objects whose roles are
mostly still unknown.

Static code corroborates this framing: `006CBF30` checks the root, `006CC760`
dispatches top-level chunks, and its `0x4000` handler calls `006CD940` for
`0x4010` records. `00405940` implements a read bounded by the current chunk.
The inspector descends only through these understood container levels; it does
not guess that arbitrary payloads are nested chunks.

### Improving the decompilation

Assembly at `007E5F60` implements an x86 stack-probe/allocation helper. The object
reader requests `0x23F8` stack bytes through it, but Ghidra had no call fixup
attached. Applying its built-in `alloca_probe` model removed the impossible
stack array and reduced `in_stack_` occurrences in `006CD940` from 122 to zero.
This improves analysis, not the executable: no game bytes were modified.
Type propagation warnings and incomplete helper/interface signatures remain;
the resulting pseudocode is **not** a recovered, trustworthy C implementation.

The fingerprint-guarded [ConfigureStackProbe.java](tools/ConfigureStackProbe.java)
script reproduces this metadata change. Use it **only in the independent
research project**, without `-readOnly` if saving:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process hd2.unpacked.exe -noanalysis -scriptPath "$PWD/tools" \
  -preScript ConfigureStackProbe.java \
  -postScript ExportFunctions.java "$PWD/analysis/work/probe-export" 006cd940 \
  -max-cpu 2
```

### First readable object fields

The next obstacle was missing argument lists: Ghidra's archive imports were
marked `__stdcall` but had no parameters. The DLL export decorations and caller
instructions establish three stack arguments for `dtaRead`, `dtaWrite`, and
`dtaSeek`. Three chunk helpers also needed explicit `__thiscall` signatures.
[ConfigureChunkReaders.java](tools/ConfigureChunkReaders.java) applies these
fingerprint-guarded corrections in the research project. Reopening that project
and applying the script again produced unchanged signatures.

The chunk-entry helper now correctly recovers the six-byte header and output
ID. In `006CD940`, bare `dtaSeek()` calls dropped from 96 to zero, and the
pseudocode became shorter without deleting any instructions. That allowed
selected fields to be followed into their consumers:

| ID | Decoded interpretation | ALPS1 occurrences |
| --- | --- | ---: |
| `0x0010` | Object name | 4,008 |
| `0x4011` | Creation-kind request, unsigned 32-bit | 1,275 |
| `0x0020` | Position, three float32 components | 1,292 |
| `0x0022` | Quaternion rotation, file order `w,x,y,z` | 1,332 |
| `0x002C` | Fallback position for failed parent-name lookup | 1,275 |
| `0x002D` | Scale, three float32 components | 1,277 |
| `0x4020` | Parent reference with a nested name field | 1,200 |
| `0x2012` | Model resource name with a derived lookup key | 1,157 |

The position consumer writes frame offset `+0xB0`. Rotation is reordered to
engine `x,y,z,w`, copied to `+0xC0`, and normalized through the named
`S_quat::Normalize` import. Scale reaches `+0xD0`; the named engine
`I3D_frame::UpdateWMatrixProc` uses its three components as basis multipliers.
All 1,332 sample rotations have squared length within `1e-6` of one. The tool
reports serialized components without normalizing them or calculating resolved
world transforms.

**Not every record creates an object.** The creation kind starts at zero for
each record. When a name field is encountered, zero selects lookup of an
existing name-table entry; nonzero selects a create-if-missing branch. In this
sample there are 2,733 lookup requests and 1,275 create-if-missing requests.
The latter contain kinds `2` (13 lights), `3` (2 cameras), `6` (103 dummies),
and `9` (1,157 models). These names are now supported by the engine's own
type-name query, not guesses from the mission data. They remain serialized
requests, not counts of successfully instantiated runtime objects.

The loader populates a name table through callback `006CC580` before parsing
the records. The callback reads a frame's name and stores its pointer in the
table. All 4,008 record names are unique, but only 200 of the 1,200 parent-name
references also name a record in this file. This does **not** establish 1,000
broken links: the name table has sources outside these records. We have not
resolved each reference or reconstructed the full scene hierarchy.

To reproduce both metadata corrections, use the separate research project
and a new export directory:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process hd2.unpacked.exe -noanalysis -scriptPath "$PWD/tools" \
  -preScript ConfigureStackProbe.java \
  -preScript ConfigureChunkReaders.java \
  -postScript ExportFunctions.java "$PWD/analysis/work/typed-loader-export" \
  006cd940 00415c90 00405940 00405890 \
  -max-cpu 2
```

For optional field-by-field output, including local asset names:

```sh
python3 -B tools/inspect_scene2.py extracted/alps1-base/scene2.bin \
  --objects --output analysis/work/alps1-object-fields.json
python3 -B tools/validate_mission_sample.py --archive
```

Without `--objects`, the inspector's existing structural output is unchanged.
The optional decoder preserves field order, repeated fields, and unknown-field
IDs/lengths; it does not invent missing values. It currently supports ASCII
names, exact known payload widths, and finite float values. Parent containers
are inspected, but other unknown payloads remain opaque. Detailed output stays
under ignored `analysis/work/`; do not publish it as repository research.

The evidence validator checks fingerprints, exact counts, quaternion tolerance,
name-link observations, six repaired signatures and four selected direct calls
against local Ghidra exports. It does not prove every semantic interpretation.
It needs the ignored local inputs/exports and therefore cannot run from a fresh
clone alone. `--archive` additionally hashes the full base mission archive.

### Model-resource field and key processing

The `0x2012` branch beginning at `006CDF32` checks the **actual frame type** at
`+0xF8` against `9` before reading a bounded, NUL-terminated resource name.
This is not the same as the record's requested creation kind: a pre-existing
frame can have a type independent of the request. The inspector reports both
the required actual kind and the serialized active creation kind without
claiming the field was applied at runtime.

The model-name string is scanned backward from its length at `006CE167`.
`006CE1D2` compares a character against a dot, `006CE1D9` decrements the scan,
and `006CE242` writes a NUL at the last dot. A new string is then made from
the resulting C-string contents. The resulting rule removes the last dot and
everything after it **anywhere in the string**, not just in a filename's last
path component. Case and whitespace are not changed by this step.

Subsequent code has cached and uncached paths, an engine virtual call and
negative result handling. The failure path references
`# ERROR LOADER : modelfile '%s' not found` at `006CE750`; that exact anchor is
checked against the local function export. Cache version/time diagnostics are
also present. The next section traces the virtual method into the engine and
recovers the candidate-result policy. Full cache behavior and resource search
directories remain unresolved.

All 1,157 ALPS1 occurrences are extensionless basenames and have serialized
active kind `9`; they use 61 distinct names/keys. This is a resource-reference
count, not a count of distinct loaded models or enemies. The existing mission
validator now checks these aggregate observations as well.

The readable [C++ helper](recovered/scene/model_resource_key.cpp) covers only
the name transformation. It uses ordinary strings instead of the game's
copy-on-write representation and rejects embedded NUL in decoded input.
Fourteen C++ cases and 71 cross-language synthetic comparisons cover this
behavior, including leading dots, multiple dots and dots before path separators.
All 61 real names are checked too, but those extensionless names cannot by
themselves establish the suffix-removal behavior.

### Engine frame types and the model-open boundary

The original `LS3DF.dll` is now also imported into the separate research project.
No game binary or baseline project was modified. The driver's
`I3DGetDriver` export at `10068FB0` returns a singleton initialized through
constructor `100151C0`. That constructor installs vtable `1009B640`.
The driver's `+0x7C` slot points to `10019F00`, a type-name query that returns
16 static `FRAME_*` strings for kinds 1 through 16.

[FrameKind](recovered/scene/frame_kind.h) and its name lookup reproduce that
supported table. The inspector retains the numeric creation kinds and adds
their names. Unknown values remain explicit rather than becoming a real type.
The original query's formatted fallback for out-of-range values is not
reconstructed. The [engine evidence map](analysis/engine-model-map.json) and
[validator](tools/validate_engine_model.py) check the DLL fingerprint, PE section
mapping, relevant vtable entries, dispatch targets, and exact type-name strings.

The driver factory slot `+0x54` points to wrapper `10019E10`, which calls
`10018E40`. Its kind-9 branch at `1001909A` constructs a model and installs
vtable `1009BA50` at `100190D6`. The model's `+0x74` slot therefore resolves
the client call at `006CE417` to engine method `100340B0`.
The caller pushes five arguments: model, candidate name, `0x10`, zero, zero;
the engine method returns with `RET 0x14`, confirming stack cleanup for all
five. The last two arguments are kept opaque in our signature rather than
assigning unverified interface types.

[ConfigureEngineModel.java](tools/ConfigureEngineModel.java) defines the
assembly-verified entries when Ghidra misses indirect targets and sets the
model-open signature. It also corrects the preceding reset call at `100340BC`:
model slot `+0x78` resolves to `10034020`, whose `RET 4` establishes one
stack argument. Without that call-site override, Ghidra shifted the following
arguments and incorrectly treated the model pointer as a filename. The corrected
wrapper calls `1005D260` with the resource name, flags, two opaque values,
and model, in that order. These are research-database changes, not executable
patches.

The internal loader selects a name prefix and appends `.4ds` using the string
at `100A707C` before calling `1005D970` at `1005D3AE`. This establishes suffix
addition inside this path, not the complete resource search or archive
precedence. It also has additional global state, callbacks and cleanup that are
not yet reconstructed.

To repeat the metadata correction after importing the original DLL into the
research project, choose a new output directory:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process LS3DF.dll -noanalysis -scriptPath "$PWD/tools" \
  -preScript ConfigureEngineModel.java \
  -postScript ExportFunctions.java "$PWD/analysis/work/engine-open-export" \
  100340b0 1005d260 10019f00 10018e40 10068fb0 100151c0 \
  -max-cpu 2
python3 -B tools/validate_engine_model.py
python3 -B tools/validate_model_load_policy.py
```

### Candidate-result policy

The [readable classifier](recovered/scene/model_load_policy.cpp) reconstructs
one bounded decision inside `006CD940`, not a cache or model loader:

| Result bits | Candidate action | Additional diagnostic |
| --- | --- | --- |
| `0x00000000` through `0x7FFFFFFF` | Accept the result | None here |
| `0x80000002`, `0x8000000A` | Stop with failure | None here |
| `0x80000013` | Try the next candidate, if one remains | `BADFILEVERSION` |
| `0x80000014` | Try the next candidate, if one remains | `BADFILETIME` |
| Other values with the high bit set | Try the next candidate, if one remains | None here |

This preserves the important distinction between retryable negatives and the
two stop codes. It does not invent SDK names for unexplained numbers. The
returned decision exposes diagnostics instead of performing the original
logging; callers must handle both fields. Accepting a candidate result does
not prove that later cache operations or the full mission load succeed.

The [policy evidence map](analysis/model-load-policy.json) records the original
20-byte selector table, five branch targets, signed success check at `006CE563`,
candidate-count/retry branch, and two diagnostic strings. Its
[validator](tools/validate_model_load_policy.py) checks those anchors against the
fingerprinted client using PE section mappings, not generated pseudocode alone.
Seventeen native cases cover success boundaries, both stop codes, diagnostics,
other failures, and neighboring values. Synthetic validator tests also require
mutated evidence and wrong fingerprints to fail explicitly.

Full candidate construction, cache ownership/eviction, refcounts, resource
search directories, and runtime behavior remain open. Neither reconstruction
executes original game code or establishes ABI/binary equivalence.

### First model-file checks

Following `1005D970` reaches the archive open and the model file's first reads.
The [new header reader](recovered/scene/model_file_header.cpp) reconstructs only
this prefix, using these assembly-supported fields:

| Offset | Size | Interpretation |
| --- | ---: | --- |
| `0x00` | 4 | Exact bytes `4DS` followed by NUL |
| `0x04` | 2 | Little-endian version, must equal `0x29` |
| `0x06` | 8 | Opaque metadata, preserved without interpreting it |
| `0x0E` | 2 | Little-endian first-table entry count, at most 6,000 |

The signature comparison covers all four bytes at `1005D9DD`. Its failure
returns `0x8000000A`. The version comparison at `1005DA17` returns
`0x80000013` on mismatch. The count comparison at `1005DA6C` accepts equality
with 6,000; larger counts return `0x80000015`. These are observed result bits,
not invented SDK enum names. They also connect to the client policy: the bad
signature code stops candidate attempts, while the unsupported-version code
logs its version diagnostic and can try another candidate.

Subsequent first-table records start with four-byte flags. Bit `0x04000000`
selects one driver factory/record reader (`+0x5C`, `1002BE30`); the other path
uses `+0x58`, `1002BBE0`. Their serialized read layouts are reconstructed below,
but opaque-field meanings and authoritative material/resource semantics remain
unresolved. The interface deliberately uses `first_table_entry_count`.

The [engine evidence map](analysis/engine-model-map.json) records the read
widths, call sites, comparisons, signature and failure immediates; its validator
now checks these directly against the fingerprinted DLL. The mission validator
also checks the two fingerprinted ALPS1 prefixes: 86 first-table entries in
`scene.4ds` and 13 in `loader.4ds`. These are table entries, not model, soldier
or runtime-object counts.

The source borrows a stream at its current position, consumes at most the
supported prefix, preserves its eight unknown bytes, and leaves table data
unread. Exceptions replace the original negative status returns. Unlike the
original unchecked archive reads, every incomplete prefix is explicitly
rejected. Streams configured to throw I/O exceptions can propagate them.
There is no archive ownership, engine-global state, geometry, table parsing,
reference resolution or complete-file validation. Accepting a header does
**not** establish that a complete model file is valid or loadable.

```sh
make -C recovered test
python3 -B recovered/tests/compare_model_headers.py \
  --reader recovered/build/inspect_model_header \
  --model extracted/alps1-base/scene.4ds \
  --model extracted/alps1-base/loader.4ds
python3 -B tools/validate_engine_model.py
python3 -B tools/validate_mission_sample.py
```

The 39 native cases include the exact 5,999/6,000/6,001 boundary, all 16
truncations, signature/version failures, opaque-byte preservation, stopping
positions, nonseekable/current-position input and stream failures. The Python
comparison matches 30 valid synthetic prefixes and requires both readers to
reject 27 malformed ones, plus optional real-header comparisons.

### First-table records

The [first-table reader](recovered/scene/model_first_table.cpp) now follows the
archive-read subsets of `1002BBE0` and `1002BE30`, plus the parent's flag reads.
It accepts a caller-owned stream positioned after the header and a count bounded
by the same shared 6,000-entry constant. The records are not scene2 chunks.

Both record variants use this serialized layout:

1. Four-byte little-endian flags.
2. Opaque payload: 76 bytes if `flags & 0x00080000`, otherwise 72.
3. If `flags & 0x00080000`, one extra name field.
4. One primary name field.
5. A secondary name field **only** when the primary serialized length is
   nonzero, `flags & 0x40000000`, and `(flags & 0xA1000000) == 0`.
6. For the `0x04000000` variant, two sequence-count bytes and 16 opaque parameter
   bytes, even if the primary name length is zero.

Every name field is a one-byte length followed by exactly that many raw bytes.
The original appends a NUL in its local buffer before engine calls; the file
reader must not scan for a terminator instead of honoring the length. Our
source preserves embedded NUL and non-ASCII bytes and distinguishes an absent
optional field from a present zero-length one. No name validation, normalization
or successful resource lookup is implied.

There is a subtle runtime flag change in the plain reader: computed mode bits
`0x108` can cause `0x40000000` to be ORed into the object's flags. Mode bit
`0x08` requires original flag `0x20000000`, which is already in the secondary
field's exclusion mask. Thus this mutation cannot enable a serialized secondary
name that the raw flags would omit. The parser preserves the raw flags and
does not recreate renderer-dependent state changes; synthetic checks cover this
gate equivalence.

Assembly checks cover the dispatch, both payload sizes, name gates, sequence
read widths, and direct archive-read targets. The sample validator checks all
record counts, flag histograms, optional-field counts and exact end offsets:

| Sample | Records | Plain | Sequence-capable | Secondary fields | Table end |
| --- | ---: | ---: | ---: | ---: | ---: |
| ALPS1 scene model | 86 | 84 | 2 | 4 | 7,869 |
| ALPS1 loading model | 13 | 13 | 0 | 0 | 1,226 |

The next two bytes at those boundaries give frame counts 790 and 5, consistent
with the next read in `1005D970`. This corroborates the boundary; it is not a
traversal or validation of those later frame records. Neither sample uses the
76-byte payload/extra-name branch, which is supported by assembly and synthetic
fixtures instead.

The C++ and Python readers agree on **every preserved byte and record boundary**
for both real tables, not only their aggregate counts. Synthetic comparisons
cover all 64 relevant flag combinations with both empty and nonempty primary
names, arbitrary bytes, 255-byte names, mixed records, the 6,000-record limit
and explicit malformed-input rejection. Native tests also reject every truncated
prefix of the maximal 866-byte record and check nonseekable input, stream
exceptions, current-position reading and leaving later sections untouched.

```sh
python3 -B tools/inspect_model_table.py extracted/alps1-base/scene.4ds \
  --output analysis/work/alps1-scene-model-table.json
python3 -B recovered/tests/compare_model_tables.py \
  --scanner recovered/build/scan_model_first_table \
  --model extracted/alps1-base/scene.4ds \
  --model extracted/alps1-base/loader.4ds
```

The inspector and scanner default to aggregates without asset names. The C++
scanner's `--records` mode is for local byte-level comparisons; keep such
detailed output under ignored analysis storage. This reconstruction does not
make resource requests, allocate engine objects, generate sequence filenames,
animate anything, interpret the opaque payloads or read model geometry.

### Function-based progress

The [README function bar](README.md#progress) now uses a frozen census of
the base client and original engine, excluding Sabre Squadron. Full local
inventories contain 17,294 client and 1,489 engine internal non-thunk entries:
18,783 counted functions in this initial analysis snapshot. Imported/external
entries and forwarding thunks are reported separately. Library/runtime routines
are not yet reliably separated from game code and therefore remain included.
The census is an estimate, not the exact original source-function total.

The [status ledger](analysis/function-status.json) records nine unique
partially reconstructed original functions and zero complete ones. Several
useful source components cover only fragments; the model key and result policy
belong to the same original object-loader function. No full-function credit is
assigned to those pieces or to simply naming a function.

[Counting rules and reproduction commands](analysis/function-progress.md)
describe the read-only inventory export, fingerprints, frozen snapshot,
explicit baseline revisions, and deterministic README regeneration. The
synthetic tracker tests need no game files. This is a source-completeness
metric only, not a percentage of research, difficulty, playability or time left.

### Readable reconstruction versus decompiler output

The [recovered source directory](recovered/README.md) now contains the first
small reconstruction: a portable C++17 read-only chunk reader with descriptive
names and no hardcoded game addresses or registers. Its
[evidence map](analysis/reconstruction-map.json) identifies the supported subsets
of `00405620`, `00415C90`, `00405940`, and `00405890`.

This is not source recovery or a drop-in game function. Standard streams replace
the archive API, exceptions replace Boolean status, and malformed-input safety
checks are explicit deviations from observed code. The 16 C++ tests cover valid
behavior and these deliberate checks. A separate C++ scanner agrees with the
Python inspector on 37 synthetic scenes, 9 rejected malformed scenes, and the
entire immediate-field census of all 4,008 ALPS1 records. Address and
undefined-behavior sanitizer tests also pass.

```sh
make -C recovered test
python3 -B recovered/tests/compare_scene_scan.py \
  --scanner recovered/build/scan_scene_chunks \
  --scene extracted/alps1-base/scene2.bin
```

The C++ scanner does not validate names or interpret gameplay fields. These
checks do not execute the original helpers, so runtime equivalence is not
established. Large loader exports are still not ready: unknown engine calls
and poor type propagation must not be hidden behind confident-looking names.
Generated pseudocode stays local, separate from the readable reconstructions.

The current Python parsers are **format-analysis tools**, not exact replacements
for game functions. In particular, they deliberately reject malformed inputs
more strictly than the observed loader. Every reconstruction should state its
supported behavior and deviations explicitly rather than claim source recovery
or binary equivalence.

### Archive format and prior art

The [native extractor](tools/extract_dta_member.py) supports **ISD0 only**:
XOR-decoded index/member metadata, raw blocks, and LZSS/RLE blocks. It reads the
two 16-bit block-size fields separately and checks the previous-block chain.
It rejects unsupported codecs, invalid lengths/backreferences, truncation, and
incorrect output sizes rather than manufacturing fallback bytes. It accepts
an explicit destination, never uses archive paths as output paths, and refuses
to overwrite existing files.

Format facts and the public base-archive keys were investigated using
[jovan-s/DTAUnpacker at faecc881](https://github.com/jovan-s/DTAUnpacker/tree/faecc88143e74adbf116bbf4324c7702f14d31d7)
and [Richard01CZ/Mafia_DTAExtractor](https://github.com/Richard01CZ/Mafia_DTAExtractor)
(`DTA Extractor/DTA_Extractor.cpp`, blob
`e32e4da3ec8984431fe17b0ad015b517c13ea501`). Their source was not copied into our
tool. The Windows DTAUnpacker attempt failed in this environment; the native
implementation was checked against local block sizes, file headers, and scene
boundaries, **not** against an independent successful extraction.

ISD1, sound-specific compression, checksum verification, and patch precedence
are not implemented. This is a deliberately narrow research tool, not a
replacement for a general archive manager.

To reproduce the sample extraction and inspection (choose new output files):

```sh
mkdir -p extracted/alps1-base analysis/work
python3 -B tools/extract_dta_member.py "Hidden and Dangerous 2/missions.dta" \
  'missions\alps1\scene2.bin' extracted/alps1-base/scene2.bin \
  --key1 0x22BCDA98 --key2 0x7654A3F0
python3 -B tools/inspect_scene2.py extracted/alps1-base/scene2.bin \
  --output analysis/work/alps1-scene2-structure.json
```

Select `missions\alps1\scene.4ds` or `missions\alps1\loader.4ds` and a separate
destination to obtain the other two samples. Names are case-insensitive and
accept either slash style. Actual output files remain ignored.

Run the synthetic checks without any game inputs or extra packages:

```sh
python3 -B -m unittest discover -s tools -p 'test_mission_tools.py' -v
```

## Repository layout and tools

| Location | Purpose | In Git? |
| --- | --- | --- |
| `tools/` | Our analysis and validation scripts | Yes |
| `recovered/` | Readable, tested reconstructions with explicit scope | Yes |
| `recovered/build/` | Native build products | No |
| `analysis/initialization-map.json` | Curated findings and evidence pointers | Yes |
| `analysis/mission-format.json` | First mission-structure observations and fingerprints | Yes |
| `analysis/reconstruction-map.json` | Reconstruction provenance and supported behavior | Yes |
| `analysis/work/` | Local repaired/unpacked binaries and generated exports | No |
| `Hidden and Dangerous 2/` | Locally supplied game installation | No |
| `.tools/`, `ghidra/` | Downloaded tool installations | No |
| `extracted/`, `assets/`, `captures/` | Local assets and runtime captures | No |
| Ghidra projects/databases | Local analysis state | No |

Existing tools:

- [restore_upx_metadata.py](tools/restore_upx_metadata.py): restores the two
  section names and four-byte UPX marker on a new copy. It is specific to the
  observed layout, not a general-purpose unpacker.
- [InspectStartup.java](tools/InspectStartup.java): exports a saved listing and
  attempts decompilation at a selected address.
- [TraceStartup.java](tools/TraceStartup.java): decodes the startup prefix while
  recognizing its inline-string calls; it is not an emulator.
- [ExportFunctions.java](tools/ExportFunctions.java): exports selected functions'
  pseudocode, calls, and references.
- [FindResourceReferences.java](tools/FindResourceReferences.java): searches defined
  strings and exports their saved cross-references.
- [validate_initialization_map.py](tools/validate_initialization_map.py): checks
  the curated map against the local binary and generated exports.
- [extract_dta_member.py](tools/extract_dta_member.py): extracts a selected
  ISD0 member using the narrow supported codecs.
- [inspect_scene2.py](tools/inspect_scene2.py): validates known scene container
  levels and emits counts/offsets without asset contents by default; `--objects`
  additionally decodes supported fields and local names.
- [validate_mission_sample.py](tools/validate_mission_sample.py): verifies the
  curated mission observations against local samples and selected code evidence.
- [ConfigureStackProbe.java](tools/ConfigureStackProbe.java): applies the verified
  stack-probe model to the base-client research database.
- [ConfigureChunkReaders.java](tools/ConfigureChunkReaders.java): applies the
  assembly-supported archive/chunk helper signatures in that research database.
- [test_mission_tools.py](tools/test_mission_tools.py): synthetic archive and
  scene-framing regression tests.

New tools are welcome when they answer a concrete question or make a check
repeatable. Prefer small, read-only tools; use separate copies for modifications.
Generated pseudocode is analysis output, not reconstructed source.

## Running the existing tools

Use Python 3.11 or newer for the Python scripts. The Java scripts run inside Ghidra, not with
`java ScriptName.java`. This workspace has used Ghidra 12.2 with Java 25 and UPX
5.2.1. Download tools from their official distributions and verify checksums.

To reproduce outer-layer unpacking for the base game from the repository root:

```sh
mkdir -p analysis/work
python3 tools/restore_upx_metadata.py \
  "Hidden and Dangerous 2/hd2.exe" analysis/work/hd2.restored.exe
.tools/upx-5.2.1-amd64_linux/upx -t analysis/work/hd2.restored.exe
.tools/upx-5.2.1-amd64_linux/upx -d \
  -o analysis/work/hd2.unpacked.exe analysis/work/hd2.restored.exe
```

These output files must not already exist. Do not overwrite a working game
executable with an analysis copy.

For headless exports, set `GHIDRA_HOME` to the installation directory and
`PROJECT_DIR` to the directory containing your separate `HD2Research.gpr`:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process hd2.unpacked.exe -readOnly -noanalysis \
  -scriptPath "$PWD/tools" \
  -postScript ExportFunctions.java "$PWD/analysis/work/example-export" 006dd550 \
  -max-cpu 2
```

This assumes the program has already been imported and analyzed. The output
directory must be new. A GUI-open project may be locked even for read-only
headless access: use a separate project, never delete its lock. Ghidra 12.2 also
rejected a project location containing `&`; keep the project at a compatible
path outside this workspace if necessary.

To validate the existing map:

```sh
python3 tools/validate_initialization_map.py
```

Validation requires the local binary and all exports referenced by the map.
Those inputs are deliberately not distributed in Git, so a fresh clone alone
cannot run this evidence check. Current local project paths in the map are
machine-specific.

## Evidence and tracker maintenance

After each research milestone:

1. Update the checklist, leaving incomplete work unchecked.
2. Add or refine map entries with the exact binary fingerprint and evidence.
3. Separate observed calls/strings from inferred roles and runtime-verified claims.
4. Run the map validator when changing mapped evidence.
5. Check `git status` before staging; keep binaries, assets, exports, and databases
   local. Extend [.gitignore](.gitignore) when introducing new output locations.

Keep original files unchanged. Do not publish commercial binaries, extracted
assets, memory dumps, or generated game-code exports as part of this repository.
Use a legitimately obtained installation and consider applicable law and license
terms before distributing reverse-engineering results.

**Existing-history warning:** the initial local commit included four generated
game executables. They have been removed from the current Git index while
remaining on disk, but are still present in that commit's history. Ignore rules
do not remove historical content. Resolve this before publishing the repository;
history has not been rewritten automatically.
