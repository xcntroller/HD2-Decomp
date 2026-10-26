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
background, **not the menu artwork**. The latest Proton log confirms that this
run included `--menu-model --frames 300`; this time it was not the old blue-only
mode.
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
extracts its 13 unique textures and the scene initializer's implicit snow
texture, and stages **15 assets** under the ignored runtime working directory.
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
cursor, button binding, audio, logos and the original scene callback remain
outside this preview.

### Plain-green preview investigation

The green matches the scene initializer's default RGB `(0, 0.5, 0.4)` rather
than the bootstrap clear color. This supports reaching scene rendering, but
does not establish that any menu geometry was drawn or textures loaded.
The host now explicitly performs the original camera's post-configuration
update and refreshes the model after reparenting. These changes are statically
supported, **not a confirmed fix for the blank preview**.

To avoid relying on screenshots or manually copied output, the launcher now
relays combined stdout/stderr while saving it to the ignored runtime
`host-output.log`. It includes model-open failures, a bounded tree of loaded
child frames/vtable RVAs, frame counts and cleanup messages. A model with no
attached children is rejected rather than reporting it ready.
The next desktop run should use the same bounded command above; its saved output
will distinguish empty geometry from a populated model that remains invisible.

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
