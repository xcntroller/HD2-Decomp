# Menu frame timing, capture and focus loss

Research and implementation cover the full shared queue/update/drag/capture and
focus-loss checkbox. The normal application now uses the recovered sequence.
**Native sanitizer/Windows i386 checks pass; the user desktop-confirmed the
bounded Main/Exit behavior on2026-10-31.**
Unused capture/edit/overlay/listener paths are source-only. No new screen is
made available. The original engine window procedure, DirectInput, device/cache
restore and sound remain delegated services and earn no new credit.

The [mixed binary map](menu-update-map.json) pins both existing original SHA256
identities, 19 instruction anchors, 54 update/graphics virtual slots and 12 direct
calls. Prior [screen/common](menu-screen-foundation.md),
[character/focus](menu-character-focus.md), [frame input](frame-input.md) and
[menu UI](menu-ui-map.json) evidence remains authoritative for unchanged bodies.
Read-only exports are in ignored `analysis/work/menu-frame-timing-20261031`,
`menu-engine-focus-20261031`, `menu-text-render2`, `menu-container-input` and
`menu-cursor-contract`. No original client, DLL, Windows fixture or Proton was
executed by the agent.

## Exact frame and queue contract

Client `0066BEA0` calls frontend virtual +48, conditionally updates sound,
calls +48 again, and unconditionally updates sound again. Only the first sound
call is skipped when frontend +20 is11 and global00982100 low three bits are
not7. The byte at +14 enables wrapping addition of elapsed milliseconds to +18.
These callbacks precede the queue. The bounded application uses normal mode10.

`0066C770` drains until the queue is empty. Each event is retained in a local,
removed/released from queue storage, and the count/storage is updated BEFORE
`0066C950` routes it. Events enqueued by routing callbacks belong to that same
drain, including the existing deferred-release replies. Native deque/move and
shared ownership preserve this order; they do not reproduce the segmented
allocation or intrusive reference ABI.

After that single drain, `0066BEA0` tests screen-stack count once. If nonzero:

1. Dispatch pointer through `0066B990`.
2. Read the current capture owner at frontend +10 and call its +1c update.
3. Resolve the current top screen afresh and call its +1c update.
4. Read optional frontend +fc overlay and call its +1c update.
5. Snapshot the +130/+134 listener count, then walk backward, refetching the
   current list base and entry on every iteration.

There is no second queue pump. Events from pointer/capture/screen/overlay/
listener updates wait for the next update. Keyboard processing precedes this
update through the already recovered application controller, so its queued
work is eligible for the pre-pointer drain. Rendering remains in the established
outer-frame order; this change does not move rendering or polling.

`frontend/frontend_update.h` composes these phases with explicit services.
CommonFrontendState retains typed capture/overlay update owners and token-to-owner
listener bindings. Missing native listener bindings or invalidated indices throw;
no serialized address is called and no unavailable service succeeds silently.
Appends cannot lengthen the listener pass. Mutation of earlier entries affects
later callbacks, as in the original. Native shared owners survive replacement
through their own callback. Destructive mutations that would leave the original
with invalid memory are explicit native errors rather than reproduced UB.

Screen `00668750` emits tag04014000 with elapsed milliseconds, then checks the
fresh focused child and calls its +1c. Group `00656AF0` performs just that focused
child forwarding, without eligibility/visibility gates. DefinedScreenEvents and
DefinedControlTree now compose those paths. Main/Exit text/button tables point
to `0064DA20`, an actual RET4, so their native update bodies are intentionally
empty. Editable `006621E0` ticks its attached text caret only while capture is
active. Other derived widget update bodies stay in the widget-internals item;
this pass does not replace them with empty implementations.

## Pointer, drag and capture integration

The portable `host/menu_pointer_session.h` is shared by the live application
and diagnostic preview. The live overload supplies fresh current-screen/picking
services. The preview explicitly selects its previously accepted host policy.

The application dispatcher gate is checked before picking or input update.
Picking uses the previous integrated cursor position. `0066A220` then receives
the raw relative device deltas; suppression, unchanged-input gating, integration
and clipping happen inside the recovered pointer update. The old application
preintegrated/clamped before picking and derived a second delta; that extra step
is removed. A fresh cursor is centered once and read back through original
IGraph +f8/+fc/+100 (`1006FA40`, `1006FB10`, `1006FB20`), following the sentinel
branch of `0066A4E0` for the bounded800x600 viewport.

`0066B990` reads the shared motion-only byte after pick/input processing. This
is the same state used by keyboard preprocessing. The C2/C0 common messages
also set input +48 through `0066A860` and inverse application input enablement;
the input byte inhibits click capture/release, not cursor sampling.

On a mask3 press over a different target, the typed capture owner's control
identifier is compared and +64 release is called with the picked target. Only
integer result1 permits dispatch to continue. Results0 and7 veto it. A successful
release clears the current capture field unconditionally, including a replacement
installed by that callback. This differs from common BF acquisition, which
accepts any nonzero release result. The typed interface therefore preserves the
integer result instead of collapsing it to bool.

