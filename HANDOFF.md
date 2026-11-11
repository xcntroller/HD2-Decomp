# Session handoff — 2026-11-11

Continue the first unchecked P2.2 checkbox in [plan.md](plan.md) through a real
mission frame. Follow [AGENTS.md](AGENTS.md). Use larger verified source
checkpoints and keep implementing after each dependency. Keep heavy builds and
Ghidra exports sequential. Compile/link Windows fixtures only. Never run the
original game, DLLs or Proton. Use ignored durable WORK for temporary fixtures
and evidence. The user performs desktop acceptance. No push.

## Latest result

[Human frame dispatch](analysis/human-frame-source.md#guard-and-area-frame-dispatch)
and [verification receipt](analysis/checkpoints/p2.2-human-ai-frame-dispatch-20261111.json).
Guard 00548170 and Area 00559DE0 now dispatch through the containing Human owner
with shared input buffers, actual action leases, typed hostile scoring and B0
movement/item output. The native tests cover a live hostile through Guard aim and
loss, Area handover, movement/E0, kind 9 output and pool lifetime. Strict i386
host instantiation, launcher link and all 27 tracker tests pass. The tally is
34 complete and 2,217 partial entries (2,212 client and five engine) across
18,783 frozen functions. Both P2.2 checkboxes remain open. Application loading
still declines and no mission desktop test has occurred.

## Continue P2.2

- Finish positive Human B0/Guard/Area/DD0 workers as reached. Target action
  completion 00584E30, weapon execution 0058BAF0, populated 00577B30 queues,
  Guard cover/door and armed alignment, specialized navigation 0057CC50/007861A0,
  Human Door kind 6 and floor-list 00581E30 still require actual receivers.
  Original files are in WORK directories `frame-human-positive-{guard,aim}`,
  `frame-human-guard-area-authority` and `frame-human-ai-path-receivers`.
- Continue the distinct shared Scene2 movement receiver 005647C0/008AADD0.
  The caller already passes actual active20. Use the same Human+2C4 and Car
  owners, captured forward gather, reverse prepare/constraints, eight substeps
  and ordered path/steering publication. Its direct workers are exported to
  WORK `frame-shared-movement`. B0 navigation and steering are documented in
  [Human frame source](analysis/human-frame-source.md#guard-and-area-waits-and-navigation).
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
