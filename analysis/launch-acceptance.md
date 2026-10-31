# Bounded launch acceptance matrix

Milestone0 items1–5 are complete within the English800x600, standalone ordinary
opening → compiled Main/Exit checkpoint. Shutdown implementation was committed
as `3f976d4` after user desktop acceptance. This final checkpoint consolidates
existing accepted results; it adds no runtime behavior or original-code research.
It does not claim the complete original executable, setup bootstrap or all menus.

## Independent target and build identity

`recovered/build/application_launch.exe` is the independently compiled client
entry point. It reconstructs startup/controller/states and calls the original
LS3DF/rw_data/IJoy engine services; it does not run the original game executable
or call original client function addresses. The older engine-window preview and
startup probe remain separate diagnostics. The original DLLs are delegated
services, not reconstructed engine implementations.

Accepted shutdown-checkpoint application SHA256:
`3d2258a4ee3f95c2da0ed72caf03072a3c1e9ea4d2d95eb78ca01ae6b4e072ac`.

The [menu timing/capture build](menu-update.md) has a different
hash and is separately user desktop-confirmed2026-10-31. The subsequently staged
[settings/resource-cache build](menu-caches.md) is separately user desktop-confirmed
2026-10-31, with archived841-frame state-exit/result0 logs.
The identities and checks below describe
this historical launch checkpoint.

The strict i386 targets application-launch/startup-probe/engine-window are up to
date against committed source. `python3 -B tools/run_application_launch.py
--stage-only` revalidated staging, preserving the same application hash and
existing writable staged config/profiles. No agent Windows/DLL/Proton execution.
No rebuild or interactive change is introduced by this acceptance document.

## Accepted outcomes and provenance

This is cumulative acceptance across the named implementation checkpoints, not
a claim that every row was rerun in one session on the final binary. Earlier
logs are overwritten by later desktop runs unless explicitly preserved. The
user's confirmations and directly inspected artifacts are distinguished below.

| Outcome | Accepted checkpoint | Evidence and limits |
| --- | --- | --- |
| Real startup resources, data/profile/settings/input/localization | [Startup](startup-integration.md), commit47e768d, 2026-10-30 | User confirmed the startup probe. [Controller](controller-integration.md) subsequently confirmed integrated63/63 input words and preparation before movies/menu. |
| Original outer frame and four-movie→menu state progression | [Controller](controller-integration.md), commitc0e7346, 2026-10-30 | User confirmed movies/Main/Exit, pointer/keyboard, cancellation and focus return; contract records the then-retained normal log and tested build. |
| Ordinary playback, music, subtitles and movie cleanup | [Playback](playback-completion.md), commitcc8b528, 2026-10-31 | User confirmed normal playback/captions. Contract records four caption activations/submissions/clears and normal Exit result0 at11,306 frames on playback build0a2cfd32…. Separate S audio cues are native-audited; installed English Intro has none. |
| Escape skips, ordinary Space and focus return | [Playback](playback-completion.md), 2026-10-31 | Included in the user's accepted normal playback run. Special outro/credits input is excluded. |
| Missing Logo1 and Intro progress to the menu | [Playback](playback-completion.md), 2026-10-31 | User explicitly confirmed the requested omitted-movie run. Staged files restored. This acceptance is on the playback checkpoint, not a newly executed final-build test. |
| Normal Main/Exit, movie/menu window close and frame-limit exit | [Shutdown](shutdown-delivery.md), commit3f976d4, 2026-10-31 | User explicitly confirmed these requested runs on the current application hash. Their per-run logs were overwritten by the final fatal test; do not recreate them or claim retained traces. |
| Genuine fatal loader path and partial startup cleanup | [Shutdown](shutdown-delivery.md#desktop-acceptance--2026-10-31), current hash | Retained direct log: rejected-root2, game-roots0, vertex5, prepare0; no profile/movie continuation; hook/control/consumer cleanup, then3D/sound/graphics/archives, expected result1. |
| Window persistence and fixture restoration | [Shutdown](shutdown-delivery.md#desktop-acceptance--2026-10-31), current hash | Persisted window.cfg is8 bytes, left47/top21. Temporary GameData override and invalid-data/omitted-movie backups absent. Saved position does not establish startup restore. |
| Independent client artifact and approved commit | This checkpoint and3f976d4 | Current built/staged identities match; runtime change was committed only after the user's desktop acceptance and explicit commit request. |

Final fatal-run native/terminal/Wine logs, window.cfg and their manifest are
preserved under ignored `analysis/work/shutdown-acceptance-20261031`. Each saved
file was rechecked against its manifest hash before completing this matrix.
The manifest also records the current application hash and explicit user
confirmation of the other runs. Earlier source/asset/audit contracts remain
linked above; absence of an old physical log is not new direct evidence.

## Validation and boundaries

The implementation sweeps passed strict i386 builds, temporary native sanitizer
and staging audits, the existing native suite and158 Python tests. Shutdown's
144-case acquisition/cleanup matrix, real-template state/movie/control/module/
loader checks and17 staging cases are recorded in its delivery contract; no
permanent tests were added. This final documentation checkpoint rechecks the
three target dependencies, staged hash, saved evidence/fixture restoration,
startup/playback/shutdown original evidence maps, frozen accounting and whitespace.
It does not invent a new sanitizer run or repeat desktop execution.

Original evidence still uses the fixed client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`
and engine SHA256
`12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
No function credit changes:308 partial (303client/fiveengine), zero complete,
frozen18,783. A delivered launch checkpoint does not make partial bodies complete.

Full registry/setup.exe negotiation, binary integrity VM/original object ABI,
extra original global pools/debug/edit-font/cache producers, populated network/
worker/gameplay branches, special videos, generic paragraphs/wide locales and
broader failure equivalence remain explicit exclusions. Existing reached native
owners use real resources; no successful placeholder services complete them.
Other Main actions, dynamic labels and wider menu flows remain open in plan.md.

## Next milestone

Character/focus source recovery was committed as66a1687. The shared menu
[queue/update/capture/focus-loss implementation](menu-update.md) is now built,
audited, staged and separately user desktop-confirmed2026-10-31. Its acceptance is separate
from this historical launch matrix. Keep the bounded launch checklist closed;
follow plan.md and HANDOFF.md for the current checkpoint.
