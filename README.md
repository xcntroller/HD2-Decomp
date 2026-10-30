# Understanding Hidden & Dangerous 2

A personal project to discover how Hidden & Dangerous 2 works, from loading a
mission to controlling a squad, and gradually document its LS3D engine.

We're starting with the base game's Steam release and its original LS3DF.dll
engine, on Linux through Proton. The function tracker below covers only these
two binaries; the expansion is outside its scope.

Menu delivery checklist: [plan.md](plan.md). It tracks the entire base-game
menu separately from the original-function counts below.

Movie/menu keyboard device input and focus return are desktop-confirmed. The
[keyboard preprocessing and text-input slice](analysis/keyboard-preprocessing.md)
is recovered in source and audited. The original
[binding-to-buffer producer](analysis/binding-producer.md) now covers keyboard,
mouse and joystick in source. [Binding installation and both original default
layouts](analysis/binding-installation.md) and
[input-manager ownership/graphics attachment](analysis/input-manager.md) are
also recovered and audited. [Original input setup and startup localization](analysis/startup-localization.md)
now include registry language selection and real text loading/parsing. Concrete
graphics input services and archive setup, application startup, profile persistence
and text rendering consumers still precede integration. [Joystick list ownership
and concrete IJoy forwarding](analysis/joystick-devices.md) are now recovered and
audited; its creation helper has no discovered caller, so normal empty-list
startup does not gain an invented enumeration step.

[Archive registration and key exchange](analysis/startup-archives.md) now have
readable source and checked DLL calls. Creation/fallback order, cleanup and
2,048 combinations of available files pass audits; keys match installed assets.
The preceding protection operation and resource-directory setup remain open,
so this does not yet enable the complete original startup path.

The shared control-state foundation entry is complete: initialization, base
messages, observers, event ownership and eligibility are recovered and audited.
See [validation and scope](analysis/menu-foundation-state.md). Widget visuals,
settings services and the other foundation entries are still in progress.

The shared native screen construction, ownership, stack and dispatch entry is
also complete. Main/Exit and the opt-in slot-zero dialog use the recovered
factory paths and are desktop-confirmed. Unused common-service branches have
native sanitizer/i386 coverage; concrete application/resource/input backend
bodies and individual screen flows remain open. See [scope and closure evidence](analysis/menu-screen-foundation.md)
and [modal evidence](analysis/menu-runtime-confirmation.md).

## Where we are

**We've got past the first major obstacle: accessing the game's code.**
We've unpacked copies of the game executables and started identifying the parts
responsible for startup, the main loop, and loading missions.
We can also extract selected mission files locally and read the basic structure
of one real mission.
We can now identify how some of its objects are named, positioned, rotated,
scaled, and linked to parent objects.
The first small, readable C++ reconstruction is also in place: the code for
reading nested sections of a file.
We've also identified model-resource references and reconstructed the loader's
small model-name processing step.
We can now name the engine's object types and explain when model loading tries
another file, stops, or accepts a result.
We've also reconstructed the first checks the engine makes when opening a model
file, including its supported version and an initial entry-count limit.
We can now read the records immediately after that header, including their
conditional name fields and extra sequence data, while preserving unknown bytes.

This is still early research, not a rebuilt or playable version of the game.
Most findings come from examining files. The bounded menu artwork, cursor, text and Exit
milestones have also been checked in desktop runs of our rebuilt host.

