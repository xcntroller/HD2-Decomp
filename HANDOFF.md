# Session handoff — 2026-11-11

Continue the first unchecked P2.2 checkbox in [plan.md](plan.md) through a real
mission frame. Follow [AGENTS.md](AGENTS.md). Use larger verified source
checkpoints and keep implementing after each dependency. Keep heavy builds and
Ghidra exports sequential. Compile/link Windows fixtures only. Never run the
original game, DLLs or Proton. Use ignored durable WORK for temporary fixtures
and evidence. The user performs desktop acceptance. No push.

## Latest result

[Shared Scene2 movement](analysis/human-scene-movement.md) and
[verification receipt](analysis/checkpoints/p2.2-shared-scene-movement-20261111.json).
005647C0/008AADD0 now gathers actual active20, publishes Human+2C4 steering,
runs eight ordered substeps, and applies Human/Car and linked floor constraints.
Car radii come from the original 180/190 bounds in its embedded physics owner.
0056A0F0 trims blocked leading jobs in order. The positive local detour search
0056A8E0/0056C230/0056A6D0 remains required before live frame integration.
Native sanitizer, actual-host i386, launcher link and 27 tracker checks pass.
The tally is 34 complete and 2,242 partial entries (2,237 client and five engine)
across 18,783 frozen functions. Both P2.2 checkboxes remain open. Application
loading still declines and no mission desktop test has occurred.

[Prior Guard and Area frame dispatch](analysis/human-frame-source.md#guard-and-area-frame-dispatch)
was committed as 33ca2d7. The containing B0 owner dispatches actual shared input,
action leases, hostile scoring and supported movement/item output.

## Continue P2.2

- Finish positive Human B0/Guard/Area/DD0 workers as reached. Target action
  completion 00584E30, weapon execution 0058BAF0, populated 00577B30 queues,
  Guard cover/door and armed alignment, specialized navigation 0057CC50/007861A0,
  Human Door kind 6 and floor-list 00581E30 still require actual receivers.
  Original files are in WORK directories `frame-human-positive-{guard,aim}`,
  `frame-human-guard-area-authority` and `frame-human-ai-path-receivers`.
- Continue local detour construction and collision sampling 0056A8E0/0056C230,
  then original 0056A6D0 route extraction and 0056A0F0 pooled path insertion.
  WORK exports are in `frame-shared-movement-replan` and
  `frame-shared-movement-path`. Use the same process scratch and collision owners.
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
