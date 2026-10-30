# Entire menu reconstruction plan

Updated: 2026-10-30. This is the menu delivery tracker. [HANDOFF.md](HANDOFF.md)
records how to resume; [RESEARCH.md](RESEARCH.md) records detailed evidence.

## Goal and completion criteria

Rebuild the base game's menu in our own readable, compiled client, initially
using the original LS3DF.dll and locally installed assets through Proton.
Complete means all reachable base-game menu screens and dialogs behave as the
original: rendering, navigation, controls, settings, profile/save data and
transitions to game/network services. Menu flows must reach a documented,
validated handoff to those services; rebuilding gameplay is a separate goal.
Sabre Squadron is outside the current scope.

Engine work follows menu dependencies: recover each necessary contract and
replace engine bodies in bounded slices. Replacing the whole LS3DF.dll is a
separate milestone; loading the original DLL does not count as reconstructing it.
Keep an explicit list of delegated calls so the eventual replacement is tractable.

## Current position

| Area | Status | Remaining work |
| --- | --- | --- |
| Independent i386 host, original engine window and teardown | Desktop-tested | Recover original application lifecycle beyond host scaffolding |
| Original main-menu artwork, cursor, bitmap text and hover pairs | Desktop-tested, bounded | Dynamic captions, localization and fidelity across settings |
| Exit and Yes/No confirmation | Shared controls/text, stack and targeted tree integration desktop-confirmed | Full factory, original observer/data callbacks and remaining flows |
| Shared pointer dispatch, picking, wheel and held-button transitions | Desktop-tested, bounded | Original object ABI, capture/device lifecycle and exact timing |
| Enter/Space on focused buttons | Desktop-tested | Keypad Enter audited, hardware test unavailable; device/character transport incomplete |
| Escape opens confirmation; Escape cancels it | Desktop-tested | Application interceptors and other screen-specific back rules |
| Previous/next focus traversal | Source reconstructed | Find original callers/key bindings and initial-focus policy before wiring |
| Original button sound and sound-driver lifecycle | Desktop-tested | Original preferences/resource cache and additional menu sounds |
| Shared control initialization and state messages | Native base contract complete; live text/buttons desktop-confirmed; derived widgets audited | Screen-specific callbacks, widget visuals and original ABI tracked separately |
| Shared factory and nested containers | Leaf factory, engine bridge and recursive group diagnostic desktop-confirmed; mixed leaves native-audited | Actual options definitions, nested screen availability, other widget kinds and original ABI |
| Options sliders, selectors, toggles and settings observers | State/message/input and observer routes reconstructed, native-audited | Engine visual/internal-part construction, actual preference services and live options integration |
| Other main-menu actions/screens | Not functional yet | Inventory, reconstruct and enable one flow at a time |

Keyboard sound check: ordinary Exit/Yes/No keyboard activation is silent in the
recovered original path. Screen key handler 00668670 invokes activation +38;
button 006552F0 emits 04001000. Pointer dispatcher 0066B990 separately invokes
sound helper 0066B110 after an accepted click. Front-end keyboard pre-hook +44
is the no-op 0067EA80. Preserve this behavior. A separate options-volume callback
also invokes the sound helper, so this finding is not a claim about every widget.
This conclusion is from original binary analysis, not a new original-game run.

## 1. Finish the shared menu foundation — current priority

The foundation is sufficient when our readable native controls and services can
deliver the real menu flows with the recovered behavior. Matching every original
client class layout is tracked separately and is not a prerequisite for native
menu delivery. The remaining shared work should be completed alongside options,
profiles and other real screens, rather than expanding this section indefinitely.

- [x] Bootstrap original graphics/3D engine, scene, models, font assets and cursor.
- [x] Recover bounded button events, focus/hover, picking and pointer sequencing.
- [x] Recover focused-button activation and queued Escape/back for the two live screens.
- [x] Restore original button.wav decoding, sound frame and accepted-click playback.
- [x] Check ordinary-button keyboard sound against original code.
- [x] Reconstruct shared control state, initialization/event messages and eligibility.
  Native base contract complete: all selectors, mutable initialization records,
  guarded derived setters, visibility/notification ordering, typed observers,
  retained local/queued events, fresh child layout and byte-level eligibility.
  Live text/buttons are desktop-confirmed; slider/selector/toggle inheritance is
  sanitizer/i386-audited. Eligibility matches the isolated original instructions
  for all 16,777,216 byte combinations. See [evidence](analysis/menu-foundation-state.md).
  Widget visuals, settings services and screen factories retain their own open
  entries below. Original class ABI remains separately tracked.
- [x] Replace hardcoded Exit/confirmation adapters with reusable controls and definitions.
  **Desktop-confirmed.** Both screens now
  use one `DefinedScreen` routing class and `DefinedControls` construction/ownership
  path. Original 17/four child slots, button definitions and supported event
  bindings are centralized; the two specialized adapters are removed.
