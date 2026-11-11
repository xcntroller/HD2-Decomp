# Reconstruction plan

Updated: 2026-11-03. [Handoff](HANDOFF.md) records the latest work and next steps;
[research](RESEARCH.md) and linked analysis documents hold technical detail.

## Scope and completion criteria

Rebuild the base game in stages: first the front-end menus, then actual level
loading and the game-dependent flows. Both initially use original LS3DF.dll and
local assets through Proton; Sabre Squadron and engine replacement are separate.

Checked delivery items mean integrated and desktop-tested unless explicitly
marked **source-only**. Source-only work, request validation and controlled
fixtures do not establish actual game entry, live networking or desktop delivery.

# Phase 1 — Front-end menus — closed2026-11-03

Rebuild reachable front-end screens and dialogs in the readable compiled client,
including navigation, settings, preparation data, service errors and a documented
boundary for unavailable game receivers. Real world loading, gameplay-save
receivers, post-mission results and in-game menus belong to Phase 2. A Phase 1
checkbox does not require a running game unless its own text says otherwise.

Phase1 is closed at the user's request2026-11-03, within the accepted front-end
scope and explicitly source-only items below. Remaining loadout/gameplay/save/result
and ready-server acceptance follow Phase2/3; the standalone function pass is
postponed. Closure does not promote those deferred or untested flows to delivered.

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
| Original controller launch | Opening/subtitles → delivered front end and bounded failure/shutdown accepted | Full setup/protection and broader original failure equivalence excluded |
| Menu visuals | Artwork, dynamic labels, localization, resolutions and fidelity accepted | Wide/GDI fonts and full original text ABI excluded |
| Exit/confirmation | Named Exit and reachable runtime callers/cancellation accepted | Game/server-dependent callers deferred |
| Shared controls/screens | Delivered routing, dynamic controls, stack, focus/capture and cleanup accepted | Original object ABI; deferred game/server screens |
| Options widgets | Complete front-end Options/settings/bindings accepted | In-game owner Phase2 |
| Startup dependencies | Integrated data/config/settings/input/archive/cache owners accepted | Full protection/setup and game/network owners |

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
  [Nine families / 30 delivery items](analysis/menu-family-delivery.md), [coverage](analysis/menu-family-delivery.json).

