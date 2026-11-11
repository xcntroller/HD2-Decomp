# Session handoff — 2026-11-11

Continue the first unchecked P2.2 checkbox in [plan.md](plan.md) through a real
mission frame. Follow [AGENTS.md](AGENTS.md). Use larger verified source
checkpoints and keep implementing after each dependency. Keep heavy builds and
Ghidra exports sequential. Compile/link Windows fixtures only. Never run the
original game, DLLs or Proton. Use ignored durable WORK for temporary fixtures
and evidence. The user performs desktop acceptance. No push.

## Latest result

[Human action completion, reload and containing module frames](analysis/human-frame-source.md#action-completion-reload-and-containing-module-frames)
and [verification receipt](analysis/checkpoints/p2.2-human-actions-and-module-frames-20261111.json).
00588010 now borrows actual runtime active20, ExplosivePlace94 and HumanEC.
00584E30 dispatches E0 completion, Door unlock, disarm and unavailability.
007D4850 uses actual ammunition stacks, item retirement and SOUND. 0058BAF0
samples shdum and original AI spread before required item virtual24. The
containing facade composes actual physics, Scene2/Human and camera modules.
Native sanitizer, actual-host i386, launcher link and 27 tracker checks pass.
The tally is 34 complete and 2,260 partial entries (2,255 client and five engine)
across 18,783 frozen functions. Both P2.2 checkboxes remain open. Application
loading still declines and no mission desktop test has occurred.

[Shared Scene2 movement](analysis/human-scene-movement.md) and
[detour verification receipt](analysis/checkpoints/p2.2-shared-scene-movement-detours-20261111.json).
005647C0/008AADD0 supplies the eight substeps, B0 path detours and steering
through actual active20. It is wired into the containing module facade.

## Continue P2.2

- Continue Human DD0 control 00787270/00787D50, other B0 families, positive
  00584E30 effects and queued 00577B30. Guard cover/door, 0057CC50/007861A0,
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
