# Session handoff - 2026-10-29

## Resume here

Input and wheel work is committed through `a295649` on `master`. The cursor
continuation is checkpointed in the local commit titled
`Bind original CURZOR model and input frame updates`.
Preserve later worktree changes. Do not push without explicit user approval.

- Latest continuation stages original `MODELS\kurzor_menu.4ds` only when
  `--use-engine-mouse` is requested (manifest ownership persists later).
  The model is 330 bytes, SHA-256
  `d29a19d542f7155e14b82e592a642caf974b068926d6bd30cae59e098fd409f2`,
  with existing `X_KURSOR.TGA`; no new texture. Real-archive staging audit
  upgrades 21 to 22 assets, preserves prior provenance, verifies all hashes,
  retains identical files/manifest on default restaging and rejects a modified
  cursor model before writes. All audit files were temporary.
- Engine-input host now loads model kind nine, names its root `CURZOR`, links
  primary sector and hides it. Original scene +7C lookup with flags FFFF must
  return that owned model; +E8 name and setter/update ABIs are checked.
  `frontend/cursor_frame.h` reconstructs the 0066A220 tail projection and
  +08 position / +24 world-update order. The confirmation adapter calls it
  on host-seeded arm/reentry and after processed input, before screen callbacks.
  Visible bitmap cursor, hit/pick ray and other modes are unchanged. No new
  cursor binding/update desktop test yet. Look for `Original CURZOR model bound`
  and `Original CURZOR position/update` logs using the same engine-input command.
- Latest continuation reconstructs wheel getter/predicate, int64 truncation
  and low-DWORD argument passing, focused-screen +4C forwarding, button
  `04011000` with zero argument, and post-callback current-screen repick/+40.
  `--use-engine-mouse` now consumes polled Z. Drag early return still skips
  wheel; double-click activation remains disabled. Repick uses the same frame
  cursor point and the actual confirmation model. User confirmed scrolling
  produces no visible effect. Latest log records Yes target 10200000 wheel
  -120/-240/+120, result 7, event 04011000 with zero argument, 432 frames and
  clean teardown. No No/outside wheel samples are established by this log. Three new
  partial entries bring totals to 38; no complete functions claimed.
- Current goal is a functional, decompiled/rebuilt menu. User authorized
  prioritizing the engine boundary that helps that goal. New next desktop step:

  ```sh
  python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --frames 0
  ```

  User desktop-confirmed it works well (2026-10-29), including right-click
  activation. This matches recovered mask-three screen transitions and
  any-button click tracking; no left-only restriction was added. Direct log
  records No three times, Escape, Yes, one poll failure and recovery, 807
  frames and clean teardown. It lacks raw button masks, so right-button
  identity is user-reported. Focus-loss/drag-off outcomes remain unverified. Omit it to restore established message-driven
  confirmation. No agent-side Proton launch or new unit tests.
- `recovered/input/mouse_poll.h` reconstructs LS3DF `1006F8A0` device
  read/Acquire/retry/clear, capability-based wheel suppression and swapped
  buttons, plus `1006FB30` first-five-button mask packing. Original data format
  is 20 bytes (signed XYZ, eight button bytes). `host/engine_mouse.h` uses
  original +DC setup with zero for foreground nonexclusive flags six, borrows
  the original device for rebuilt polling and calls +E4 for engine-owned close.
  Portable snapshot storage replaces engine globals. Unused nonexclusive Win32
  metric queries and exclusive-mode accumulation are excluded; original device
  creation/configuration/teardown remain in LS3DF.dll.
- New functional flag feeds polled engine buttons into reconstructed
  confirmation pointer update/dispatch/screen callbacks once per frame. Main
  Exit remains message-driven. Position/motion and hover remain host supplied;
  one sampled point supplies hover and click picking. Frame delta uses bounded
  QPC intervals, without original wait/scaling. Button press/release/drag event
  emissions are reconstructed and route through the existing tagged bindings.
  Queue pumps after input. Live double-click dispatch stays disabled; wheel is now reconstructed as above.
  Departure cancellation is retained; focus loss/outside/poll failure reset
  frame input; reentry first requires all buttons up. No activation from stale
  focus fallback. Original captured screen-control objects/helper are absent.
