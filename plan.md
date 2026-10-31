# Entire menu reconstruction plan

Updated: 2026-10-31. [Handoff](HANDOFF.md) records the latest work and next steps;
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

Next: shared menu foundation input/focus callers and initial-focus policy.
[Keyboard recovery](analysis/keyboard-preprocessing.md).
Full registry/setup bootstrap and binary integrity VM remain outside the checkpoint.

## Current position

| Area | Verified | Remaining |
| --- | --- | --- |
| Original controller launch | Four movies/subtitles → Main/Exit and bounded failure/shutdown accepted | Other menu flows; broader failures remain excluded |
| Menu visuals | Original artwork, cursor, text and button hover | Dynamic labels, localization, fidelity |
| Exit/confirmation | Pointer, Enter/Space, Escape and modal cancellation | Other actions/dialogs |
| Shared controls/screens | State, messages, factory, stack and nested diagnostic | Screen callbacks, dynamic lists, original ABI |
| Options widgets | Native state/input/observers audited | Engine visuals, actual screen integration |
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
- [ ] Complete character targets, focus-entry/navigation callers and initial-focus
  policy. Polling is confirmed; preprocessing/text FIFO are **source-only**.
  [Keyboard](analysis/keyboard-preprocessing.md).
- [ ] Resolve exact queue/update timing, drag/capture and focus-loss behavior.
- [ ] Integrate original settings/resource caches in place of host defaults.
- [ ] Build options widget internals and connect recovered observers/settings.
  Native slider/selector/toggle routes are audited; no new screen is enabled.

## 2. Establish the complete screen and dependency inventory

- [x] Inventory 14 static registrations and 11 augmentation builders. **Source-only.**
  [Screen inventory](analysis/menu-screens.md), [metadata](analysis/menu-screens.json).
- [ ] Enumerate remaining derived/dynamic/shared/in-game screens and reachable routes.
- [ ] Record IDs, assets, controls, captions, initialization, dependencies and return paths.
- [ ] Resolve duplicate/placeholder definitions before counting separate screens.
- [ ] Split each family below into concrete delivery checklists; identify running-game
  requirements and ways to validate their UI contracts independently.

| Family | Required flow | Current status |
| --- | --- | --- |
| Main menu | Nine actions, profile/save/date/version labels, eligibility | Exit only |
| Shared dialogs | Yes/No/OK, prompts, text/password, errors and cancellation | Quit Yes/No; slot-zero diagnostic |
| Credits/cinematics | Controller, scrolling/list/playback and return | Entry routes identified |
| Options | Nested pages, widgets, apply/cancel/persistence | Definitions/observers inventoried |
| Profiles | List/select/create/rename/delete and validation | Builder/dependencies inventoried |
| Saves/loading | Metadata, previews, selection, errors and handoff | Loading primitives recovered; UI open |
| Single player | Campaign/lone-wolf/carnage, squad/equipment/briefing | Initial definition located |
| Multiplayer | LAN/Internet browse, create/connect/lobby/errors | Static families/builders mapped |
| In-game menu | Pause/options/save/load/quit and return variants | Not inventoried |

## 3. Deliver complete flows in dependency order

- [ ] After launch, deliver credits or another low-dependency screen through entry/action/back.
- [ ] Deliver options, including binding editors and original settings semantics.
- [ ] Deliver profiles/saves using isolated writable copies for persistence validation.
- [ ] Deliver single-player setup through a documented game-controller handoff.
- [ ] Deliver multiplayer UI/service contracts; distinguish working local/network paths
  from unavailable legacy services.
- [ ] Deliver in-game menu variants and transitions back to the front end.

For each flow: evidence → readable source → build/audit → integrate → user desktop
validation → tracker update → commit. Never enable dead actions or count placeholder
service responses as completed flows.

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
