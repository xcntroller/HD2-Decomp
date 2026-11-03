# Phase 1 input fidelity — 2026-11-02

The next section4 checkbox covers **held/repeat input, eligibility, focus,
wheel/drag and modal cancellation** across the delivered front end. The preceding
[presentation checkpoint](menu-fidelity.md#desktop-acceptance--2026-11-02) is
accepted and committed as **b747f45**. This continuation audits the complete
input boundary; it finds no demonstrated runtime defect requiring a code change.
Native verification passes. The user subsequently confirmed the focused desktop
check; the input checkbox is complete within the delivered front-end scope.

## Boundary and dependencies

The scope includes Main/Exit, Options pages/widgets and bindings, Profiles and
their edit/list dialogs, shared runtime dialogs, Cinematics, reachable team
selection and multiplayer front-end controls. It follows keyboard/device state
through preprocessing, character capture, screen/group lookup, concrete control
callbacks, the single queue drain, listeners and modal return. Static artwork,
font layout and resolution conversion are the accepted preceding checkpoint.

Required live adapters are `BorrowedGraphicsInput`, application/preparation
keyboard contexts, `MenuFrameInput`, `DefinedScreen`/nested control trees,
`BuiltOptionsWidget` and its parts, ProfileEdit/Records/List, Options binding
rows/lists, PreMissionList/Choice, CinematicsGrid and the runtime dialog owner.
Existing device polling/reacquisition and window activation remain delegated
engine services. They are not replaced by successful fixtures in the application.
Native audit fixtures supply only device/frame/row services for controlled tests.

Source-only loadouts, game/save receivers and ready-server UI retain their
Phase2/3 exclusions. Exercising a shared helper in a native audit does not make
those screens desktop-tested or their external services available.

## Confirmed original contracts reused

The completed foundation/widget contracts below are reused rather than recovered
again. Existing fingerprint maps preserve the actual instructions and addresses.

| Area | Original evidence and audit boundary |
| --- | --- |
| Keyboard production | `006DB620` initializes with flags0; `1006F190`, `1006F320`, `1006F470` define polling, scan selection and character reads. General scan repeat is disabled in the normal setup. Explicit repeat mode retains strict elapsed >350/>34 thresholds and wrapping time. [Frame input](frame-input.md), [preprocessing](keyboard-preprocessing.md). |
| Keyboard routing | `0066B3E0` offers characters to the current capture owner, then sends the full scan path to the current screen. `00668670` handles Enter/Space/keypad Enter; `00656AA0` gates group forwarding by eligibility. Unconsumed Escape ends a drag before queuing Back. [Character/focus](menu-character-focus.md). |
| Eligibility/focus | `0064D9F0` preserves the bitwise disabled/visible/focusable predicate; `00656B20`, `00656B90`, `00656C30` preserve special target15, fresh lookups, leave veto and pointer focus. Disabled screen activation is gated by its adapter; no blanket eligibility policy is invented for original direct-control calls. [State](menu-foundation-state.md). |
| Arrow/track repeat | `00664270/A0/B0` arm arrow repeat, compare against shared input idle300 and stop on release/leave. `006656D0`, `006659C0` handle the track's signed delta timer, initial300 and subsequent50, with strict negative expiry. Focused and listener updates both remain present. [Widget construction](menu-widget-construction.md). |
| Wheel/thumb | `006658C0` rounds wheel magnitude upward in120-unit steps; positive decrements. `00664810`/`00664980` use normalized selected-axis movement and original travel. Queued thumb updates and immediate arrow-linked updates remain distinct. |
| Pointer/update | `0066A220`, `0066B990`, `0066BEA0`, `0066C770` retain previous-position picking, double-before-click, capture release0/1/7, current-screen rereads, drag-before-wheel and one pre-pointer queue drain. [Update contract](menu-update.md). |
| Capture/edit | `00662790`, `00662880`, `006629C0`, `00662980`, `00662490`, `0066C950` retain delayed BF/C0 ownership, commit/cancel, original-text restoration and identity-dependent input-mode reset. [Capture contracts](menu-character-focus.md). |
| Modal cancellation | `006691B0`, `006693A0`, `00669090`, `0066C950` retain mode0 key swallowing, low-byte Escape, one reply, removal-before-reply and retained underlying owner. Profile C3 requests focus08100000; Options C7 stays its genuine reply. [Dialog integration](runtime-dialog-integration.md), [callers](dialog-callers-verification.md). |
| Screen-specific lists | `006554B0` and existing Profile/binding/movie/team contracts retain nonwrapping navigation, per-list page sizes, veto/capture handling, filtered identity and separate hover/selection. [Profiles](profiles-flow.md), [Options](options-flow.md), [Cinematics](cinematics-flow.md), [team](singleplayer-pipeline.md). |

The app borrows its already initialized keyboard instead of constructing a second
native keyboard owner. Holding an activation key must not cascade across screens
through fabricated scan repeat. Mouse arrow/track repeat is independently driven
by the original widget timers. Text FIFO/message transport remains its existing
source-backed path; this audit introduces no new text-repeat or Unicode policy.

Focus loss follows the established engine/device boundary. The application does
not add a global press/drag/capture reset on Alt-Tab. Current snapshots and normal
release processing remain authoritative. The separate diagnostic preview retains
its explicit historical cancellation/rearm safeguards.

Dropdown cancellation is control-specific: a focused expanded combo consumes
Escape and closes; pressing outside its header/arrow/list closes it through the
existing owner path. No global popup-key interceptor is introduced. Generic
screen/Back dispatch remains authoritative when another control owns key focus.
This distinction is part of the bounded host implementation, not a new original
function-recovery claim.

## Native verification

Two temporary ASan/UBSan programs compose or extract the actual production code:

1. Real `BuiltOptionsWidget` arrow/track/thumb products, mutable messages,
   listeners and text state, connected to real `DefinedScreen`, `MenuFrameInput`,
   pointer/keyboard helpers, `EditCharacterTarget`, `MenuEventQueue`, shared
   capture state and `RuntimeConfirmation`.
2. Exact key/wheel/cancellation methods from ProfileRecords, OptionsBindingList,
   filtered PreMissionList, CinematicsGrid and PreMissionChoice, with native
   row geometry, tree/scene and refresh services. Only the native class shell
   and service backends are fixtures; the tested method bodies are extracted.

Coverage includes:

- Immediate arrow step, idle299/300 repeat boundary, continued repeat and leave
  cancellation; track300/301 and50/51 boundaries, held-release termination;
  wheel ±1/±121 rounding and INT_MIN step count; moving/stationary thumb changes.
- Range/parent-disabled gates, selector press versus held behavior, toggle's
  original default1, hidden/disabled focus entry, unavailable target filtering,
  preserved internal part IDs and suppressed disabled screen activation.
- Actual pointer/screen composition at640×480,800×600 and1920×1080: focus movement,
  established drag source through target departure/release, previous-position
  picking, focus-leave veto, capture release0/7 veto versus1 transfer, wheel
  delivery and held-drag wheel suppression.
- Keyboard preprocessing across screen changes with a held activation key;
  Escape ends an active drag before Back. Explicit repeat mode checks exact
  strict thresholds, released-key cleanup, wrap and destructive character reads.
- Real BF/C0 delayed edit acquisition/release, Escape restoration, Enter observer
  opening a dialog before deactivation, caret499/500 and mode0..3 dialog handling.
  Dismissal does not reactivate the retained screen. Repeated button callbacks
  cannot produce a duplicate reply. Profile C3 focus delivery occurs after pop.
  BF nonzero-release policy and C0 requester identity/input-mode behavior pass.
- Profile/Home/End/Page clamp and hover selection, binding nine-row paging,
  focused capture/leave veto, filtered team roster navigation and three-row
  fixture paging, unlocked movie adjacency and separate focus/selection; focused
  combo Escape and header/list/arrow/outside-press dismissal at three resolutions.

Existing source/native audits remain authoritative for unchanged special cases:
binding capture warm-up/assignment/duplicate clearing and200ms suppression;
numeric edit clamp/validation; profile error reply contexts; team drag ghost/
removal/locked policy; multiplayer map reorder and service eligibility. Their
live flows are already accepted within their documented boundaries. This audit
does not claim fresh desktop execution of those cases.

Both audit compositions compile/link strictly for Windows i386 without execution.
All three application targets, existing native regressions,158 tooling tests,
confirmed input/focus/widget maps and frozen-ledger checks pass. No runtime code
changes were necessary. The accepted built/staged executable remains SHA256
`bb8cbe802887e64d25f7ea8f4c700f47faaa822730c1c0214e26239331340071`.
No new staging or profile/config changes are needed.

Logs: `analysis/work/input-fidelity-20261102/{sanitizers,adapters-sanitizers,
windows-audits,windows-build,native-tests,python-tests,evidence,function-progress}.log`.
Temporary generators, native/Windows sources and binaries are removed after
verification. No permanent tests are added, and the agent executes no original
game/DLL, Windows fixture or Proton. Function credit is unchanged: **564 partial
(559 client/five engine), zero complete /18,783**.

## Focused desktop review supplied

Use the normal launcher and current accepted build; no full route repetition is
required. Check these interaction boundaries on reachable controls:

1. Hold Enter/Space while entering and leaving a screen; it should not activate
   successive destinations. Hold an Options slider arrow/track, then release or
   move away; repeat should stop. Wheel and thumb movement should remain bounded.
2. Disabled/hidden settings stay unavailable. Hover between an active text edit
   and another control, then click to transfer input. Binding capture accepts or
   cancels with Escape without accidentally leaving Options.
3. Wheel and Home/End/Page through a populated list. Start a team/map drag and
   release elsewhere; check Escape/Back and focus return with no retained ghost
   or unintended activation. The accepted team-removal rules remain in force.
4. Cancel an edit, a focused expanded preset and an ordinary Yes/No/OK dialog;
   reopen each and confirm one reply and usable underlying focus. Include a
   profile duplicate/error return or Options unassigned-binding prompt where
   convenient. Check Alt-Tab/minimize during capture or a held interaction.

## Desktop acceptance — 2026-11-02

The user reports: “All seems to work fine - record as completed and continue”.
This accepts the focused input review above on the unchanged current build,
SHA256 `bb8cbe802887e64d25f7ea8f4c700f47faaa822730c1c0214e26239331340071`.
The confirmation is an overall desktop result; no individual case execution or
additional logs are inferred. Native coverage and Phase2/3 exclusions remain as
documented. Section4's input checkbox is checked. No commit was requested by
this confirmation; sound/music/preferences/failure/cleanup is the next item.