**Our next goal is the menu, not a complete mission.** We'll aim for our own
small rebuilt program showing the original menu, initially using the existing
engine and game assets through Proton. This is not a replacement menu drawn to
look similar. **Our own graphics-only program now opens an original-engine
window through Proton:** the desktop test displayed the blue test surface and
then shut down. A separate **original menu-artwork preview** is now built and
now displays the real menu artwork in a desktop test, after correcting the
camera/model updates. It stages only the needed local assets and uses the
original engine and menu camera. **The original cursor and mouse tracking are
also desktop-verified.** A new, opt-in `--menu-ui` build adds static English text
using the original bitmap fonts and an Exit button that closes this host.
Desktop tests confirm visible text, working Exit and the corrected original
horizontal text inset. The host now also initializes the original button visual
pairs and switches Exit's artwork on hover; the user has confirmed this behavior
on the desktop too. The opt-in `--menu-confirm` experiment also shows the original
confirmation artwork and labels; the user confirmed Yes, No and the corrected
layering, the bounded original activation and queue flow, press/release identity,
confirmation focus routing and identifier lookup. An opt-in engine-picking
comparison probe, the separate engine-picking interaction with pointer-motion
events, original button click-event ordering and focused-screen click routing
have also been desktop-checked. An opt-in shadow probe now observes original
button-mask and double-click bookkeeping without changing interaction; its
host-supplied timing remains approximate. The original clock is now traced to a
bounded millisecond frame delta, and the shadow uses reconstructed input gates
and screen callback ordering. Engine mouse polling slots are now
identified; cursor accumulation/clamping and six screen input callbacks now
have portable reconstructions in the shadow. The host still supplies approximate
snapshots. A new `--use-engine-mouse` option rebuilds the engine mouse polling
branch and uses device button snapshots with reconstructed confirmation
dispatch; the user has confirmed it works, including right-click activation
consistent with the recovered masks. Wheel routing now follows the recovered
screen/button path; the user confirmed its expected visible no-op. The original
hidden `CURZOR` scene model and position/update calls are desktop-confirmed.
Engine-input mode now also routes main-menu Exit through the rebuilt dispatcher
and original model picking; the user has confirmed that extension works. Shared input state across main-menu/confirmation transitions
is also desktop-confirmed, including held-button cancellation. Enter/Space activation for the focused Exit/Yes/No
control in engine-input mode is desktop-confirmed; keypad Enter remains
untested on hardware. Original previous/next focus traversal now has portable
reconstructions; live key bindings remain unidentified, so it is not connected
to the host. Original `sound_click` frame/callback and sound-driver startup/
shutdown are desktop-confirmed. Original `button.wav` decoding and engine-buffer loading for accepted pointer
clicks are now desktop-confirmed, including audible playback. Escape now follows original queued back requests: open
quit confirmation from the main menu, cancel it from confirmation. This behavior
is desktop-confirmed. Driver initialization failure retains
silent fallback. Screen effects and much of input remain
host scaffolding, not the complete
original dispatcher. Pixel-perfect
fidelity, other screens and profile/save data are not implemented.

The desktop-tested continuation routes Exit/Yes/No tagged definition events
through reconstructed initialization-list sequencing and reads original control
state defaults for eligibility. It builds and passes focused native audits;
desktop behavior is user-confirmed. The control factory's initialization/insertion
tail also has an audited source reconstruction; full factory and derived text
initialization remain incomplete. See [plan.md](plan.md) for the remaining work.

The desktop-confirmed larger slice replaces the separate main-menu and
confirmation routing adapters with shared definition-backed controls/screens.
Construction, ownership, target lookup, focus, keys, pointer and wheel routing
now use the same implementation for both screens. The original 17/four child
slots and button activation bindings are centralized; Exit/Yes/No remain the
only enabled actions. Build, focused audits and desktop testing pass.
The reusable-adapter item is checked in [plan.md](plan.md).

The desktop-confirmed slice integrates shared control-state events, observer
registration/removal and local event delivery. Hidden OK now runs its original
hide definition through the shared handler. Derived text/button message routing
also has audited source. Attached text/caption integration is described below.
Build, native/sanitizer audits, existing checks and user desktop testing pass.

The desktop-confirmed text slice connects definition caption/style events to attached text
records and renders from that state. It includes localized set/append, indexed
replacement/removal/clear, disabled mirroring and original empty label 2002.
Build, native/sanitizer audits and existing checks pass; **desktop validation passed** (506 frames, clean teardown). General paragraph layout, original
font API and dynamic/version callbacks remain incomplete.

