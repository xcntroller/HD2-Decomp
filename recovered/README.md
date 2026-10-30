# Readable reconstructions

This directory contains our readable reconstructions of small, understood
behaviors. It is **not original developer source**, a dump of Ghidra output,
or a rebuilt game.

The first component is the read-only
[chunk reader](io/chunk_reader.cpp), with its [public interface](io/chunk_reader.h).
Its names and API are ours; its supported behavior is based on the observed
base-game chunk helpers. The [evidence map](../analysis/reconstruction-map.json)
records addresses, scope, and intentional differences.

The second component is the
[model resource key helper](scene/model_resource_key.cpp), with its
[public interface](scene/model_resource_key.h). This is a small string
transformation, not a recovered model loader.

The [frame-kind lookup](scene/frame_kind.h) names the 16 engine types supported
by the DLL's type-name query. The
[model-load policy](scene/model_load_policy.h) classifies candidate results.
Neither component creates frames or loads models.

The [model-file header reader](scene/model_file_header.h) implements only the
initial 16-byte checks performed by the engine's model-file loader.

## Experimental engine-window host

[The engine-window host](host/engine_window.cpp) is independently written
integration glue, **not another recovered original function or the complete original menu**.
Its [56-byte settings layout](host/graphics_settings.h) is statically supported.
The user has now observed the original-engine windows and blue clear-color
surface in a desktop Proton run, followed by shutdown.

It is an optional i386 Windows target:

```sh
make -C recovered engine-window
```