- [ ] Recover screen creation, binding, lifetime, push/pop and general event dispatch.
  Registered stack, regular/modal transition distinction, pop/back/resume and
  visibility/focus lifecycle integrated and desktop-confirmed for the two live
  screens. Targeted screen/child delivery and native observer plumbing are
  integrated, audited and desktop-confirmed. Shared factory loop and screen +8c
  binding now reconstructed with typed builder hooks, mutable root initialization,
  retained metadata and an owning registry accepted by the shared stack; native
  sanitizer/i386 audits pass. The host still uses its two-screen construction.
  Slot-zero definition/request/reply/key protocol, construction ordering and
  02000096/97 routing are now native-audited; actual engine slot-zero product/input
  binding is absent. Concrete builder/backend integration, application callbacks
  and remaining routing stay open. See [screen notes](analysis/menu-screens.md)
  and [modal evidence](analysis/menu-runtime-confirmation.md).
- [x] Integrate bounded recursive kind-five construction, ownership and input routing.
  Implemented with existing leaf backends; mixed-tree sanitizer/i386 audits pass.
  Updated five-frame engine diagnostic desktop-confirmed (372 frames). Actual
  options definitions, widget constructors and nested screen policy remain open.
- [x] Integrate bounded registered stack/lifecycle and all 21 current child message endpoints.
  Desktop-confirmed; background targeting, native observers and retained-copy
  query semantics additionally audited. Original dynamic lists remain unattached.
- [ ] Recover keyboard preprocessing, character input, focus entry and navigation callers.
- [ ] Resolve exact queue/update ordering, timing, drag/capture and focus-loss behavior.
- [ ] Integrate original settings and resource-cache contracts instead of host defaults.
- [ ] Supply the options widget backends and connect original settings observers.
  Slider/selector/toggle native state, messages, focus/input and four observer
  routes are reconstructed and sanitizer/i386-audited. Engine internal controls,
  real preference services and live integration remain open.

Current session checkpoint: shared-state entry complete; screen construction and
slot-zero protocol recovered and native-audited. User confirmed the preceding
screen metadata/stack changes (380 frames, clean teardown). Next work is concrete
screen-product ownership and engine-backed slot-zero integration, followed by
remaining application callbacks. The broad screen entry above remains open.

Earlier desktop-confirmed slice: targeted screen event trees, retained-copy delivery and
native screen observer bindings. All 17/four child slots accept messages,
independently of host input availability. Nonzero targets route to any bound
screen, including the menu under a dialog. Root base messages, child lookup,
focus requests and lifecycle notifications share the same runtime state.

Strict i386 build, temporary native/sanitized/i386 audits and existing checks
pass. **User desktop-confirmed** (507 frames, clean teardown). Included in the
committed session checkpoint. Only main and
confirmation are bound. Original dynamic observer lists remain unattached;
profile/date/save/version values and other flows are unfinished. Previous stack
slice is desktop-confirmed and committed as 335c519 (3320 frames, clean teardown).

Latest desktop-confirmed slice: all 21 leaf controls use the shared
definition factory, and source-only kind-five container initialization, IDs and
dispatch have been audited with fake backends. This does not complete the general
factory or enable another screen. The options helper 0066dbf0 creates an engine
type-six dummy frame, distinct from a kind-five UI container.

The engine dummy-frame bridge and nested state/event probe are implemented and
desktop-confirmed (425 frames, clean teardown). The probe is not the actual options screen. The
[options inventory](analysis/menu-options.md) identifies six settings groups,
their widgets and remaining callback dependencies.

Current slice: recursive kind-five ownership/factory, scoped frame lookup and
nested input are implemented and audited. The engine diagnostic now constructs
five controls over three levels from definitions and is desktop-confirmed (372 frames);
mixed nested text/button controls are native-audit verified only.

Current source-only slice reconstructs slider/selector/toggle state, messages,
focus and input plus graphics/float/toggle/voice observer routes. Native audits
exercise the connected graph, including silent preset updates without feedback.
No new desktop behavior is enabled and no repeat desktop test is needed for
these source-only additions.

Next priority: engine visual/internal-part construction for these widgets, real
preference services, actual options definitions and screen availability, then
full entry/apply/back behavior. Do not enable a screen solely because its model loads.

Prefer complete plan items or cohesive dependency groups per iteration, as
requested by the user. Avoid returning for desktop checks after individual
message handlers when they can be validated together in one larger slice.

## 2. Establish the complete screen and dependency inventory

- [x] Inventory all 14 static registrations in 00623DD0 and map the 11
  identifier-specific augmentation builders in 00627060 (source/evidence only).
  [Screen inventory](analysis/menu-screens.md) and [structured metadata](analysis/menu-screens.json)
  record 53 static children, 100 initializer call sites and builder dependencies.
  Five/seven are distinct despite their shared name/model; runtime zero is a
  separate dialog. Nested/derived and in-game inventory is still unfinished.