Release clears dragging before drag-end, then resolves the screen again for
focus and again for release. Double-click precedes ordinary click; click checks
the current capture owner afresh. Only click result1 requests the real click
sound. Motion focuses when not already dragging, calls drag-begin while the
flag is still clear, sets it after the callback, then resolves drag afresh.
Held motion that dispatches drag returns before wheel. Wheel dispatch resolves
the current screen, then resolves/picks/focuses afresh after its callback.
The application no longer adds the preview's extra target/focus click check.

Original timing includes strict signed elapsed<400 double-click comparison and
wrapping time. Double-click is now dispatched through actual screen/button
callbacks instead of being forcibly suppressed by the host adapter. Original
press state survives departure and screen changes; moving away and back before
release can still click the captured target. The application has no added
all-buttons-up rearm or screen-transition drag cancellation. Those host policies
remain explicit in the separate engine-window diagnostic, along with its older
queue policy; that preview is not evidence for exact application timing.

## Focus-loss contract and engine boundary

LS3DF WndProc `1006B820` handles WM_ACTIVATE using inactive100D2190 and special
mode100D2191. In ordinary mode, minimization deactivates an active window. Without
minimization, activation restores an inactive window; deactivation checks the
foreground HWND and IsChild(main,foreground), retaining activity for the main
window or a child. The special mode restores an inactive engine regardless of
activation words. This procedure does not clear frontend capture, dragging,
pressed targets, click timing or focused controls.

Deactivation saves the caption. Flags bit20 gates caption suffix/thread suspend;
the inactive byte is set afterward. Restoration restores the caption and clears
the inactive byte before device state callbacks. If the device is present it
invalidates then restores69 cached entries; sound volumes refresh in reverse
order; the optional thread resumes. These operations remain in the original DLL.
The existing application window hook forwards WM_ACTIVATE unchanged to that
procedure; it does not duplicate the engine restore machinery.

Engine pump `1006A470` waits only when the queue is empty, the engine is inactive,
special mode is clear and graphics flags bit20 is set. The bounded windowed
settings use80, so focus loss does not pause this pump or the application update.
Existing device polling/reacquisition supplies the real snapshots, including
zeroed current buttons/keys on its established read-failure paths. Those releases
flow through the ordinary menu dispatcher. The application now preserves cursor,
press/capture, drag and click timing across focus loss instead of clearing them
or absorbing reentry. Cursor drawing remains limited to the focused window;
that presentation choice does not clear input state.

## Verification and accounting

Temporary native ASan/UBSan integration audit covers the actual MenuFrameInput,
DefinedScreen, DefinedScreenEvents, MenuEventQueue and both typed edit owners:

- Capture0/1/7 veto, same-target capture, replacement during release, click owner
  gating, retained-owner lifetime and BF/C0/C2 mode transitions.
- Previous-position picking, gate-before-pick, raw-delta clipping/suppression,
  mode changes inside picking and click inhibition independent of movement.
- Drag flag visibility, screen changes inside drag callbacks, release/focus
  ordering, wheel early return, screen replacement/repick and double-before-click
  at399/400 ms and signed wrap.
- Departure/return presses, held screen transitions, simulated foreground-device
  zero releases with retained state and explicit legacy preview safeguards.
- One drain, reentrant queue append/clear, next-frame post-drain delivery, capture
  update changing current screen, reverse listener live mutation/missing binding,
  all256 sound-gate values in modes10/11, timer wrapping and no-screen tail gate.
- Screen observer changing focus before child update, invalid focus and real edit
  identity/caret/release composition.

The same fixture compiles/links for Windows i386 without execution. All three
strict Windows targets, the existing native suite and158 tooling tests pass.
Temporary audits are removed after verification; no permanent tests are added.
Map validation and frozen accounting pass. Only frozen client bodies0066A860
and00656AF0 gain partial entries:336 partial (331client/fiveengine), zero complete,
frozen18,783. Frame0066BEA0, screen00668750, empty update0064DA20 and edit006621E0
are absent from that inventory and receive no new credit. Original engine
activation/pump/cursor calls remain delegated and receive no new credit.

## Desktop acceptance —2026-10-31

The user confirms the requested desktop checks with “All seems to work” and
explicitly requests recording and committing the change. This covers the
bounded Main/Exit, clicks/dragging, focus loss/return and shutdown checks.
Unused edit, overlay and listener routes remain native-audited/source-only.

Built/staged application SHA256 both match the tested build:
`a5536eabc18024a03f0013560177c90e167b01fac3468ace742303afe9545da2`.
Retained final native log records all four movies opening, actual21 text objects,
Main Exit, confirmation cancellation/reopening, drag-begin/drag/drag-end tags,
status4, state-exit after1006 frames and teardown result0. Alt-Tab/minimize and
quick-click gestures rely on the user's report; the log does not individually
identify those desktop actions. Do not promote unused-widget source audits to
desktop acceptance.

Native/terminal/Wine logs and window.cfg are copied, with hashes and the exact
user confirmation, under ignored `analysis/work/menu-update-acceptance-20261031`.
The manifest preserves each artifact's original path and tested executable hash.
Installed game assets are unchanged. No agent Windows/DLL/Proton execution.
The checkbox is closed and the user has authorized the commit. No push.