The registered screen stack and visibility/focus lifecycle are desktop-confirmed.
The latest desktop-confirmed slice adds targeted delivery to all 21 current screen children,
retained-copy messages and native screen observer bindings. It supports updates
to the menu underneath a modal independently of input availability. Build,
existing checks, sanitized audits and user desktop testing pass (507 frames and
clean teardown). The slice is included in the committed session checkpoint. Original dynamic callback lists, nested factories and
additional menu flows remain incomplete. See the [screen inventory](analysis/menu-screens.md).

The latest desktop-confirmed slice routes all 21 existing leaf controls through a
shared definition factory. Nested kind-five container construction, identifiers
and event dispatch are reconstructed; other widget constructors and actual
options integration remain unfinished. The
build and existing checks pass; the desktop run ended cleanly after 1003 frames.

An opt-in group diagnostic now connects recovered container state and routing to
two linked original-engine dummy frames. Its i386 build and existing checks pass;
desktop validation passed (425 frames, clean teardown). The [options inventory](analysis/menu-options.md)
records the further widgets and callbacks required before options can work.

The latest desktop-confirmed slice adds recursively owned controls and nested
focus/key/pointer forwarding, with mixed text/button trees checked under
sanitizers. Leaf visual lookup is scoped to its bound frame. The updated engine
diagnostic constructs a three-level tree of five groups from definitions and
checks duplicate names in separate branches; desktop validation passed (372 frames, clean teardown).

The current source-only slice reconstructs options slider/selector/toggle state,
message and input behavior, plus four settings-observer routes. Connected native
audits pass under sanitizers and compile for i386. Engine widget visuals and real
preference services are still required before the options screen can be enabled.

## Progress

Completed steps mean we've reached a useful milestone, not that we understand
every detail of that area.

<!-- FUNCTION-PROGRESS:START -->
### Original-function reconstruction (base game only)

`[------------------------------] 0.00% complete`

**0 complete / 18,783 identified functions** (frozen, estimated baseline); **149 unique functions partially reconstructed**.

| Program | Counted functions | Complete | Partial |
| --- | ---: | ---: | ---: |
| Base game client | 17,294 | 0 | 144 |
| Original LS3DF.dll | 1,489 | 0 | 5 |

This covers the **base game and original LS3D engine only**, not Sabre
Squadron. The total is an estimate from Ghidra, including library code we
haven't separated yet. Imported routines and small forwarding stubs are
counted separately, outside this bar.

The bar fills only when an entire original function is reconstructed.
Our existing source recovers useful **parts** of functions, so it appears
under Partial rather than Complete. Discoveries and research milestones
are listed below; **0% complete does not mean no progress**.

Ghidra can miss functions or get their boundaries wrong, so the baseline
may be deliberately revised. This percentage does not measure understanding,
difficulty, playability, or time remaining.

[Status ledger](analysis/function-status.json) |
[Frozen counts](analysis/function-baseline.json) |
[Counting rules and refresh commands](analysis/function-progress.md)
<!-- FUNCTION-PROGRESS:END -->

### Done

- [x] Inspect the game files and establish a consistent starting version.
- [x] Unpack copies of the game executables without changing the originals.
- [x] Set up tools to examine the code.
- [x] Build an initial map of how the game starts and runs.
- [x] Locate the code involved in loading missions.
- [x] Extract sample files for one mission and match their structure to the loader.
- [x] Decode the first object-placement fields and distinguish lookup from creation.
- [x] Reconstruct and test a small file-reading component with readable source.
- [x] Identify model-resource references and reconstruct their name-processing rule.
- [x] Recover engine object-type names and the model loader's retry/stop decisions.
- [x] Reconstruct and test the first model-file header checks.
- [x] Read and cross-check the first table in both sample model files.
- [x] Trace startup into the original menu and locate its screen/control definitions.
- [x] Save our findings and add repeatable checks for the evidence.

### Next: get our rebuilt program to the menu

