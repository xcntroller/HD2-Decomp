# Understanding Hidden & Dangerous 2

Readable reconstruction of the base game's menu and launch path, initially using
its original LS3DF.dll engine and local assets through Proton. Gameplay, Sabre
Squadron and engine replacement are outside the current menu milestone.

## Current build

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

The accepted UI boundary is **English,800x600, Main/Exit/Credits/Cinematics**. Other menu actions,
dynamic labels and broader settings/profile/save flows remain unfinished. Full
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
windowed launcher. Profiles/saves are the next delivery milestone.

The [screen-and-route census](analysis/menu-route-census.md) now covers eight
concrete frontend owners, conditional registries, shared dialogs, embedded pages
and in-game/controller routes. Its thirty numbered declarations are candidates,
not a count of distinct delivered screens. The [control/dependency contracts](analysis/menu-contracts.md)
now record IDs, assets, captions/styles, initialization and return rules across
512 control sites. [Identity/placeholder resolution](analysis/menu-definition-resolution.md)
preserves all 30 declarations and IDs: 28 populated owner-scoped roles and two
reserved empty hero slots. Shared artwork/code, embedded content and campaign
variants are classified separately. The [family delivery checklists](analysis/menu-family-delivery.md)
define 29 delivery items across all nine families (27 remain open), including
independent UI audits and real game/network requirements. Inventory/planning is
**source-only**; flow delivery and original-function credit are tracked separately.

The [shared runtime dialog receiver](analysis/runtime-dialog-integration.md) is
now integrated and native-audited, including the genuine Options unassigned-command
caller and category/row reply. Its delivery checkbox awaits desktop acceptance. No diagnostic
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

`[------------------------------] 0.00% complete`

**0 complete / 18,783 identified functions** (frozen, estimated baseline); **415 unique functions partially reconstructed**.

| Program | Counted functions | Complete | Partial |
| --- | ---: | ---: | ---: |
| Base game client | 17,294 | 0 | 410 |
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