- [ ] Enumerate every definition, derived/dynamic screen and shared dialog reachable
  from startup, the main menu, and the in-game menu; cross-check definitions,
  event routing, resource keys and callers. Identify which in-game flows require
  a running game and provide a menu-focused way to validate their UI contracts.
- [ ] For each screen record original identifier, model/assets, control kinds,
  captions, initialization, transitions, data dependencies and return behavior.
- [ ] Resolve duplicate/placeholder definitions before treating them as separate screens.
- [ ] Expand the table below into individual screen checklists as evidence is recovered.

Initial families below are work packages, not a claim of a complete screen list.
00623DD0 directly establishes main_menu, change_profile, singleplayer, confirm,
single_mission_2, options, multiplayer_menu, create_server, password,
server_filters, gameinfo, MP Connect console and Add Server definitions.
Additional screens/content are supplied elsewhere. Main-menu buttons also expose
resume, cinematics and credits; LAN and Internet have separate entry actions.

| Work package | Work required before completion | Current status |
| --- | --- | --- |
| Main menu | All nine original actions, live profile/last-save/date/version labels, eligibility | Exit only functional |
| Shared dialogs | Yes/No/OK, prompts, text/password entry, errors, cancellation and layering | Quit Yes/No only |
| Credits and cinematics | Recover controller, scrolling/list/playback and return | Credits enters application state 12; Cinematics selects distinct slot seven/builder 00614140 |
| Options | Recover nested pages, shared widgets, settings/apply/cancel/persistence | Slot six/builder 0061cf60; entry 0061bea0; five settings groups referenced |
| Profiles | List/select/create/rename/delete, validation and persistence | 17 static children, builder 006211b0 and callback dependencies inventoried |
| Resume, saves and loading | Original metadata, previews, selection, errors and game-state handoff; save dialogs where reachable | Not started |
| Single player | Campaign/lone-wolf/carnage and mission-selection flows; recover setup, squad/equipment and briefing screens where reachable | Initial definition located |
| Multiplayer | LAN/Internet browser, dialogs, create/connect/lobby/setup and errors | Slots eight–fourteen and their builders mapped; entry 006333c0 |
| In-game menu and return paths | Inventory pause/options/save/load/quit variants and reconcile reused screens | Not inventoried |

## 3. Deliver complete flows in dependency order

- [ ] First additional screen: choose credits or another low-dependency screen after
  tracing its actual controller/assets; complete entry, interaction and return.
- [ ] Options: reconstruct shared sliders, toggles, lists, tabs and binding editors
  as their original use requires them. Apply original settings semantics.
- [ ] Profiles and saves: reconstruct editable text, scrolling lists and dialogs;
  use isolated writable copies for persistence validation.
- [ ] Single-player setup: reconstruct every reachable screen through the actual
  launch request, with a documented game-controller boundary.
- [ ] Multiplayer: reconstruct browser/create/connect/lobby UI and service contracts;
  distinguish working UI, local/network validation, and unavailable legacy services.
- [ ] Complete in-game menu variants and transitions back into the front end.

For each flow: recover evidence → write readable source → build and audit →
enable the action → desktop-test entry/action/back/error paths → update this
tracker → commit the working slice. Split large families into concrete screens
before marking them complete. Do not enable dead buttons or report a placeholder
service response as a completed original flow.

## 4. Finish fidelity and replace remaining scaffolding

- [ ] Verify layout, stacking, clipping, fonts/localization and supported resolutions.
- [ ] Verify pointer/keyboard rules, repeat, held transitions, disabled/hidden controls,
  focus changes, wheel, drag and modal cancellation against recovered behavior.
- [ ] Verify sound/music, preferences, failure paths and deterministic cleanup.
- [ ] Audit every host approximation and delegated engine/service call; record or
  reconstruct remaining menu dependencies, with source and binary evidence.
- [ ] Run a full menu traversal checklist after individual screens pass; record
  unavailable environments/services explicitly and leave dependent items open.

## Evidence and validation rules

- Update this file after each slice, with the screen/behavior and its validation
  state. A checked delivery item means integrated and desktop-tested unless it
  explicitly says source-only. Research and partial functions are not completion.
- Fingerprint and anchor original-client/engine evidence. Keep proprietary assets,
  binaries, generated pseudocode and logs local/ignored; preserve original game files.
- Build the strict i386 host, run relevant existing checks and temporary focused
  audits as needed. No new unit tests under the current project agreement.
- Commit interactive changes only after the user confirms desktop behavior works.
  No agent-side Proton launches; no pushes without explicit user instruction.
- Track function reconstruction separately in [README.md](README.md) and
  [analysis/function-status.json](analysis/function-status.json). Its frozen
  18,783-function denominator is not a percentage of menu delivery.