- Frozen inventory lacks the two engine entries and the tiny pointer-event
  button callbacks, so they receive no ledger credit; totals are now 38 partial (three wheel functions added).
- Traced the original timer through cached exports and section-mapped assembly:
  `006DC950 -> 006DCAE0 -> 006C20F0 -> state +14 / 0066BEA0 -> 0066B990 ->
  0066A220`. Source is QueryPerformanceCounter/Frequency converted to integer
  microseconds, a wrapping unsigned low-word difference divided by 1000, then
  bounded to 10..100 ms by the main-loop caller. An optional state time scale
  and remainder carry precede dispatch; its provider semantics remain unresolved.
  The 400 threshold is accumulated update milliseconds, potentially scaled.
- Original queue processing precedes input. Picking precedes input update and
  relative cursor integration; bound screen updates follow input. Do not swap
  this ordering based on the host's current event loop.
- `recovered/frontend/pointer_update.h` now includes idle bookkeeping and
  `pointer_cursor.h` accumulation/clamping. XY sums round to float before
  clipping; reported deltas correct overflow using the original subtraction
  order and long-double intermediates. Wheel accumulates without clipping.
  Original FPU control-word behavior remains absent. Engine frame updates are
  now connected only in the confirmation adapter as described above.
  The observational shadow seeds cursor/ranges from the first available host
  point per entry/reentry and uses host position differences; message timing
  still approximates frame polling. Dispatch reads corrected XY motion.
  Suppression and inhibition stay clear and are not desktop-tested.
- Six screen helpers (`00656CA0/CD0/D10/D40/D70/E00`) in
  `recovered/frontend/screen_input.h` reconstruct press/release/drag/end/begin/
  double-click routing. Invalid focus returns zero for the first five, seven
  for double-click. Release additionally requires hit index to match focus.
  All are partial: portable accessors/callbacks replace original ABI/objects.
  The shadow now maintains its own simulated Yes/No focus and routes through
  these helpers; enter/leave succeed and child callbacks return seven.
  Logs include focus, result, cursor and corrected motion. No live events or
  activation run, and new logging has not been desktop-verified.
- New `recovered/frontend/pointer_dispatch.h` reconstructs
  `0066B990` sequencing including wheel: gate, pick/update, target-change gate, alternate +2D
  branch, mask-3 press/release, captured-control veto, double-before-single
  click, drag begin/end/move and early return before wheel. Context must
  resolve the current screen for every callback. Wheel handling/repick order
  is now reconstructed; accepted-click helper remains delegated. `recovered/host/pointer_dispatch_probe.h`
  runs portable screen routing against simulated controls; it never invokes
  actual screen/control objects or activation. Capture state is absent; wheel is not sampled.
  Consecutive identical focus/drag logs are suppressed. New logging has not
  been desktop-tested. Interactive behavior is unchanged.
- Original mouse poll graphics +E8 is `1006F8A0`: device +24 reads 20 bytes
  at `100D1EC8`, acquisition and one retry follow failure, repeated failure
  clears five DWORDs. Graphics +EC/+F0/+F4 return the stored signed XYZ via
  `1006FA10/20/30`. Opt-in setup/shutdown and rebuilt device polling are now
  added as described above.
  Successful-path cursor warping, absolute accumulation and device/focus
  setup remain unreconstructed.
- Strict i386 host build, all existing native checks, 12 resource checks, 17 launcher checks,
  9 frontend checks, 27 accounting checks, 252 UI instruction anchors, 20 engine vtable slots, frontend evidence and
  frozen accounting pass. No new unit tests or agent-side Proton launches.
  Function totals are **0 complete / 18,783; 38 partial**.

Next session:

1. Continue from cached exports in `analysis/work/frontend-inspection/`,
   `analysis/work/menu-input-route/`, `analysis/work/engine-loop/` and
   `analysis/work/initialization/`. Read the new timing section in `RESEARCH.md`.