Set `WINDOWS_CXX` to an i386 MinGW-compatible compiler path when needed; see
[the technical build/launch instructions](../RESEARCH.md#graphics-only-host).
Normal `make test` remains native and does not require a Windows compiler.
The host now uses [recovered frame pacing](frontend/frame_clock.h) through
[original Win32 timer calls](host/frame_clock.h), sharing the interval with
mouse input. Pacing is desktop-confirmed.
[Keyboard polling, scan production and modifier/character lookup](input/keyboard.h)
now run through a [keyboard device owner](host/engine_keyboard.h) alongside
engine mouse; keyboard behavior and focus return are desktop-confirmed.
[Client preprocessing](frontend/keyboard_preprocessing.h) and the separate
[text FIFO](input/text_queue.h) are source-only and sanitizer/i386-audited.
[Binding-to-buffer production](input/binding_producer.h) and configured packed
key snapshots are now source-recovered and sanitizer/i386-audited, including
controller-to-front-end Escape routing. [Original binding installation](input/binding_installation.h),
[mode/dirty configuration](input/binding_configuration.h) and
[both compiled default layouts](input/binding_defaults.h) are now also recovered
and audited. The [input-manager owner](input/input_manager.h) now composes
these with borrowed graphics and retained joystick lifetime; its reset/teardown
paths are sanitizer/i386-audited. [Input setup](frontend/input_setup.h),
[language resolution](frontend/startup_language.h) and
[original text loading/parsing](frontend/localized_text.h) now have concrete
[Win32/CRT/rw_data forwarding](host/startup_localization.h), compile/link checked
for i386. [Joystick-list ownership](input/joystick_devices.h) and
[concrete IJoy forwarding](host/joystick_backend.h) now also compile/link and
pass native sanitizer audits through the real input manager. Factory-helper
callers remain unresolved; no new enumeration step is added to startup. This is
source-only; graphics input services/archive setup, application services,
profile persistence and renderer/text consumers remain open. See
[evidence](../analysis/startup-localization.md).
[Archive registration and key exchange](io/startup_archives.h) now use a
[concrete rw_data adapter](host/startup_archives.h) sharing that text/engine
context. Fallback, creation order, masked key protocol and forward cleanup are
sanitizer-audited across 2,048 availability patterns and compile/link with the
actual text loader for i386. Full-wrapper protection and resource-directory
registration remain open; no live resource path changes. See
[scope and key validation](../analysis/startup-archives.md).
With `--menu-confirm --opening-movies`, the host now runs the recovered normal
Logo1/Logo2/Logo3/Intro sequence through the original engine IShow/Bink interface
before constructing the compiled menu. The user desktop-confirmed this path.
[Playback source](frontend/opening_playback.h) reconstructs normal
completion, Escape skip and aspect fitting; [the engine boundary](host/engine_video.h)
owns video release. Subtitles, music coordination, original preferences and full
application startup remain unfinished. See [scope and validation instructions](../analysis/opening-video.md).

[Application controller source](frontend/application_controller.h) additionally
recovers startup/reinitialize gates, input-buffer queries, frame/update/render
order, the single accumulated 100 ms tick, finalizers and normal video-state
replacement. It is source-only and sanitizer/i386-audited. The original
application service bodies, input producer and concrete movie/menu state owners
are required before live integration. See [scope and limitations](../analysis/application-controller.md).

The launcher fingerprints the original DLL and uses its own ignored Proton
prefix/working directory. Launch inside the agent environment fails before host
entry; launch from the user's desktop terminal displayed the test surface.
Exact frame count, exit status and full cleanup remain unconfirmed.

The default test still clears/presents an engine-owned window and exits through
`WM_CLOSE` after a bounded frame count. Optional `--menu-model` mode uses
[scene integration glue](host/menu_scene.h) to open the original menu model,
configure the original orthographic camera and ambient light, and render through
the existing engine. A [separate ABI map](../analysis/menu-scene-map.json)
records its verified interfaces, ownership and one scene-layout field.
The launcher stages a small base-archive asset slice into the ignored runtime
directory; this does not reproduce original archive-overlay behavior.

The initial plain-green preview was followed by a **user-confirmed successful
artwork test** after adding the camera/model updates. This is not yet an
interactive menu. The [cursor renderer](host/menu_cursor.h) now loads the
original texture and reproduces the original textured-quad layout, UV mapping
and resolution scaling; its Win32 mouse tracking is new integration glue.
Its [geometry helper](host/cursor_geometry.h) has native tests, and the
[cursor evidence map](../analysis/menu-cursor-map.json) checks its engine ABI.
The user has confirmed the original cursor appears and tracks the mouse.

The Windows host now writes diagnostics directly to the isolated runtime's
`host-runtime.log`; Proton did not relay the earlier standard-output pipe in
the observed desktop run. The launcher also saves relayed output and appends
the direct host log to `host-output.log` after the run.
In `--menu-model`, text, button actions, screen transitions and logo playback
remain absent. Opt-in `--menu-ui` now adds a bounded static-text/Exit slice:

- [font/label parsers](host/bitmap_font.h) validate the original TBL data and our
  provenance-tracked local label bundle;
- [glyph geometry](host/menu_text_geometry.h) reproduces single-line font sizing,
  character advances, triangle/UV order, the relevant alignment styles and the
  original per-control horizontal text inset;
- [text integration](host/menu_text.h) loads the original ANSI Set2 atlases and
  uses the actual menu visual bounds, not hand-positioned replacement controls;
- [button visuals](host/menu_button_visuals.h) initialize all nine original
  normal/actived pairs, align their local positions as the client does, and
  switch only Exit's artwork on hover;
- only Exit responds to press/release inside its model-derived rectangle.
  The remaining buttons are deliberately disabled; profile/save/date/version
  values are not invented.

This is new host integration, not a reconstructed menu dispatcher. Exit closes
the host directly in `--menu-ui`; `--menu-confirm` separately previews the
original confirmation screen using host-owned input and screen flow.
The new overlay explicitly snapshots/restores changed Direct3D render/stage
states, texture, pixel shader and viewport. Access to the borrowed original
device uses a guarded engine field; IGraph still owns all primitive submission
and its vertex-format cache. A full device state block would incorrectly restore
FVF behind that cache and is intentionally not used.
See the [machine-checkable UI evidence](../analysis/menu-ui-map.json) and
[desktop test instructions](../RESEARCH.md#static-text-and-exit-slice).
**Desktop-verified:** static text appears, Exit closes the host and the
2026-10-27 correction to the original horizontal inset works as expected.
The correction restores
the original horizontal inset (7 pixels per button edge at 800x600); plain static
labels retain their original zero-pixel inset at this resolution. Text rectangles
are separate from the unchanged Exit hover/click rectangle. The latest observed
runtime log records 575 submitted frames, repeated Exit normal/focused
transitions, Exit activation and driver/graphics cleanup. The user also confirmed
the normal/actived visual switching; it does not add the original focus sound/events,
dispatcher or quit-confirmation screen. Artwork-only mode remains unchanged.
See the [session handoff](../HANDOFF.md) for the next step.
The shared state foundation is complete. Main/Exit and the opt-in slot-zero
dialog use owning products; their interactions are desktop-confirmed. Both
construction paths now use the recovered factory helpers, with temporary
definitions and indexed bindings; startup is desktop-confirmed. The shared native
screen/dispatch contract is complete, including source-audited common services.
Concrete application/resource/input backends and screen-specific callback bodies
stay open. See [closure evidence](../analysis/menu-screen-foundation.md) and
[the menu plan](../plan.md).
Neither the original executable nor the user's game prefix is patched.
See [the preview test command](../RESEARCH.md#original-menu-model-preview).

Original confirmation artwork can now be staged explicitly with
[`stage_menu_resources.py --include-confirm`](../tools/stage_menu_resources.py).
This adds the original model and its texture dependencies, retaining provenance
on later default staging. It does not enable a modal in the host yet; the
[confirmation contract](../analysis/menu-ui-map.json) separates the main-menu
screen from the distinct common confirmation API.
For a separate artwork-only diagnostic with the confirmed screen-4 model
position and hierarchy logging, run from a desktop terminal:

```sh
python3 -B tools/run_engine_window.py --confirm-model --frames 0
```

Escape or window close exits; the diagnostic initializes the original button
visual pairs and logs projected control bounds. No confirmation text or
interaction is implemented, and the diagnostic is not the final modal layout.

An experimental interactive Exit confirmation is separately available with
`python3 -B tools/run_engine_window.py --menu-confirm --frames 0`. Yes closes
the host; No and Escape return to the menu. It uses original art and labels,
and the user confirmed the corrected popup composition and interaction on the
desktop. Input and screen flow remain host-owned.

The [button focus callbacks](frontend/button_focus.h) now reconstruct the
original focus-entry and focus-loss operation sequences through a portable
interface. The host calls them for its existing visual transitions; it assumes
the focus-entry gate is clear. The original control object remains to be
reconstructed, so these functions are recorded as partial. The user confirmed
that the previous callback wiring still works on the desktop.

The separately built [control-event routing core](frontend/control_events.cpp)
reconstructs observer selection and tagged-record routing, using portable
callbacks, event records and destination operations. Runtime control bindings,
the original object ABI, intrusive payload ownership and queue are not fully
reconstructed. The host now registers the original Exit/Yes/No activation
records and routes focus events through the filter; these records do not match
focus tags. The user confirmed that filtering works.

The [bounded menu event queue](frontend/menu_event_queue.cpp) now accepts
activation events from these controls in `--menu-confirm`, processes their
original push/deferred/reply/pop/quit sequence in FIFO order, and uses the
original screen-four discard/pop events for Escape. Mouse press detection,
screen visibility and final quit remain host-owned; other event types, local
control dispatch and the original queue representation are unsupported.
Default `--menu-ui` still exits directly. The user confirmed the queue wiring;
the latest host log independently shows No and Yes with expected queue order,
but does not show Escape. It is not the complete original menu dispatcher.

The [pointer click subset](frontend/pointer_click.h) now supplies the original
press/release target-identity comparison for `--menu-confirm`. The host still
selects targets with rectangles, cancels a held press on pointer departure or
focus loss, and does not reconstruct the original double-click or input-update
logic. The user confirmed this integration works; the latest log independently
shows No and Yes with clean teardown, but not Escape or the drag-off result.

The [screen focus subset](frontend/screen_focus.h) now routes changes between
the confirmation controls using the original leave, eligibility and entry
sequence. The user confirmed this integration works; the latest log records No
and Yes and clean teardown, but not Escape. The host assumes the two controls
are eligible and normally invokes the router when sampled hover changes; full
input timing and special focus target 15 remain unsupported.

The [target lookup subset](frontend/screen_target.h) now uses the original
identifier masks and definition-derived confirmation control IDs. The host
still maps rectangle hits to those IDs rather than performing original frame
picking. The user confirmed this integration works.

An opt-in `--probe-confirm-picking` diagnostic compares the original engine
raycast with the host's rectangle result without using the raycast to control
interaction. It assigns the original Yes/No IDs to their two visual frames,
and logs changed comparisons. The user confirmed the probe works; the log
showed both buttons matching and a sample where engine picking returned No
while the rectangle hit was empty. The latest log also shows an engine
Yes/empty rectangle sample. Pointer positions were not logged.

```sh
python3 -B tools/run_engine_window.py --menu-confirm --probe-confirm-picking --frames 0
```

The separately gated `--use-confirm-picking` mode uses the original engine pick
for confirmation hover and press/release target selection. It preserves the
host's existing press cancellation and leaves default modes unchanged. The user
confirmed this mode works; the latest log confirms it was enabled and records
No, Escape and Yes, with 741 frames and clean teardown. The
comparison flag can be used alongside it:

```sh
python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --probe-confirm-picking --frames 0
```

In this opt-in mode, the reconstructed [motion predicate](frontend/pointer_motion.h)
and ordinary screen routing also emit `04005000` when the cursor moves within
an already-focused button. The first such event per button is logged; further
events are still dispatched. Host position sampling does not reconstruct
original relative input, drag gating, double-click timing, or complete screen
and control construction. The latest log recorded motion events for both buttons.

The bounded confirmation path now uses [button click dispatch](frontend/button_input.h):
it emits `04012000` before the existing `04001000` activation event for Exit,
Yes and No. Existing bindings do not select the new click tag. The original
target argument is omitted because these button activation callbacks ignore it.
The latest desktop log shows `04012000` immediately followed by `04001000` for
Exit, No and Yes.

The [screen click subset](frontend/screen_input.h) now routes a confirmation
click through the focused button when sampled focus matches the hit. When it
does not match, the host logs this and retains its previous hit-based selection
to avoid activating a different button with stale sampled focus. The user
confirmed the routing works; the latest log shows it for No and Yes without a
logged fallback. This fallback, other screens and the full original input
update order remain host scaffolding.

The comparison flag also enables an observational input probe using
[button selectors and transitions](frontend/pointer_buttons.h) and
[click timing](frontend/pointer_click_timing.h). Selector zero means *any*
button; selector three matches either of the first two button bits. The probe
logs Win32 button snapshots, picked IDs, capture, click and double-click flags.
It uses message timestamps in milliseconds as an approximation, resets its
shadow state when the confirmation closes or focus is lost, and never dispatches
the flags. Original polling, timer units and input-update ordering remain
unverified. The user reported it works; the latest desktop log shows matching
left-button press/release captures and `click=1 double=0` for No and Yes.
It contains no right-button or `double=1` samples.
The established `--menu-ui` behavior remains unchanged.

## Chunk reader

The reader:

- measures the input length and rewinds to its beginning;
- reads six-byte little-endian headers: a two-byte ID and a four-byte total
  length, including the header;
- keeps a stack of active chunk ends;
- bounds payload reads by the current chunk;
- skips unread payload when leaving a chunk;
- treats file length as the boundary outside any chunk.

It accepts a caller-owned, seekable `std::istream`. Keep that stream alive and
do not move its cursor independently while using the reader. Destroying the
reader does not close the stream. `enter_chunk()` returns `std::nullopt` at the
current boundary; `leave_chunk()` at depth zero seeks to the file end, matching
the observed read-mode helper's depth-zero behavior.

### Supported subset and differences

This is a portable C++17 implementation of the **read-only, valid-input subset**,
not the game's original binary layout or archive interface:

- `std::istream` replaces `dtaRead`/`dtaSeek`. DTA opening, archive precedence,
  ownership/closing, write mode, and chunk-size backpatching are not reconstructed.
- The original helpers generally return Boolean status. This API throws
  `ChunkReadError` for validation and ordinary stream failures; a stream configured
  to throw I/O exceptions may also propagate its own exception.
- Truncated headers, lengths below six, chunks crossing their parent/file end,
  failed seeks, and reads crossing the file boundary are rejected explicitly.
  The observed game code does not consistently enforce these checks.
- Nesting is capped at 256, consistent with the inferred end-stack capacity.
  This is an explicit safety guard, not evidence of an original runtime check.
- Files above signed 32-bit offset range are unsupported. Arithmetic uses wider
  internal offsets instead of reproducing potential x86 overflow.
- No gameplay meaning is attached to a chunk ID. No world transforms, object
  creation, or runtime scene state are simulated.

After an I/O failure, discard the reader rather than assume cursor rollback.
Malformed headers may have been consumed before their lengths were rejected.
The reader never silently substitutes data or reports a failed seek as success.

## Model resource keys

The object loader's `0x2012` handler reads a model resource name, and a later
step scans backward for the last dot. If present, it terminates the string
there. The recovered helper reproduces that **string-content transformation**:

| Input | Key |
| --- | --- |
| `crate` | `crate` |
| `crate.4ds` | `crate` |
| `crate.variant.4ds` | `crate.variant` |
| `.hidden` | empty |
| `folder.name/object` | `folder` |

The last example is intentional: the observed scan is not aware of directory
components. Do not replace it with filesystem extension handling. Case,
whitespace, and path separators are otherwise preserved.

The input is already-decoded name content without its serialized NUL terminator.
Embedded NUL bytes are rejected explicitly. `std::string` replaces the game's
copy-on-write storage and bookkeeping. Resource lookup directories, extension
addition, actual frame creation, cache behavior, and successful loading remain
outside this helper's scope.

## Frame kinds and model-load decisions

The frame-kind enum uses the engine's numeric kinds and `FRAME_*` names.
Unknown numeric kinds return an empty optional; the original engine's formatted
fallback is deliberately not recreated. The scene inspector retains the
serialized numbers alongside any recognized labels.

`classify_model_load_result()` accepts the raw 32-bit result bits. Nonnegative
results select `accept`; `0x80000002` and `0x8000000A` select `stop_failure`.
Other negative results select `try_next_candidate`, with separate diagnostics
for `0x80000013` (bad file version) and `0x80000014` (bad file time).
The caller must consume both action and diagnostic. Trying another candidate
is conditional on one remaining; accepting a result is not a guarantee that
the complete load succeeds.

This pure classifier reproduces inline branch decisions, not the retry loop,
logging, file search, engine method, or cache. Unsigned result bits avoid a
platform-dependent conversion to signed integers. Evidence and original
addresses live in the [engine map](../analysis/engine-model-map.json) and
[policy map](../analysis/model-load-policy.json), not in reconstructed logic.

## Model-file headers

`read_model_file_header()` reads a 16-byte prefix from the stream's current
position: the exact `4DS` plus NUL signature, little-endian version `0x29`,
eight opaque metadata bytes, and a little-endian first-table entry count.
The count limit is **inclusive**: 6,000 is accepted; 6,001 is rejected.
The first table's complete meaning is not yet established, so its neutral name
does not claim that its entries are soldiers, models, or materials.

The reader leaves subsequent data unread and supports nonseekable input.
`ModelHeaderError` exposes a typed reason for incomplete reads, invalid signature,
unsupported version or excessive count. A stream configured to throw I/O
exceptions may propagate those exceptions. After failure, do not assume the
input cursor was rolled back.

Unlike the original archive calls, short reads are explicitly rejected. The
source does not open/close archives, modify engine globals, interpret the eight
unknown bytes or construct objects. **An accepted prefix does not validate the
complete model file.** It is another partial reconstruction, not a complete
original function for the progress bar.

## First model-file table

The [first-table reader](scene/model_first_table.h) reads a specified number of
records from the stream's current position. Use the preceding header's count;
both components share the 6,000-entry limit. The reader leaves the following
frame section untouched.

It preserves serialized flags, 72- or 76-byte opaque payloads, raw length-prefixed
name fields, and optional sequence counts/parameters. Optional names distinguish
absence from an encoded zero-length name. Embedded NUL and non-ASCII bytes are
preserved; the original serialized length, not C-string contents, controls
whether a secondary field exists.

This is only the file-reading subset of two engine routines. There are no engine
allocations, renderer options, resource requests, generated sequence filenames,
playback or opaque-field interpretation. Runtime object-flag changes are not
recreated; the evidence explains why they do not change the serialized read
layout. `ModelTableReadError` reports incomplete fields and excessive counts.
Configured stream exceptions may propagate. No cursor rollback is promised.

## Build and test

From the repository root, with a C++17 compiler, GNU Make, and Python 3.11+:

```sh
make -C recovered test
```

This builds with strict warnings and runs:

- 16 C++ tests for headers, payloads, nesting, skipping, bounds, short reads,
  failed seeks, stream positioning, and the signed offset limit;
- 37 deterministic synthetic scene comparisons against the independent Python
  inspector;
- 9 malformed-scene cases that both scanners must reject explicitly;
- 14 C++ model-key cases, including an embedded-NUL rejection;
- 71 deterministic model-key comparisons against Python;
- frame-kind checks for all 16 supported values and unknown values;
- 17 model-load-result cases covering success, retry, stop, and diagnostics;
- 39 model-header cases, including exact count boundaries, all prefix truncations,
  opaque-byte preservation, stopping positions and stream behavior;
- 30 synthetic model-header comparisons and 27 malformed headers both readers
  must reject;
- 1,002 native first-table cases, including all 128 flag/primary-presence
  combinations and every truncation of a maximal record;
- four first-table comparison fixtures (including mixed records and 6,000
  entries), plus 107 malformed tables both readers must reject.

No game files are needed for those tests. Build products go into ignored
`recovered/build/`.

To cross-check an already extracted local mission:

```sh
python3 -B recovered/tests/compare_scene_scan.py \
  --scanner recovered/build/scan_scene_chunks \
  --scene extracted/alps1-base/scene2.bin
```

This compares file size, top-level/container/record counts, every immediate
record-field count, and unknown record IDs. For the recorded ALPS1 sample, the
reader matches 15 top-level chunks and all 4,008 records. It does not decode
payload semantics or check names; use the existing Python inspector for that.

To compare model keys used by the real sample:

```sh
python3 -B recovered/tests/compare_model_resource_keys.py \
  --normalizer recovered/build/model_resource_key \
  --scene extracted/alps1-base/scene2.bin
```

The ALPS1 sample has 1,157 model-resource references using 61 distinct names.
All of those names are extensionless, so the suffix-removal edge cases are
established by assembly inspection and synthetic tests rather than that sample.
The comparison driver does not print the real asset names.

To compare the already extracted local model-file prefixes:

```sh
python3 -B recovered/tests/compare_model_headers.py \
  --reader recovered/build/inspect_model_header \
  --model extracted/alps1-base/scene.4ds \
  --model extracted/alps1-base/loader.4ds
```

Their first-table counts are 86 and 13 respectively. The mission evidence
validator checks the exact prefixes against the fingerprinted samples.

To compare all first-table fields and record boundaries in those same files:

```sh
python3 -B recovered/tests/compare_model_tables.py \
  --scanner recovered/build/scan_model_first_table \
  --model extracted/alps1-base/scene.4ds \
  --model extracted/alps1-base/loader.4ds
```

The aggregate scanners print no asset names. Detailed `--records` output from
the native scanner contains local raw fields; do not publish it as source.

An address/undefined-behavior sanitizer build can stay in ignored analysis output:

```sh
make -C recovered BUILD_DIR=../analysis/work/recovered-sanitized \
  CXXFLAGS="-std=c++17 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer" \
  test
```

The [scene scanner](tools/scan_scene_chunks.cpp) is a test driver for the reader,
not recovered mission-loader logic. It follows only the established root,
object-container, and object-record levels and emits aggregate JSON without
asset contents. Additional unknown payloads are skipped, not guessed.

## Evidence standard

Each further reconstruction should identify its input binary and static
evidence, use descriptive names, state the supported behavior and deviations,
and include tests. Original addresses belong in the evidence documentation,
not in normal reconstructed logic.

These tests compare reconstructions and file observations, **not execution of
the original game helpers**. Runtime equivalence, ABI compatibility, binary
matching, and a complete source reconstruction have not been established.
