# Understanding Hidden & Dangerous 2

Readable reconstruction of the base game, initially using its original LS3DF.dll
engine and local assets through Proton. Phase1 front-end delivery is closed;
Phase2 covers level loading and game-dependent flows. Sabre Squadron
and engine replacement are separate.

## Current build

**P2.2 is in progress: mission loading is not playable yet.** Source recovery now
covers the loading presentation, Scene2/resource readers, actor/item/collision
owners, vehicles, Human control/equipment/AI, first-person view, HUD, Sight,
sound, proximity and the script compiler/context/module receivers. These are source checkpoints; the application still
declines loading until the actual world and remaining consumers are connected.
[Current boundary and remaining work](analysis/mission-world-loading.md) |
[Source progress](analysis/loading-modules.md) |
[Guard and Area frame checks](analysis/checkpoints/p2.2-human-ai-frame-dispatch-20261111.json).
[Cold Car physics/finalization](analysis/car-runtime-loading.md) now has a verified
source composition; [cold Car actor activation](analysis/car-activation-loading.md) is also recovered.
[Cold Car callback/world binding](analysis/car-callback-world-binding.md) supplies the actual factory, Physics contact and shared solver/pool owners.
[Live squad authority](analysis/live-squad-session-ownership.md) remains with the persistent application records.
[HUD world/parent binding](analysis/hud-world-binding.md) now composes with the actual identity, objective and navigation owners.
[Containing world owners](analysis/containing-world-loading.md) now construct the actual shared Human/VM/HUD/module graph before collision loading, with checked late tree binding. [Concrete loading session](analysis/mission-loading-integration.md) now binds the real screen/stages/module tail, shared Bird/effects update and input defaults. [Concrete activation/writeback bindings](analysis/session-activation-integration.md) now share the actual request/route/squad and prepared Human lifetime; a concrete [shared blast-wave receiver](analysis/blast-wave-process.md) is now available for its ordered finish. [Process packet/camera binding](analysis/process-packet-world-binding.md) now connects source queues to the actual roster and camera owners. [Frame and render source](analysis/frame-render-source.md) now supplies native coordination policies, shared emitter/dubbing ticks, fades, a separate text overlay and volumetric draw leaves. Required finish/guard and actual frame/visibility integration remains.

The accepted bounded launch checkpoint covers: four opening movies with music and
English subtitles lead into the compiled Main/Exit menu. Pointer/keyboard input,
Escape skips, confirmation/cancellation, focus return and bounded shutdown/failure
handling are desktop-accepted. The [acceptance matrix](analysis/launch-acceptance.md)
records tested builds and distinguishes retained evidence from user confirmations.

The current build also includes the complete [Main Credits flow](analysis/credits-flow.md),
with real scrolling text/CE fonts, music and return to the retained Main owner.
It passes native sanitizer audits/builds and is **user desktop-confirmed2026-10-31**
against the requested checklist.

The [Cinematics flow](analysis/cinematics-flow.md) is now integrated and native-audited,
with real profile availability, muted preview, selected playback and retained return.
Cinematics is **user desktop-confirmed2026-10-31**, including the corrected
`.bik2` preview and `.bik` full playback. The launcher stages both
sets. Africa/Czech intro entries are unused development remnants and ignored as
expected absences; original catalog flags remain intact.