2. Read the new desktop log first if the user ran the engine-mouse command.
   Resolve any device/setup/input issue before expanding it. Then continue
   original relative-motion/device cursor positioning and input object
   construction after the new CURZOR desktop check.
   The known source/units do not make the host's timestamps equivalent to it.
   Original captured-control objects, shared input object lifetime, engine
   device cursor positioning and screen/control ABI remain absent.
3. Keep the probe observational. Selector zero means **any** button, not button
   index zero. Do not remove pointer-departure cancellation until safe Win32
   capture and outside-window releases are implemented.
4. Preserve UX/mode gates. User desktop command:

   ```sh
   python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --probe-confirm-picking --frames 0
   ```

Latest desktop evidence is the 2026-10-29 wheel run: user sees expected no-op;
log confirms Yes wheel deltas/event, 432 frames and clean teardown. Earlier
engine-button run had No, Escape, Yes, right-click user report and 807 frames.
New CURZOR binding/update awaits desktop validation; no double-click dispatch
claim is made. Logs are ignored and overwritten at the next launch.

## Earlier milestones and detailed evidence

The latest desktop `--menu-ui` run **works**: original artwork, cursor and static
English bitmap text render, and the bounded Exit adapter closes the host.
The user confirmed today's horizontal-inset correction **works as expected**.
The user also confirmed original normal/actived button-visual selection:
"Yup, works well. Just as I remember it. Continue". Only Exit responds to hover; the remaining
buttons initialize in their normal state and stay disabled.
Do not claim the original menu dispatcher or complete game has been recovered.

