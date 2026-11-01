# Entire menu reconstruction plan

Updated: 2026-11-01. [Handoff](HANDOFF.md) records the latest work and next steps;
[research](RESEARCH.md) and linked analysis documents hold technical detail.

## Goal and completion criteria

Rebuild every reachable base-game menu screen/dialog in our readable compiled
client, initially using original LS3DF.dll and local assets through Proton.
Match rendering, navigation, controls, settings/profile/save data and service
transitions. Validate game/network handoffs; gameplay and Sabre Squadron are
outside scope. Engine replacement is a separate milestone; delegated calls stay
explicit.

Checked delivery items mean integrated and desktop-tested unless marked
**source-only**. Research and partial functions do not establish delivery.

## 0. Rebuild the original launch path to the menu — bounded checkpoint complete

Delivered ordinary opening movies → recovered controller transition → compiled
Main/Exit. [Final acceptance matrix](analysis/launch-acceptance.md) records tested
builds, evidence and exclusions. The older preview remains a separate diagnostic.

Keep this delivery checklist stable. Record subsystem slices in the linked
[recovery index](analysis/launch-next-steps.md#recovered-source-index), not as new
checkboxes. Add prerequisites only when original call evidence proves launch
requires them; recovery of a helper alone does not complete a delivery item.

- [x] Integrate original startup owners, resources, settings, input and localization.
  [Bounded startup checkpoint](analysis/startup-integration.md) desktop-confirmed 2026-10-30.
- [x] Integrate original video/menu states and controller update/render transitions.
  [Bounded controller delivery](analysis/controller-integration.md) desktop-confirmed 2026-10-30.
- [x] Complete opening playback, skips, music/subtitles and movie-to-menu cleanup.
  [Ordinary playback delivery](analysis/playback-completion.md) desktop-confirmed 2026-10-31.
- [x] Preserve failure handling, partial-startup cleanup and normal shutdown.
  [Bounded shutdown delivery](analysis/shutdown-delivery.md) desktop-accepted 2026-10-31.
- [x] Build the independent launch target; desktop-confirm playback, skips, missing
  movies, menu entry and teardown, then commit after user confirmation.
  [Consolidated acceptance](analysis/launch-acceptance.md), shutdown commit3f976d4.

Baseline: the four-movie sequence and host preview are desktop-confirmed.
[Opening evidence](analysis/opening-video.md). Recovered source components remain
separate from delivery; their completed work is preserved in the recovery index.

Current: settings/cache integration is desktop-confirmed; complete native options
widget internals/settings wiring are **source-only**, audited through the actual
factory/backend. [Widget contract](analysis/menu-widget-construction.md).
[Completed input/focus contracts](analysis/menu-character-focus.md).
Full registry/setup bootstrap and binary integrity VM remain outside the checkpoint.

## Current position

| Area | Verified | Remaining |
| --- | --- | --- |
| Original controller launch | Four movies/subtitles → Main/Exit and bounded failure/shutdown accepted | Other menu flows; broader failures remain excluded |
| Menu visuals | Original artwork, cursor, text and button hover | Dynamic labels, localization, fidelity |
| Exit/confirmation | Pointer, Enter/Space, Escape and modal cancellation | Other actions/dialogs |
| Shared controls/screens | State, messages, factory, stack and nested diagnostic | Screen callbacks, dynamic lists, original ABI |
| Options widgets | Native parts, engine backend, factories and real settings wiring audited | Full Options flow and desktop validation |
| Startup dependencies | Data/config/settings/input/archive source audited | Full ownership, protection, caches and live wiring |

## 1. Finish the shared menu foundation — alongside launch dependencies

Complete foundation work as real flows need it; avoid expanding it indefinitely.
Original object ABI is tracked separately from native menu delivery.

- [x] Bootstrap graphics/3D scene, models, font assets and cursor.
- [x] Recover button focus/hover/picking and pointer sequencing.
- [x] Integrate focused activation and queued Escape/back for the live screens.
- [x] Restore button.wav/driver lifecycle and accepted-click sound.
- [x] Verify ordinary Exit/Yes/No keyboard activation is silent in original code.
- [x] Recover shared state/initialization/messages/eligibility. Derived widget
  contracts are **source-only**. [Control state](analysis/menu-foundation-state.md).
- [x] Replace Exit/confirmation adapters with reusable definitions and controls.
- [x] Recover screen construction/binding/ownership, stack and event dispatch.
  Unused service branches are **source-only**. [Screen contracts](analysis/menu-screen-foundation.md).
- [x] Integrate recursive kind-five ownership/input; nested engine diagnostic confirmed.
- [x] Integrate registered stack/lifecycle and all 21 leaf message endpoints.
- [x] Complete character targets, focus-entry/navigation callers and initial-focus
  policy. Unused edit/list/binding routes are **source-only**; Main/Exit preserved.
  [Character/focus contracts](analysis/menu-character-focus.md).
- [x] Resolve exact queue/update timing, drag/capture and focus-loss behavior.
  Main/Exit desktop-confirmed2026-10-31; unused routes source-only. [Contract](analysis/menu-update.md).
- [x] Integrate original settings/resource caches in place of host defaults.
  Main/Exit desktop-confirmed2026-10-31; unused adapters source-only. [Contract](analysis/menu-caches.md).
- [x] Build options widget internals and connect recovered observers/settings.
  **Source-only** factory/engine/settings composition audited; full flow separate.
  [Contract](analysis/menu-widget-construction.md).

## 2. Establish the complete screen and dependency inventory

- [x] Inventory 14 static registrations and 11 augmentation builders. **Source-only.**
  [Screen inventory](analysis/menu-screens.md), [metadata](analysis/menu-screens.json).
- [x] Enumerate remaining derived/dynamic/shared/in-game screens and routes. **Source-only.**
  [Owner/route census](analysis/menu-route-census.md), [catalog](analysis/menu-route-census.json).
- [x] Record IDs, assets, controls, captions, initialization, dependencies and returns. **Source-only.**
  [Control/dependency contracts](analysis/menu-contracts.md), [catalog](analysis/menu-contracts.json).
- [x] Resolve duplicate/placeholder definitions. **Source-only:** 28 populated roles, two reserved slots.
  [Identity/counting rules](analysis/menu-definition-resolution.md), [catalog](analysis/menu-definition-resolution.json).
- [x] Split all families into delivery checklists and independent/running-game validation. **Source-only.**
  [Nine families / 29 delivery items](analysis/menu-family-delivery.md), [coverage](analysis/menu-family-delivery.json).

| Family | Required flow | Current status |
| --- | --- | --- |
| [Main menu](analysis/menu-family-delivery.md#main-menu) | Nine actions, profile/save/date/version labels, eligibility | Exit and Credits |
| [Shared dialogs](analysis/menu-family-delivery.md#shared-dialogs) | Yes/No/OK, prompts, text/password, errors and cancellation | Quit Yes/No accepted; runtime integration accepted through Options; other callers open |
| [Credits/cinematics](analysis/menu-family-delivery.md#credits-and-cinematics) | Retained controller, list/playback and return | Credits and Cinematics accepted |
| [Options](analysis/menu-family-delivery.md#options) | All pages/bindings, apply/Back/persistence and owner variants | Front-end accepted; in-game owner open |
| [Profiles](analysis/menu-family-delivery.md#profiles) | List/select/create/rename/delete and validation | Startup persistence recovered; desktop accepted |
| [Saves/loading](analysis/menu-family-delivery.md#saves-and-loading) | Actual metadata, errors and game/controller handoff | No extra save-screen registration inferred; real receivers required |
| [Single player](analysis/menu-family-delivery.md#single-player) | Campaign/lone-wolf/carnage, hero/team/equipment/briefing/results | Resolved roles/dependencies; real handoffs remain open |
| [Multiplayer](analysis/menu-family-delivery.md#multiplayer) | Browser/create/connect/lobby/errors and owner variants | Controlled local services required; legacy service availability explicit |
| [In-game menu](analysis/menu-family-delivery.md#in-game-menu) | Pause/player/options/actions and retained-game return | Independent UI audits possible; real game required for full return |

## 3. Deliver complete flows in dependency order

- [x] Deliver Credits through entry, scrolling and return to the retained Main menu.
  [Desktop accepted2026-10-31](analysis/credits-flow.md).
- [x] Deliver Cinematics: available-movie list, selected playback and return/Back.
  [Desktop accepted2026-10-31](analysis/cinematics-flow.md).
- [x] Integrate shared runtime dialogs, preserving named Yes/No/OK and reply semantics.
  [Accepted through Options2026-10-31](analysis/runtime-dialog-integration.md).
- [x] Deliver Options, including binding editors and original settings semantics.
  [Desktop accepted2026-10-31](analysis/options-flow.md).
- [x] Deliver profile management and persistence using isolated writable copies.
  [Desktop accepted2026-10-31](analysis/profiles-flow.md#desktop-acceptance). Gameplay saves follow later.
- [x] Finish Main's live profile, last-save, date/time and version labels and eligibility.
  [Desktop accepted2026-10-31](analysis/main-menu-state.md#desktop-acceptance).
- [ ] Deliver single-player setup through a documented game-controller handoff.
  [Full pre-mission scope/research](analysis/singleplayer-pipeline.md); validate payload, report game entry unavailable.
  Team-selection checkpoint accepted2026-11-01; loadouts implemented, completely
  untested until accessible. Full flow open; multiplayer is next per user.
- [ ] Deliver multiplayer UI/service contracts; distinguish working local/network paths
  from unavailable legacy services.
- [ ] Deliver in-game menu variants and transitions back to the front end.
- [ ] Validate shared dialog callers, reply/context lifetimes and cancellation in every flow.
  [Caller acceptance](analysis/menu-family-delivery.md#shared-dialogs).
- [ ] Finish all nine Main actions and verify complete menu traversal and returns.
  [Main traversal boundary](analysis/menu-family-delivery.md#main-menu).

For each flow: evidence → readable source → build/audit → integrate → user desktop
validation → tracker update → commit. Never enable dead actions or count placeholder
service responses as completed flows.

The linked family checklists define concrete flow boundaries and validation levels;
Credits, Cinematics, front-end Options and runtime dialog integration are accepted; 22 delivery items remain open. Native fixtures establish UI/payload/ownership
contracts, separately from actual desktop and real game/network handoff acceptance.

## 4. Finish fidelity and replace remaining scaffolding

- [ ] Verify layout, stacking, clipping, fonts/localization and resolutions.
- [ ] Verify held/repeat input, eligibility, focus, wheel/drag and modal cancellation.
- [ ] Verify sound/music, preferences, failures and deterministic cleanup.
- [ ] Audit host approximations and delegated engine/service calls against evidence.
- [ ] Run full menu traversal; record unavailable environments/services explicitly.

## Evidence and validation rules

- Prefer large coherent batches; keep checklist entries short and link detailed evidence.
- Fingerprint original client/engine evidence; keep proprietary assets and exports ignored.
- Preserve original files. No original-game/DLL/Proton execution by the agent.
- Run strict i386 builds, relevant existing checks and temporary focused audits.
  No permanent new unit tests under the project agreement.
- Interactive commits require user desktop confirmation; no pushes without instruction.
- Track functions separately in [README](README.md) and the
  [ledger](analysis/function-status.json); frozen 18,783 is not menu-delivery progress.
