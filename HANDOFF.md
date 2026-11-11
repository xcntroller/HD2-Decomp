# Session handoff — 2026-11-11

Continue the first unchecked P2.2 checkbox in [plan.md](plan.md) through a real
mission frame. Follow [AGENTS.md](AGENTS.md). Use larger verified source
checkpoints and keep implementing after each dependency. Keep heavy builds and
Ghidra exports sequential. Compile/link Windows fixtures only. Never run the
original game, DLLs or Proton. Use ignored durable WORK for temporary fixtures
and evidence. The user performs desktop acceptance. No push.

## Latest result

[Animation-driven motion velocity](analysis/human-frame-source.md#animation-driven-motion-velocity)
and [verification receipt](analysis/checkpoints/p2.2-human-motion-velocity-20261111.json).
0076B670 now supplies actual shared-owner velocity/playback selection and E0
positive floor commands. 00773980 and00774D50 supply its track receivers.
All237 original table words and288 mappings passed native sanitizer checks,
as did the actual E0 forward/backward entry points with explicit doubles.
The early106 zero-speed branch uses a documented zero guard for unwritten
native XYZ. Strict actual-host i386 and launcher link passed. The tally is
34 complete and2,308 partial entries (2,303 client and five engine) across
18,783 frozen functions. Both P2.2 checkboxes remain open. Application loading
still declines and no mission desktop test has occurred.

## Continue P2.2

- Continue positive DD0 ground collision/replay/air workers, 0077DE30 look apply,
  00779B30/00755500 track/motion and remaining 007A7470 geometry workers.
  WORK `frame-human-live-model-geometry` contains foot, hand, look and
  mount/attachment originals. Climb movement still requires
  positive0074A690/00788840 and AI0057CC50. WORK
  `frame-human-ground-motion`, `frame-human-ladder-movement` and
  `frame-human-motion-producers` preserve the originals. Presentation track events4..9/13..20/22..24 still need
  positive workers, as do the camera reset and attached230 branch. WORK `frame-human-dd0-bound-workers`,
  `frame-human-dd0-animation-workers`, `frame-human-presentation-effects`,
  `frame-human-breathing-audio` and `frame-human-shot-and-control-workers`
  preserve the exact bodies. Continue other B0 families, positive 00584E30
  effects and queued 00577B30. Guard cover/door, AI0057CC50,
  floor-list 00581E30 and 0058BAF0 kind9 release remain required. For item
  virtual24 follow 007D4FD0 through the actual ShotMgr pool and 0044B910.
  Original files are in WORK `frame-human-control-updates`,
  `frame-human-shot-bullet-lifetime` and `frame-human-ai-path-receivers`.
- Type11 original 005B7320/005B7480 share the embedded rigid-body virtual24.
  WORK `frame-live-loose-items` has both exports. They cannot use module2
  Physics object's separate motion solver.
- Connect the real frame graph and its required positive modules, FPV/HUD/text,
  control and render callbacks. 00482260 calls procedural 00482E50 before the
  volume-light loop. Preserve the process texture owner, actual driver state
  restoration and startup fade. [Frame and render source](analysis/frame-render-source.md)
  and [module/actor frames](analysis/module-actor-frame-source.md).
- Integrate actual startup integrity producers 009905B0, 009F02A0, 009F0A60 and
  009995F0. Resolve their order including 0099AEB0 before publishing validated
  observations. Preserve the 896 draws in 006E9EA0. The independent advisory is
  on local `research/p22` at `52b4f27`, under `analysis/p2.2-critical-path/`.
  Start with its HANDOFF and integrity-and-rendering notes.
- Bind real [containing World](analysis/containing-world-loading.md),
  [loading](analysis/mission-loading-integration.md),
  [activation](analysis/session-activation-integration.md) and ordered cleanup
  in GameApplicationState. Keep the actual resolver and shared mutable squad.
  Retain real loaded14 and status publication boundaries. Exercise failure and
  repeat before user-operated Python acceptance. Add only bounded genuine
  execution markers per [diagnostic guidance](tools/runtime_diagnostics/INSTRUMENTATION.md).
  The existing World construction marker describes the owner only.

## Execution

WORK is `analysis/work/mission-world-20261103`. Client SHA is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Native sanitizer runtime is `.tools/hd2-sanitizer-runtime/usr/lib64`.
i386 compiler is `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Build `make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra JAVA_HOME is `/home/control/.sdkman/candidates/java/25.0.0.0-oracle`.
Use the `HD2Research` project in `/home/control/Documents/Projects/HD2ResearchProject`
with `-readOnly -noanalysis` and a fresh export directory. Check frozen addresses
first and inspect export logs for errors even on exit zero. Run tracker render,
check and tests at verified subsystem checkpoints.
