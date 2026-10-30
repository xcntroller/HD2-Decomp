# Entire menu reconstruction plan

Updated: 2026-10-30. [Handoff](HANDOFF.md) records the latest work and next steps;
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

## 0. Rebuild the original launch path to the menu — current delivery priority

Deliver original opening movies → recovered controller transition → compiled
menu. The existing host preview is desktop-confirmed but is not this milestone.

- [x] Trace normal four-movie sequence and completion codes; desktop-confirmed.
  [Opening video](analysis/opening-video.md).
- [ ] Finish movie creation/playback/input/sound/cleanup. Core IShow playback,
  sizing, Escape and menu entry are confirmed; music/subtitles remain.
- [ ] Integrate the recovered controller startup/update/state lifetime with the
  compiled menu factory. Shared source is audited; concrete owners/services remain.
  [Controller](analysis/application-controller.md).
- [x] Recover keyboard/mouse/joystick binding production, installation and both
  default layouts. **Source-only.** [Bindings](analysis/binding-installation.md).
- [x] Recover input-manager ownership, attachment and teardown. **Source-only.**
  [Manager](analysis/input-manager.md).
- [x] Recover input setup, installed language and text loading/cache. **Source-only.**
  [Localization](analysis/startup-localization.md).
- [x] Recover joystick-list lifecycle and checked IJoy calls. **Source-only.**
  Do not add unreachable factory enumeration. [Joysticks](analysis/joystick-devices.md).
- [x] Recover archive order, fallback, keys and ownership. **Source-only.**
  [Archives](analysis/startup-archives.md).
- [x] Recover resource-directory registration, traversal and five client lists.
  **Source-only.** [Directories](analysis/resource-directories.md).
- [x] Connect input setup/manager to real graphics/IJoy methods. **Source-only.**
  [Input services](analysis/graphics-input-services.md).
- [x] Recover startup window-position restoration/persistence. **Source-only.**
  [Window position](analysis/window-position.md).
- [x] Recover original DTA chunk-file read/write lifecycle. **Source-only.**
  [Chunk files](analysis/chunk-file.md).
- [x] Recover HumanStore formats, names, overlays and portraits. **Source-only.**
  [HumanStore](analysis/human-store.md).
- [x] Recover both GameData formats/all kinds and combined data preparation.
  **Source-only.** [Application data](analysis/startup-data.md).
- [x] Recover profile directories/map, binary/legacy loading, movie flags and save
  finalization. **Source-only.** [Profiles](analysis/startup-data.md).
- [x] Recover Options.bin/defaults, settings application and profile selection.
  **Source-only.** [Settings](analysis/startup-settings.md).
- [x] Recover game.cfg owner/I/O and owning data subset; sound-movie change/fade
  helpers. **Source-only.** [Startup owners](analysis/startup-owners.md).
- [x] Recover sound catalog/parsers/cleanup and primary music selection.
  **Source-only.** [Sound catalog](analysis/sound-catalog.md).
- [ ] Recover full sound-manager registration/update/movie lifetime, sample caches
  and shared gate lifecycle.
- [ ] Recover optional control interface, remaining application/state/protection
  owners and loaded-cache consumers; complete live device/text integration.
- [ ] Build the independent launch executable and desktop-test normal playback,
  skip, missing movies, menu entry and teardown; commit after user confirmation.

## Current position

| Area | Verified | Remaining |
| --- | --- | --- |
| Movie/menu preview | Four movies, timing, keyboard/focus return, teardown | Original startup/controller integration |
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