| Family | Required flow | Current status |
| --- | --- | --- |
| [Main menu](analysis/menu-family-delivery.md#main-menu) | Nine actions, profile/save/date/version labels, eligibility | Labels/eight available actions and returns accepted; Resume unavailable |
| [Shared dialogs](analysis/menu-family-delivery.md#shared-dialogs) | Yes/No/OK, prompts, text/password, errors and cancellation | Quit Yes/No accepted; runtime integration and Phase1 callers accepted; game/server-dependent callers deferred |
| [Credits/cinematics](analysis/menu-family-delivery.md#credits-and-cinematics) | Retained controller, list/playback and return | Credits and Cinematics accepted |
| [Options](analysis/menu-family-delivery.md#options) | All pages/bindings, apply/Back/persistence and owner variants | Front-end accepted; in-game owner open |
| [Profiles](analysis/menu-family-delivery.md#profiles) | List/select/create/rename/delete and validation | Startup persistence recovered; desktop accepted |
| [Saves/loading](analysis/menu-family-delivery.md#saves-and-loading) | Front-end save metadata and request/error boundaries | Stored Main caption/unavailable boundary accepted; broader metadata open, receivers Phase2 |
| [Single player](analysis/menu-family-delivery.md#single-player) | Campaign/lone-wolf/carnage, hero/team/equipment/briefing and owned request | Team checkpoint accepted; loadout screens completely untested; results: Phase 2 |
| [Multiplayer](analysis/menu-family-delivery.md#multiplayer) | Browser/create/connect/errors and owner variants | Reachable front end accepted; ready-server UI/services untested until Phase3; status9 receiver Phase2 |
| [In-game menu](analysis/menu-family-delivery.md#in-game-menu) | Pause/player/options/actions and retained-game return | Phase 2; actual running game required for delivery |

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
- [x] Deliver the bounded team-selection checkpoint.
  [User desktop acceptance](analysis/singleplayer-pipeline.md#desktop-acceptance--team-selection-checkpoint-2026-11-01).
- [x] Implement equipment/loadout screens and data flow — **source-only**.
  Screens are completely untested until accessible; this checks implementation only.
- [x] Build and native-audit the owned mission/squad/equipment handoff request — **source-only**.
  [Pipeline and limits](analysis/singleplayer-pipeline.md); the game receiver is Phase 2.
- [x] Deliver reachable multiplayer front-end UI and explicit service boundaries.
  [Desktop accepted2026-11-01](analysis/multiplayer-pipeline.md#desktop-acceptance--2026-11-01).
  All UI requiring a ready server remains untested; server/services and validation are Phase 3.
- [x] Validate front-end shared dialog callers, reply/context lifetimes and cancellation.
  [Caller acceptance](analysis/menu-family-delivery.md#shared-dialogs).
  [Desktop accepted2026-11-02](analysis/dialog-callers-verification.md#desktop-acceptance--2026-11-02).
- [x] Finish supported Main actions, front-end traversal and explicit unavailable boundaries.
  Single-player completion and verification are deferred to Phase 2.
  [Desktop accepted2026-11-02](analysis/main-traversal.md#desktop-acceptance--2026-11-02),
  including the four Options hover/caption-click corrections.

For each flow: evidence → readable source → build/audit → integrate → user desktop
validation → tracker update → commit. Never enable dead actions or count placeholder
service responses as completed flows.

The linked family checklists define flow boundaries and validation levels.
Credits, Cinematics, front-end Options, profile management, Main labels and the
bounded team checkpoint are accepted. Native fixtures establish only their
explicit UI/payload/ownership contracts.

## 4. Finish fidelity and replace remaining scaffolding

- [x] Verify layout, stacking, clipping, fonts/localization and resolutions.
  [Desktop accepted2026-11-02](analysis/menu-fidelity.md#desktop-acceptance--2026-11-02),
  including localization, team removal and widescreen corrections.
- [x] Verify held/repeat input, eligibility, focus, wheel/drag and modal cancellation.
  [Desktop accepted2026-11-02](analysis/input-fidelity.md#desktop-acceptance--2026-11-02).
- [x] Verify sound/music, preferences, failures and deterministic cleanup.
  [Desktop accepted2026-11-02](analysis/audio-preferences-fidelity.md#desktop-acceptance--2026-11-02).
- [x] Audit host approximations and delegated engine/service calls against evidence.
  **Source-only:** [71-file boundary audit](analysis/host-boundary-audit.md); policies and deferrals recorded.
- [x] Run Phase 1 menu traversal; record deferred flows and unavailable services.
  [Cumulative accepted traversal and current native walk](analysis/phase1-traversal.md); all71 original edges classified.

## 5. Close Phase 1 and prepare Phase 2

- [x] Package the current implementation as a standalone Windows executable with
  documented asset/DLL requirements and launch instructions.
  [Portable package accepted2026-11-02](analysis/standalone-package.md#desktop-acceptance--2026-11-02).
- [x] Remove redundant resource staging and deliver fullscreen launch.
  [Desktop accepted2026-11-02](analysis/resource-fullscreen.md#desktop-acceptance--2026-11-02).
- [x] Fix team slot dimming, rapid picker double-clicks and unreachable-server selection.
  [Desktop accepted2026-11-03](analysis/team-browser-selection.md#desktop-acceptance--2026-11-03).
- [x] Audit existing partial functions and complete a small bounded group.
  **Source-only:** [564-entry audit; five complete button queries](analysis/function-completion-audit.md).
- [x] Complete cursor delta queries and Credits scroll storage.
  **Source-only:** [four complete bodies; two exact instruction matches](analysis/delta-scroll-completion.md).
- [x] Complete the scalar-owner and input-foundation batch, then verify it in one sweep.
  **Source-only:** [41 reviewed;11 complete;30 further recovery tasks](analysis/scalar-input-batch.md).
- [x] Complete the input-manager and binding-configuration batch in one sweep.
  **Source-only:** [17 reviewed;14 complete;three further recovery tasks](analysis/input-owner-batch.md).
- [x] Research Phase 2 dependencies and replace its outline with a detailed,
  sectioned delivery plan, including evidence and verification gates.
  **Planning only:** [dependency-led delivery and function completion](analysis/phase2-delivery-plan.md).

The exhaustive function-completion step is **postponed by the user2026-11-03**;
its retained checkbox is in the postponed backlog below. Phase2 does not wait
for review of all530 partials. Completed source work and the ledger are preserved.

# Phase 2 — Load a real level and deliver game-dependent flows

Phase1 closed and committed as4e530f7; receiver contract verified and unloaded owner construction user-accepted2026-11-03. First targets: **one real world
frame, then a controllable player and mission loop**. The
[detailed plan](analysis/phase2-delivery-plan.md) gives original function seeds,
research gaps, dependencies, failure contracts and acceptance for every section;
the [machine index](analysis/phase2-delivery-plan.json) records their ordering.

For each delivery batch: research its complete original boundary, finish required
function work, integrate existing recoveries, then verify once. Complete bounded
full bodies where feasible; keep wider controller/loader exclusions explicitly
partial. Source-only work and desktop delivery remain distinct. General ABI
cleanup and byte matching are selective follow-up, not a new exhaustive gate.
[Function policy](analysis/function-completion-plan.md).

## 0. Confirm the first receiver and loading path — within the first implementation batch

- [x] Trace the receiver/loading boundary, select a reachable mission and resolve
  wrappers, interfaces and ownership/failures — **source-only**.
  [Tutorial contract and first owner source](analysis/game-owner-start.md).

Continue directly into section1 when this boundary is ready; section0 is targeted
preparation, not a standalone audit-only milestone.

## 1. Construct the game owner and consume the real request

- [x] Implement/integrate game/session subowners, scene/camera/callbacks and the
  owned mission/squad/equipment handoff, completing needed original functions.
  [Unloaded construction accepted2026-11-03](analysis/game-owner-integration.md#desktop-acceptance--2026-11-03); loaded camera follows in section2.
- [x] Verify repeat construction, partial-startup failure, teardown and Main return;
  publish real owner/world views and retain honest loading readiness.
  Native failure/repeat checks pass; [user accepted checkpoint2026-11-03](analysis/game-owner-integration.md#desktop-acceptance--2026-11-03), confirming the constructor log.

## 2. Load an original mission to its first world frame

- [ ] Implement required loading-screen, Scene/Scene2/object/resource stages and
  required actor/collision loading and ordered cleanup; finish needed parser/loader bodies.
  [Full boundary/source progress](analysis/mission-world-loading.md), [frame/render](analysis/frame-render-source.md), [module/actor frames](analysis/module-actor-frame-source.md), [projection process](analysis/human-projection-frame-process.md), [Physics frames](analysis/physics-object-frame-motion.md), [Sight frames](analysis/sight-shot-frame-source.md), [Human frames](analysis/human-frame-source.md), [shared movement](analysis/human-scene-movement.md). The containing module facade and ground-frame coordination are in source. Positive actor, rendering, integrity and application integration remain.
- [ ] User-verify actual mission scene/camera rendering, failure return and repeat
  loading through Python; unsupported required assets/data cannot report success.

## 3. Activate player controls and mission simulation

- [ ] Implement actor/equipment spawning, input, camera, required collision/physics,
  interactions, sound and script/objective consumers from the actual game call graph.
- [ ] Integrate needed completed input bodies and close relevant producer/lifetime
  gaps; verify a controllable soldier and genuine objective/failure, not just a viewer.

## 4. Finish preparation, loadouts and briefing against the running game

- [ ] Complete required preparation-function gaps and integrate descriptor-driven
  Campaign/Lone Wolf/Carnage/standalone requests with actual spawned squad/equipment.
- [ ] User-verify loadouts, constraints/presets, briefing and repeated Back/cancel.
  Loadouts remain COMPLETELY UNTESTED until accessible through genuine progress;
  this acceptance does not block section2's already accessible mission path.

## 5. Retain, pause and resume the game

- [ ] Complete/integrate retained-game entry, pause/player/equipment panels,
  in-game Options/dialogs and their required owner/input/audio cleanup functions.
- [ ] User-verify repeated pause/resume and modal cancellation preserve the same
  real game; wire save/network-dependent actions only with their actual receivers.

## 6. Implement real gameplay save, restore, restart and resume

- [ ] Recover/implement required metadata, serializer, reference restoration and
  controller receivers, completing relevant format/ownership functions together.
- [ ] User-verify isolated actual save/load state round trips, restart and Main
  resume, including malformed/incompatible data and failed-write/cancel behavior.

## 7. Deliver real mission and campaign results

- [ ] Complete/integrate required result generation, debrief/diary/campaign variants,
  descriptor progression and persistence functions using genuine game results.
- [ ] User-verify next/retry/Main return and failure/persistence handling; preserve
  confirmed variant/sentinel contracts and distinguish captured-result UI tests.

## 8. Prepare the shared multiplayer game receiver

- [ ] Complete/integrate required status9 preprocessing/shared-owner request and
  cleanup contracts; preserve the separate status7/8 connection owner.
- [ ] Verify the source/integration boundary with honest unavailable transport.
  Real Connect/Host, ready-server and live status9 acceptance remain Phase3;
  Phase2 does not require a Phase3 server to finish this source boundary.

## 9. Verify the integrated Phase2 delivery

- [ ] User-verify two real mission/resource paths and the complete delivered
  preparation/gameplay/pause/save/results/return chain, with the Phase3 live exclusion.
- [ ] Run one consolidated affected verification sweep; record completed functions,
  exact remaining partial branches and desktop acceptance. Backlog exhaustion is
  not an exit requirement; required unsupported delivery behaviors remain open.

# Phase 3 — Reconstruct multiplayer server and verify live networking

The user reports the old server binary implementation appears broken. Reconstruct
the server from original evidence; do not run the original server or assume it
is a usable test endpoint. Detailed ordering follows research after Phase 2.

- [ ] Recover and rebuild the server runtime and real ready-session lifecycle.
- [ ] Implement real Host/Connect and required auxiliary discovery/query services.
  Keep legacy Internet/GameSpy availability explicit; never substitute fake success.
- [ ] Desktop-test every UI behavior that requires a ready server.
  Currently **untested**: populated browser/discovery, server info/player/team/rule
  displays, responding favorites/refresh, Join/password/connection consoles and
  standalone connection screens. Verify against the rebuilt controlled server.
- [ ] Verify host/client game handoff, failure/cancel cleanup and return paths.
  Complete live acceptance of the Phase2 section8 receiver against the rebuilt server.

## Postponed standalone completion pass

- [ ] Complete all remaining partial functions whose dependencies are available.
  **Postponed by user2026-11-03; not an active-phase prerequisite.** Preserve the
  [530-entry backlog](analysis/function-completion-queue.md); bring relevant subsets
  into delivery batches. Resume a dedicated exhaustive pass only if reprioritized.

## Evidence and validation rules

- Prefer large coherent batches; keep checklist entries short and link detailed evidence.
- Fingerprint original client/engine evidence; keep proprietary assets and exports ignored.
- Preserve original files. No original-game/DLL/Proton execution by the agent.
- Run strict i386 builds, relevant existing checks and temporary focused audits.
  No permanent new unit tests under the project agreement.
- Interactive commits require user desktop confirmation; no pushes without instruction.
- Track functions separately in [README](README.md) and the
  [ledger](analysis/function-status.json); frozen 18,783 is not menu-delivery progress.