[Main front-end traversal and the Options hover/caption-click corrections](analysis/main-traversal.md#desktop-acceptance--2026-11-02)
are user-accepted2026-11-02. The new [resolution/language expansion](analysis/menu-fidelity.md)
is **user desktop-accepted2026-11-02**, including team removal and widescreen. Gameplay/save
receivers and the documented single-player/server deferrals remain open. Full
original setup/protection, object ABI, unused global caches and live network/
gameplay services are not claimed complete. Original engine calls stay delegated;
source recovery alone is not desktop delivery.

Build and run from the repository root, with the original game installed locally
under `Hidden and Dangerous 2`:

```sh
make -C recovered application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++
python3 -B tools/run_application_launch.py
```

The launcher stages isolated runtime files under `analysis/work/startup-runtime`,
preserving staged config/profiles and leaving installed assets unchanged.
Use `--stage-only` to prepare files without executing Windows code. The old
engine-window preview and startup probe remain separate diagnostics.

To build a portable Windows folder/ZIP with the current client and required local
assets/DLLs, run `python3 -B tools/package_application.py --output dist/NEW-NAME`
after compiling; existing outputs are refused. The current review artifacts are
`dist/HD2-Phase1-clean/` and `dist/HD2-Phase1-clean.zip`.
Extract the entire ZIP and double-click `HD2FrontEnd.exe`, then create/select a
profile. The package starts with clean user state and needs no Python/source
checkout at runtime. [Portable launch, dependencies and limits](docs/STANDALONE.md)
also documents Proton use. The earlier portable build is
[user-accepted2026-11-02](analysis/standalone-package.md#desktop-acceptance--2026-11-02);
the linked package record preserves its identity and staging-layout history.

The Phase1 follow-up removes development resource extracts/font aliases from
new packages and loads original archive/Text paths. Launch with `--fullscreen`
for exclusive fullscreen or `--windowed` for the default window. The revised
package and fullscreen behavior are user desktop-accepted2026-11-02.
[Current artifacts, evidence and review](analysis/resource-fullscreen.md).

The later [team/browser selection fixes](analysis/team-browser-selection.md)
are user desktop-accepted2026-11-03. The
[first function-completion pass](analysis/function-completion-audit.md) audits
all564 existing entries and completes five button queries source-only. Static
matching reports0 exact bodies; behavior/ABI verification is recorded separately.
No new package was generated.

The earlier function-completion work includes the
[delta/scroll group](analysis/delta-scroll-completion.md),
[scalar/input batch](analysis/scalar-input-batch.md) and
[input-owner batch](analysis/input-owner-batch.md). The current ledger has34
complete client functions and 2,500 partial entries (2,495 client and five engine).
The [Physics object frame checkpoint](analysis/physics-object-frame-motion.md) supplies type0..5 motion, contact-seeded tipping, actual module2 queries, impulse/wake and ordered retirement source-only. Tree replacement/pickup and application/desktop integration remain required.
The [Sight frame checkpoint](analysis/sight-shot-frame-source.md) binds scheduling, Human range/visibility metrics and marker rays to SAME World owners, source-only. Positive Dog/attachment branches and application/desktop delivery remain required.
The [Human frame checkpoint](analysis/human-frame-source.md) covers callback order, perception, E0 commands, Guard, Area and Script dispatch, action completion, reload, AI aim and DD0 frame coordination, queued control actions, face transitions, pooled breathing/motion audio, ground-frame coordination, shared track displacement and velocity selection, ground geometry, walking collision, water/fall state and direction replay, Human contacts, shared ground projection/geometry publication, Look dispatch/posture-room/lateral collision and lazy held-model classification, Player command/input phases, weapon readiness/scope and breath state, and offline Ladder requests, Ladder setup/movement and model-geometry dispatch in source. The [shared Scene2 movement source](analysis/human-scene-movement.md) supplies floor and actor contacts and pooled local detours. The containing module facade uses these owners. Positive B0, DD0, fire and application integration remain required.
The [Human motion/pose checkpoint](analysis/checkpoints/p2.2-human-motion-pose-20261111.json) adds source arbitration, step/water/air motion, face hits, secondary clips, hand IK and weapon posture/camera receivers. The [model-presentation checkpoint](analysis/checkpoints/p2.2-human-model-presentation-20261111.json) completes the first Human-plan checkbox in source, adding Look, prone, attached/target/vehicle posing, retained pose chains and actual mounted-control aim/limits. [P22-HUM remains open](analysis/p22-hum-plan.md) for reaction/ragdoll, action, AI, Player and particle paths.
The [P2.2 reconciliation](analysis/function-ledger-p2.2-reconciliation.md) records
the earlier accumulated source subsets. Later checkpoints include Human
weapons/formation/Players, [prepared spawning](analysis/session-activation-loading.md),
[camera and local dispatch](analysis/camera-local-dispatch-loading.md),
[script compilation/context](analysis/mission-script-loading.md) and
[script module/visibility/rotation receivers](analysis/mission-script-receivers.md) and
[world sound/subtitle receivers](analysis/sound-world-loading.md), and
[shared scene/actor ownership](analysis/world-scene-ownership.md) and
[Door loading/first-update receivers](analysis/door-loading.md) and
[Pike/ExplosivePlace loading](analysis/pike-explosive-loading.md) and
[generated Fowl loading](analysis/fowl-loading.md) and
[Animation/SupplyBox loading and first update](analysis/animation-supply-loading.md) and
[StaticWeapon loading](analysis/static-weapon-loading.md).
[initial Human/VM/view controls](analysis/mission-script-initial-controls.md) add22 source partials;
[script runtime/objectives/lifecycle](analysis/mission-script-runtime-composition.md) add13
and extend the required camera look update.
[Player/first-person loading composition](analysis/first-person-loading-composition.md)
adds six source partials and real same-owner player lookups.
[HUD Human/presentation composition](analysis/hud-player-presentation-loading.md)
adds15 source partials for soldier/compass publication, getters and inventory lookup.
Source recovery remains separate from desktop integration.

Phase1 front-end delivery is closed by the user2026-11-03 within its recorded
acceptance and deferred-flow boundaries, committed as4e530f7. The
[game-owner construction and failure-return path](analysis/game-owner-integration.md)
are integrated, native-verified and user-accepted2026-11-03 for unloaded construction/error return. Actual
world loading is not delivered yet; untested loadouts and live game/server delivery
remain explicitly open.

**The exhaustive pre-Phase2 completion pass is postponed by the user2026-11-03.**
The [new Phase2 plan](analysis/phase2-delivery-plan.md) combines required function
completion with each integrated delivery batch. First targets are a real level's
first world frame and then a controllable player/mission loop, followed by remaining
preparation, pause/options, actual saves and results. Live multiplayer acceptance
stays Phase3. The [retained backlog](analysis/function-completion-queue.md) no longer
blocks Phase2; existing source and progress are preserved.

Select window/render dimensions and an installed translation per launch:

```sh
python3 -B tools/run_application_launch.py --list-languages
python3 -B tools/run_application_launch.py --width 1280 --height 720 --language Czech
```

Available translations are Czech, english, EnglishUS, french, german, italian
and spanish. Localization is user-confirmed2026-11-02; the
[team-selection/widescreen corrections](analysis/team-removal.md) are also accepted.
Font sets follow the original width thresholds. Default size is
800×600; omitting `--language` uses the original registry selection. An explicit
language override does not change the registry. Japanese/wide-font dependencies
are unavailable in this installation; see the linked fidelity contract.

Single Player is enabled. The team-selection checkpoint is **user desktop-accepted
2026-11-01**, including the final name alignment, drag depth and drop-area fixes.
The full single-player pipeline remains incomplete.

**Loadout/equipment screens are implemented but completely untested as screens
until we can access them.** Native data/ownership audits and successful builds
are separate evidence; they do not establish screen rendering or interaction.
Do not mark loadouts or the complete preparation flow desktop-tested.

The final owned mission/squad/equipment request explicitly reports **game entry
unavailable**; gameplay/save loading remain outside this menu checkpoint.
[Acceptance and remaining limits](analysis/singleplayer-pipeline.md#desktop-acceptance--team-selection-checkpoint-2026-11-01).
Loadout testing remains deferred until the actual game entry path is available.
[Plan](plan.md) and [handoff](HANDOFF.md) record the active P2.2 loading work.

[Multiplayer front-end work](analysis/multiplayer-pipeline.md#desktop-acceptance--2026-11-01) is desktop accepted2026-11-01 for reachable
browser, Create/maps/settings, filter editing, Add Server and unavailable-service
boundaries. Information/password/console and the distinct type8 connection owner
are source-integrated. Real DirectPlay LAN discovery is wired; native audits and
strict i386 builds pass. All UI requiring
a ready server, including server information, remains **untested**. The old server
implementation appears broken per user testing; server reconstruction, Host/Connect,
auxiliary services and live UI verification are planned for Phase 3. Actual status9
game-entry reconstruction belongs to Phase 2, with live integration after the server.

Original menu queue/update timing, drag/capture and focus-loss behavior are
[desktop-confirmed](analysis/menu-update.md) for Main/Exit; unused widget routes
remain source-only. The settings/resource cache integration is also
[desktop-confirmed](analysis/menu-caches.md) for the bounded Main/Exit flow. [plan.md](plan.md) is the single menu delivery checklist;
[HANDOFF.md](HANDOFF.md) records the latest result and next work.

The [front-end Options flow](analysis/options-flow.md) is integrated with all
pages, binding capture/lists, real settings, apply/Back and isolated persistence.
Native suites and sanitizer audits pass. Front-end Options is **user desktop-accepted
2026-10-31**, including the corrected binding layout. Shared runtime dialog
integration is accepted through its genuine Options caller; other dialog callers
and in-game Options remain open. Gamma is intentionally unavailable in the
windowed launcher.

[Profile management](analysis/profiles-flow.md) is integrated with real selection,
create/rename/delete, difficulty setup and isolated persistence. User desktop acceptance is recorded2026-10-31, including the corrected
selection background, hover and same-profile double-click behavior. Gameplay saves remain
with later controller/in-game deliveries.

[Main live labels and eligibility](analysis/main-menu-state.md) are now integrated:
original version, profile/save empty states, stored save captions, no-profile
CREATE PROFILE/Exit gating and the original title offset. Native checks pass;
this Main update is user desktop-accepted2026-10-31. Actual
Resume/gameplay and other unavailable destinations retain their later gates.

The [screen-and-route census](analysis/menu-route-census.md) now covers eight
concrete frontend owners, conditional registries, shared dialogs, embedded pages
and in-game/controller routes. Its thirty numbered declarations are candidates,
not a count of distinct delivered screens. The [control/dependency contracts](analysis/menu-contracts.md)
now record IDs, assets, captions/styles, initialization and return rules across
512 control sites. [Identity/placeholder resolution](analysis/menu-definition-resolution.md)
preserves all 30 declarations and IDs: 28 populated owner-scoped roles and two
reserved empty hero slots. Shared artwork/code, embedded content and campaign
variants are classified separately. The [family delivery checklists](analysis/menu-family-delivery.md)
define 29 delivery items across all nine families (22 remain open), including
independent UI audits and real game/network requirements. Inventory/planning is
**source-only**; flow delivery and original-function credit are tracked separately.

The [shared runtime dialog receiver](analysis/runtime-dialog-integration.md) is
now integrated and native-audited, including the genuine Options unassigned-command
caller and category/row reply. This integration is desktop-accepted through Options;
other callers remain open. No diagnostic
prompt or unsupported Main action is enabled.

## Verification and evidence

Original behavior and function addresses are recorded before implementation.
Reached services use real owners/bodies; unsupported branches remain explicit.
Existing native checks and tooling tests run with:

```sh
make -C recovered test
python3 -B -m unittest discover -s tools -p 'test_*.py'
python3 -B tools/function_progress.py --check
```

Temporary sanitizer audits and strict Windows i386 compile/link checks accompany
relevant changes. No permanent new tests are added under the current agreement.
Only user desktop runs establish interactive acceptance; the agent does not run
the original game, DLLs or Proton. See [AGENTS.md](AGENTS.md) for workflow rules.

## Original-function progress

The count below measures original function bodies, separately from menu delivery.
A working checkpoint can depend on partial bodies and delegated engine calls.

<!-- FUNCTION-PROGRESS:START -->
### Original-function reconstruction (base game only)

`[------------------------------] 0.18% complete`

**34 complete / 18,783 identified functions** (frozen, estimated baseline); **3,082 unique functions partially reconstructed**.

| Program | Counted functions | Complete | Partial |
| --- | ---: | ---: | ---: |
| Base game client | 17,294 | 34 | 3,077 |
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

## Project map

- [Plan](plan.md): remaining menu outcomes and delivery criteria.
- [Handoff](HANDOFF.md): current state and next session.
- [Research](RESEARCH.md): detailed findings, evidence and tool instructions.
- [P2.2 loading](analysis/loading-modules.md): recovered source and integration gaps.
- [Function reconciliation](analysis/function-ledger-p2.2-reconciliation.md): current source tally and exclusions.
- [Launch acceptance](analysis/launch-acceptance.md): delivered boundary and provenance.
- [Menu inventory](analysis/menu-screens.md): known screens and dependencies.
- [Control contracts](analysis/menu-contracts.md): complete source-only dependency inventory.
- [Definition identity](analysis/menu-definition-resolution.md): duplicates, reserved slots and counting rules.
- [Family checklists](analysis/menu-family-delivery.md): concrete flows, prerequisites and acceptance boundaries.
- [Readable source](recovered/) and [tools](tools/): reconstruction and analysis code.

Commercial binaries/assets, generated exports and local analysis databases stay
out of Git; see [.gitignore](.gitignore). Four generated game executables remain
in the initial local commit's history despite being untracked now; that history
still needs cleanup before publishing.