- [x] Find how startup reaches the menu and identify its main dependencies.
- [x] Build our own small Windows program for testing the original graphics interface.
- [x] Open an engine window from our own small program through Proton.
- [x] Build a separate test for the original menu artwork and its required assets.
- [x] Verify that the original menu artwork appears in a desktop run.
- [x] Add the original cursor texture and its engine drawing path.
- [x] Verify cursor appearance and mouse tracking in a desktop run.
- [x] Implement an opt-in static-text slice using original labels and bitmap fonts.
- [x] Add a host-owned Exit adapter using the original button's model bounds.
- [x] Verify visible text and working Exit in a desktop run.
- [x] Load the original menu artwork, static text and cursor.
- [x] Connect bounded host input and make Exit close the host cleanly.
- [x] Correct text overflowing the button borders against original layout evidence.
- [x] Restore original normal/active model-visual selection for Exit hover.
- [x] Verify the new button visual states in a desktop run.
- [x] Trace main-menu confirmation events and add opt-in original-art staging.
- [ ] Integrate the original quit-confirmation screen.
- [ ] Compare our running menu with the original game's behavior.

Deeper ALPS1 work is paused. Its findings remain useful for shared file readers
and model loading, but we don't need to decode an entire mission before working
toward something visible. Other menu screens and gameplay come after this first
interactive milestone.

The [graphics bootstrap instructions](RESEARCH.md#graphics-only-host) describe
the first successful desktop-terminal window test.
The [menu preview instructions](RESEARCH.md#original-menu-model-preview) cover
the artwork/cursor test. The [static text and Exit test](RESEARCH.md#static-text-and-exit-slice)
documents the successful bounded interactive milestone and its layout limitation.
An opt-in `--opening-movies` addition to `--menu-confirm` now runs the normal
opening sequence through the engine's original IShow/Bink interface before
constructing the compiled menu. The user desktop-confirmed this path; the log
records all four movie opens and clean teardown. Movie copies live in the isolated runtime directory;
original files remain untouched. Full startup/controller recovery, subtitles,
music coordination and original settings remain open;
see [the launch plan](plan.md#0-rebuild-the-original-launch-path-to-the-menu--current-delivery-priority)
and [video evidence](analysis/opening-video.md).

Shared application-controller startup gates, frame/input/render order, tick
handling and normal state replacement are now recovered as source and audited
under sanitizers and Windows i386. They still require original application
services and concrete state owners before live integration; see
[controller evidence](analysis/application-controller.md).

### Longer term

- [ ] Understand how the game and engine work together.
- [ ] Check more of the original game's behavior against our reconstructions.
- [ ] Explore individual systems, such as squad orders, AI, and animation.
- [ ] Create useful tools and reconstruct small, well-understood parts.

A complete source reconstruction is a possible long-term ambition, not a
promise. The function bar is a narrow, reproducible count, not a percentage of
the whole project or a forecast of when a reconstruction will be finished.

## How we work

We examine small parts of the game, record what the evidence tells us, and test
our explanations. Tools can turn machine instructions into approximate readable
code, but the result still needs interpretation.

We keep the original files untouched and distinguish between what we've
observed, what we think it means, and what we've confirmed in the running game.
We also build small tools when they make the work easier or repeatable.

This tracker will be updated as milestones are completed or our understanding
changes.

## Looking deeper

- [Session handoff](HANDOFF.md): current working state, known issue and restart commands.
- [Technical research guide](RESEARCH.md): detailed findings, tool instructions,
  and remaining questions.
- [Research map](analysis/initialization-map.json): structured evidence behind
  the initial startup and mission-loading map.
- [Menu findings](analysis/frontend-map.json): the route into the original menu,
  its known dependencies, and what still prevents a running rebuild.
- [Mission findings](analysis/mission-format.json): the first checked file
  structures, sample fingerprints, and what remains unknown.
- [Tools](tools/): scripts created for this project.
- [Readable reconstructions](recovered/): small, tested implementations of
  understood behavior, starting with the chunk reader.

## Keeping game files local

This repository is for our tools and research, not for distributing the game.
Commercial binaries, extracted assets, generated code exports, and local
analysis databases stay out of Git. See [.gitignore](.gitignore).

**Before publishing:** four generated game executables were included in the
initial local commit. They're no longer tracked in the current index, but remain
in that commit's history. That history still needs to be addressed.