Read [README](README.md), [the text/Exit notes](RESEARCH.md#static-text-and-exit-slice)
and [the UI evidence map](analysis/menu-ui-map.json) next.

## Today's layout correction

- Reused cached Ghidra exports; no fresh export or new unit tests.
- Original text-object constructor `00661B30` defaults byte `+90` to 10.
  Button initializer `006550E0` retains it; plain static-text initializer
  `0065C680` overrides it to 1. The main-menu definitions do not override it.
- Original `00644A80` projects X with an inset of
  `trunc(width * byte_90 / 1024)`: 7 pixels at each button edge at 800x600,
  zero for plain static labels. The host now applies that verified step.
- Text rectangles are separate from raw visual rectangles. Exit hover/click
  bounds, fonts, vertical geometry, artwork and other modes are unchanged.
  Binding logs now include both `bounds=` and `text=`.
- A real-asset C++ geometry audit against the cached rounded runtime bounds
  found all 13 labels fit horizontally (widest button text 204 pixels versus
  approximately 309.4 pixels available after inset).
- Validation: existing native geometry checks passed; strict i386 build passed;
  UI evidence validated 30 anchors, 3 engine and 2 client vtable slots.
  No agent-side Proton launch was attempted (known window-driver limitation).
- Desktop follow-up: user reported "Good work; works as expected. Continue".
  Direct log confirms corrected bounds, Exit activation, 971 frames and teardown.
  This is bounded confirmation, not a claim of pixel-perfect original layout.

## New button-visual slice (desktop-confirmed)

- [Button visual adapter](recovered/host/menu_button_visuals.h) reuses the bounded
  exact-name lookup for all nine normal/actived pairs. It is created only in
  `--menu-ui`; `--menu-model` remains an untouched artwork/cursor preview.
- Cached original code `006550E0` aligns active local XYZ to normal when flag
  `+E0 & 0x80` is clear, then hides/updates active. The adapter reproduces the
  same local-position/dirty-flag bridge; do not substitute a world-space setter.
- Original focus callbacks `00655250`/`006552A0` switch visibility and update
  both visuals. The engine visibility setter is stdcall `1001CFE0`, vtable `+30`.
  One hover sample now selects Exit artwork and caption color in the same frame.
- All unsupported controls remain normal/disabled; Exit hover/leave switches
  `normal07`/`actived07`. Focus loss or leaving the client restores normal.
  Text bounds and click bounds remain unchanged. No new assets are needed.
- Strict i386 build and existing native geometry checks pass. Expanded UI map
  validates 40 anchors, 5 engine and 4 client vtable slots. No new tests/exports.
- Latest desktop log: nine pairs initialized, repeated focused/normal Exit
  transitions, Exit activation, 575 frames and normal teardown. User confirmed
  the behavior matches their recollection. Not exhaustive pixel-perfect proof.
- This is a host-owned focus adapter, not the original dispatcher, focus sound,
  tagged-event routing or confirmation screen. Do not claim those are recovered.

## Current continuation: original quit confirmation

- [UI map](analysis/menu-ui-map.json) now records the original main-menu path
  and binary anchors. Screen 4 is `CONFIRM` / `confirm`; text style 12,
  localization 2004; Yes/No style 13, localizations 2001/2000; `bok` hidden.
  Exact definition names are `byes*` and `bno*`, not exact runtime frame names.
- At screen-stack depth one, Exit queues screen 4 then deferred `22000004`.
  Yes `14001100` releases deferred events after clearing bit `20000000`;
  No `14001200` discards them. Both then emit `06001005` to pop the screen.
  Released `02000004` reaches `0066D33B` and sets front-end state +4 to 4.
- Do not conflate this with `0066C1D0`'s separate common confirmation object at
  front-end+100: its initializer `00668790` uses style 24 and callbacks.
  Separate cached code `00627060` independently confirms screen 4's model
  placement `(0.5,-0.375,-0.5)` at `0062711F..00627147`.
- [Resource stager](tools/stage_menu_resources.py) now accepts
  `--include-confirm` for `MODELS\confirm.4ds` and all five original textures.
  Existing manifests upgrade safely; later default staging retains ownership
  of these optional assets. Only `X_NMNBTN.BMP` overlaps the main-menu assets.
  No automatic launcher or host behavior change; Exit still closes directly.
- New opt-in `--confirm-model` diagnostic stages the original model and opens
  it alone at the verified screen-4 position, logging the loaded hierarchy.
  It is artwork-only, not a reconstruction of modal composition or input.
  The diagnostic was subsequently desktop-checked; its original command was:

  ```sh
  python3 -B tools/run_engine_window.py --confirm-model --frames 0
  ```

  Close with Escape or the window close button. The direct host log is overwritten
  on each launch. It may show uninitialized overlapping visual states; do not
  interpret the artwork-only diagnostic as the original modal composition.
  Strict i386 build, all existing native checks, 29 existing resource/launcher
  checks, launch-mode isolation audit and 61 UI binary anchors passed.
  Do not attempt Proton in the agent environment.
- Desktop diagnostic loaded successfully. The user observed a blank confirm
  dialog with button boxes scattered around: this is the raw diagnostic model
  without text or initialized control visuals, not the normal menu or intended
  final dialog. The runtime hierarchy contains 13 frames: `bok` with
  `actived04`/`normal03`; `shadow01`; `confirm` with child `text`; `bno?` with
  `normal01`/`actived01`; and `byes?` with `actived03`/`normal02`. Literal `?`
  characters are part of the actual runtime names; definition patterns contain
  `*`. The log was still open when inspected, so no shutdown claim is made.
  Return to the regular menu with `--menu-ui`, not `--confirm-model`.
- The diagnostic now initializes all three original visual pairs through the
  same shared adapter as main-menu buttons, and logs projected `text`, `byes?`
  and `bno?` bounds. It still has no prompt, input or original modal
  composition; `bok` is not hidden merely because its control is disabled.
  The user confirmed the updated desktop diagnostic displays the popup and all
  three buttons. Logged bounds at 800x600: text `(288.3,248.6)-(511.2,347.9)`,
  Yes `(296.0,340.8)-(392.2,364.7)`, No `(406.9,340.8)-(503.2,364.7)`.
  The third button is the `bok` visual: it remains visible in this diagnostic,
  which does not implement original control disable/visibility semantics. Its
  appearance is not evidence that the final Yes/No popup should show it.
  The subsequent interactive confirmation was desktop-confirmed as described below.
- New experimental `--menu-confirm` mode builds on original model/labels and
  host-owned bounded mouse input. Exit opens confirmation, Yes exits, No or
  Escape cancels. Usual `--menu-ui` remains unchanged. Original control event
  `01000004` dispatches vtable `+14` to `0065C790`, which hides and updates the
  bound frame: the middle `bok` must be hidden in this Yes/No screen.
  Staged localized prompt/Yes/No use IDs 2004/2001/2000 and existing Set2
  font. Real-font audit found widths 98/23/19 pixels against available
  222.9/82.2/82.3, respectively.
- To run the opt-in experiment on the desktop:

  ```sh
  python3 -B tools/run_engine_window.py --menu-confirm --frames 0
  ```

  It is not the original dispatcher and its keyboard/focus behavior is bounded.
  Existing default mode can be restored using `--menu-ui`.
- Desktop test confirmed Exit opens the popup, No returns to the menu and Yes
  closes cleanly (178 frames). The user reported menu captions over the popup
  and a grey box behind the prompt.
- Original static-text initializer `0065C680` sets the text visual direction
  to `(0,0,-1)` through engine vtable `+0C` before calculating its bounds.
  The experimental confirmation now does the same, using ABI-checked
  `1001C8D0`; the user later confirmed the grey box is gone.
- Original front-end render `0066DE00` draws the previous screen first, then
  disables driver state 5 and draws the current screen in a second pass while
  switching model visibility. The experimental host now follows that ordering,
  drawing main text between the passes and confirmation text after the second;
  state 5 and both model visibilities are restored. No clipping approximation.
  The two-pass rendering and prompt-frame orientation are built and passed
  existing checks. **User desktop-confirmed (2026-10-27): all issues fixed,
  works well** — no caption overlap, no grey box, No/Yes correct.
- The user chose to pivot from extending host glue to reconstructing original
  front-end functions and gradually replacing the scaffolding. The first slice
  is [button focus callbacks](recovered/frontend/button_focus.h): `00655250`
  and `006552A0` sequence visibility, frame updates, tagged event emission and
  return values; entry checks the original `+64` gate. The host invokes these
  through an adapter and assumes that gate is clear. Original object layout
  remains unimplemented.
  Both functions are tracked as partial, not complete. The user confirmed the
  callback wiring still works; the desktop log shows Exit focus transitions,
  No and Yes, and normal teardown.
- The [control-event routing core](recovered/frontend/control_events.cpp)
  reconstructs the portable observer selection in `0065C900` and tagged-event
  filtering and local/queue routing in `0065CF50`. It preserves distinct
  receiver and stack-context values for callbacks, but uses portable containers,
  callback interfaces and shared payload ownership instead of original ABI,
  intrusive references and queue. These functions are also partial.
- Original `0065D150` initializes a control and sends its definition's `+10`
  list through `0065CF00`; tagged events reach `0065CD40`, which copies them
  into the control event vector. The host loads the original Exit/Yes/No
  activation records through that reconstructed tagged branch and filters
  their focus events through `0065CF50`. No focus record matches. The host still
  uses its own mouse handling. The confirmation hover adapter switches only
  the previously and newly focused controls rather than emitting extra leaves.
  The user reported it works as expected; the runtime log
  independently records Exit, focus transitions, Yes and normal teardown.
- The [menu event queue](recovered/frontend/menu_event_queue.cpp) and button
  activation callback now reconstruct the bounded Exit/confirmation sequence:
  activation emits `04001000`, matching records are queued, and the FIFO pump
  handles push, deferred quit, reply release/discard, pop and quit. On Yes the
  released quit is appended after the already queued pop. Escape queues the
  screen-four discard/pop pair. The host still owns mouse hit/press detection,
  actual model visibility, final quit and the screen object; unsupported event
  cases fail explicitly. `--menu-ui` retains direct Exit. The user reported the
  queue wiring works; that run's log independently showed Exit, No, Exit, Yes,
  pop before quit, 184 frames and clean teardown, but not Escape. `006552F0`
  and `00626B20` are absent from the frozen function inventory, so no ledger
  credit is claimed for those addresses despite their binary evidence.
- [PointerClick](recovered/frontend/pointer_click.h) reconstructs the target
  capture and equality/clear portion of `0066A220`; `--menu-confirm` now uses
  it for Exit and Yes/No. The host still does rectangle hit testing and cancels
  on pointer departure or focus loss, unlike the unmodified original captured
  target logic. The original timing, double-click, frame picking, per-update
  sequencing and object ABI remain unreconstructed. The user confirmed this
  change works. That run's host log independently showed No and Yes, pop before
  quit, 339 frames and clean teardown; that run did not show Escape or establish the
  drag-off result.
- [Screen focus routing](recovered/frontend/screen_focus.h) reconstructs the
  `00656B90` leave path and `00656C30` focus routing through portable adapters,
  plus the ordinary branch of `00656B20`. `--menu-confirm` now uses it for
  Yes/No hover changes. The host assumes both controls eligible; input-update
  timing and special target 15 remain unsupported. The
  user confirmed this integration works. That run's host log independently
  recorded No and Yes, pop before quit, 517 frames and clean teardown; it did
  not show Escape.
- [Screen target lookup](recovered/frontend/screen_target.h) reconstructs the
  original `0065C830` identifier-field selection and first-match search. The
  confirmation definition creates screen `10000000` and, in order, text
  `10100000`, Yes `10200000`, No `10300000` and OK `10400000`. The host maps
  rectangle hits to the two button IDs; the user confirmed this integration
  works. That run's host log independently recorded No twice, Yes, 588 frames
  and clean teardown, but not Escape.
- [Pointer picking](recovered/frontend/pointer_pick.h) now reconstructs the
  original cursor-to-ray and picked-frame identifier subset. A separately
  gated diagnostic calls the original scene picker and logs its ID alongside
  the rectangle result. The user confirmed it works; an earlier probe log
  showed matching Yes/No samples and a No-engine/empty-rectangle disagreement.
  A later probe showed matching Yes/No samples and a Yes-engine/empty-rectangle
  disagreement, without recording its position. It closed after 351 frames.
- A new opt-in `--use-confirm-picking` mode routes confirmation hover and
  press/release target selection through the original engine picker. It tags
  both Yes/No visual frames and preserves host cancellation and default modes;
  other controls, original input timing and screen/control objects remain
  incomplete. The user confirmed it works. The latest log confirms the active
  mode, records No, Escape and Yes, and shows 741 frames and clean teardown.
  Its edge samples do not record positions or establish exhaustive coverage.
- The opt-in mode now forwards sampled cursor motion through the original
  `00669FF0` movement predicate and the ordinary `0066B990` screen-motion
  branch. Motion within an already-focused button produces its `04005000`
  event; the host logs that event once per button to avoid flooding. Default
  rectangle-driven modes are unchanged. The host samples absolute positions,
  not original relative input, and does not reconstruct drag gating or full
  timing. The latest log records `04005000` for both Yes and No; the probe
  remains observational even when combined with the interactive mode.
- [Button click dispatch](recovered/frontend/button_input.h) now reconstructs
  `0064DBC0`'s `04012000` event followed by activation via `+38`, for the
  bounded Exit and confirmation buttons. The target parameter is omitted
  because the button activation callback ignores it. No registered event
  matches `04012000`, so queue behavior is unchanged. The user ran the desktop
  check; its log records `04012000` immediately followed by `04001000` for
  Exit, No and Yes and clean teardown. It does not record Escape.
- [Screen click routing](recovered/frontend/screen_input.h) now reconstructs
  `00656DD0`'s focused-index validation, focused-control callback and
  invalid-focus return 7. Confirmation clicks use it when the sampled focus
  matches the hit. If not, the host logs the mismatch and retains its previous
  hit-based selection, since the original input update order is not yet
  reconstructed and using stale focus could activate the wrong button. The
  user confirmed this works. The latest log shows focused-control routing for
  both No and Yes, and no logged fallback.
  `0064DBC0`, `00656B20` and `0065C830` are absent from the frozen function
  inventory, so no ledger credit is claimed for them. The strict i386 host
  build, existing native checks, 17 launcher checks, 151 binary anchors and
  function accounting pass. No new unit tests were added and Proton was not
  launched by the agent.
- Mouse-state research: selector zero means **any** button, not "button zero."
  `0066A050` implements this and nonzero bitmask selection; `0066A080`,
  `0066A0A0`, `0066A0C0` and `0066A0F0` implement previous/current tests and
  press/release transitions. `0066B990` uses selector 3 for screen button
  transitions. Engine `1006FB30` packs five button bytes into bits 1..10 hex.
  Constructor `0066A4B0` initializes captured/previous target and timer to zero.
  On release `0066A220` emits a double-click flag only
  for matching captured/current and previous/current targets with elapsed
  timer strictly below 400; every release resets the timer and capture.
  `0066B990` dispatches screen `+60` before ordinary click `+5C`. The focused
  button's double-click callback emits `04013000`; its click callback emits
  `04012000` and then calls activation. The new
  [button transition](recovered/frontend/pointer_buttons.h) and
  [click timing](recovered/frontend/pointer_click_timing.h) subsets are wired
  only into an observational `--probe-confirm-picking` shadow. It logs Win32
  button snapshots, picked identifiers, captured targets and click/double-click
  flags; message time in milliseconds is an approximation, not a verified
  original tick source. It resets on focus loss and between confirmation
  sessions. Original polling, timer units, update ordering and screen state
  remain incomplete; no double-click events are dispatched. Do not remove host
  cancellation on pointer departure without safe Win32 capture/outside release.
  The user reported it works; the latest log records matching left-button
  press/release captures and `click=1 double=0` for No and Yes, then Escape and
  clean teardown. It contains no right-button or `double=1` samples, so
  multi-button and timed double-click probe behavior is not desktop-verified.
- Next: expand original control and screen object construction, input and
  event routing beyond the bounded confirmation path. No new unit tests.
- Validation: 29 existing resource/launcher checks passed. Real-archive audit
  upgraded 16 assets to 21, preserved all prior records, checked every output
  hash, retained identical files/manifest through default restaging, and
  rejected a modified confirmation model before writes. Temporary assets were
  removed; desktop runtime and game stayed untouched. UI map validates 59
  anchors; front-end evidence and frozen function accounting also pass.

## Project scope and working agreement

- Reconstruct the base Steam Hidden & Dangerous 2 client and original LS3DF.dll.
  Sabre Squadron is excluded from current work and function counts.
- Prioritize the original menu over deeper ALPS1 mission research.
- Use our independently written i386 Windows host with the original engine and
  locally installed assets through Proton, not a lookalike replacement UI.
- Preserve original game files and the original Ghidra project. Generated assets,
  binaries, logs, toolchains and pseudocode remain ignored and local.
- Sensible local commits are authorized. Never push without explicit approval.
- Function baseline: 18,783 internal non-thunk functions; 29 partial, 0 complete.
  Integration glue and research do not receive reconstruction credit.

## Earlier text and hover desktop evidence

After the original text/Exit milestone, the user reported text crossing borders.
After restoring the original horizontal inset, the user confirmed:
"Good work; works as expected. Continue".

The earlier hover run followed the user's "Yup, works well" confirmation.
Its log contained nine initialized visual pairs, all 13 text bindings,
focused/normal transitions, `Exit control activated`, 575 frames and clean
teardown. That log was overwritten by later runs; the observation is preserved
in the UI map. Exact pixel-perfect visual fidelity is not established.

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

The verified missing horizontal inset is restored and desktop-confirmed.
Continue comparing actual original layout/style behavior against our geometry;
do not paper over future mismatches with guessed offsets, clipping or smaller
fonts. Current mode is fixed 800x600,
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

In the main menu only Exit is enabled. Original confirmation,
tagged-event dispatch and focus behavior are only partially reconstructed in
the experimental `--menu-confirm` path; the default `--menu-ui` Exit still
closes directly. Original focus sound, other interactive screens and real
dynamic profile/date/save/version values are absent. Logos are original
`Gamemovie/Logo1.bik`, `Logo2.bik`, `Logo3.bik`; playback, audio, gameplay,
Sabre Squadron and deeper ALPS1 work remain deferred.
