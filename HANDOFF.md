# P22-LIFE handoff — 2026-11-11

Agent #3, `feature/p22-life`, worktree `HD2-Decomp-p22-life`.
Starting and latest read-only fetched `origin/master`:
`49ff7dc12710cd645dc2224f9879b829cf055308`. No merge/rebase/push or modification
of Agent #1's master or Agent #2's UI worktree. Research/p22 and UI d01face were
read through Git. Application loading remains disabled.

[Exact LIFE checklist](analysis/p22-life-plan.md) ·
[Lifecycle/API and original contracts](analysis/p22-life-implementation.md) ·
[Verification receipt](analysis/checkpoints/p22-life-20261111.json).

**P22-LIFE source boundary is closed for the reconstructed offline graph; external
positive dependencies and application/desktop acceptance remain open.**

The concrete `GameSessionLifecycle<Controller>` owns the existing actual loading
World, activation, GameBlastWaves and UI adapters over SAME SessionOwner/request/
mutable squad/process/Scene. It tracks entered/completed prefixes, preserves the
primary error, records secondary cleanup/graphics failures, and separates finish,
file unload and close. Real status1 finish precedes the file loaded14 guard.
Mandatory retirement failures stop dependent teardown and retain the graph for
explicit retry. Callback/page loans, partial VM contexts, actor prefixes,
frameEC/item/model/Physics users, camera/effects/particle loans and collision
sweep/buckets retire before their owners/parent reset. Native successful finish
and file publication order is retained. Source guards and exclusions are explicit.

Original address correction:006987E0 is `return 1`, not destruction. Actual
wrapper/body are00698810/00698830. No new destructor or delegated engine credit.
Five existing partial rows are extended; frozen18,783, complete34 and partial2,471
are unchanged. No whole-function or desktop promotion.

Next for P22-OWN: stop/destroy frame borrowers before LIFE close; destroy LIFE
before Parent/engine reset. Supply the real controller/Startup/catalogue/suffix/
SessionActivationState and mandatory render receiver. The typed API is callable
but has NOT been installed in GameApplicationState. Preserve original status and
loaded publication; do not enable application success without the remaining
Human/GUARD/FRAME/RENDER/UI dependencies. Explicit close reports mandatory errors;
the destructor reports remaining failures and uses existing guarded RAII fallback.

External work stays separate: Agent #1's positive Human/Guard/integrity work;
Agent #2's populated roll/signal retirement, HUD/input orchestration and required
order/seat/focus producers; FRAME/RENDER scheduling and OWN activation. Reached
missing positive receivers still reject. Application loading/failure/Main-return/
re-entry and user-operated desktop acceptance remain pending. No original EXE,
DLL, Windows fixture or Proton was executed.

Verification work/logs are ignored `analysis/work/p22-life-20261111`; original
read-only exports/toolchain are in `/home/control/Documents/Projects/H&D2 Decomp 4`.
Focused native ASan/UBSan/LSan includes every observed load/activation throw,
false tails, cleanup retries,100 finish cycles, every parent-constructor allocation
failure, actual partial Script context retirement, retained frame/page/EC/bucket
lifetimes and graphics double failure. Engine/archive/clock/actor endpoints are
doubles. Strict actual i386 composition uses the real launcher controller and
links without execution. Temporary audits are fingerprinted and removed after
verification; no permanent tests. No push or merge is authorized.
The user authorizes commits upon completing each checkbox in
[the Human plan](analysis/p22-hum-plan.md). Accumulated motion/pose work was
committed as `bf53313`, `add9c1a` and `49ff7dc`. Continue the same P22-HUM ID.

The first Human checkbox, model presentation, is complete in source. Actual
007A7470 dispatch now runs Look, prone and attached/target/vehicle consumers,
using retained pose chains, shared control owners and actual aim/limit bindings.
Focused native sanitizers, actual World i386 geometry/command/activation
instantiation and launcher compile/link pass. The checkpoint adds29 client
partials and extends six existing rows, giving34 complete/2,500 partial. Detailed
contracts, adaptation choices and verification limits are in
[Human frame source](analysis/human-frame-source.md#model-presentation-source-checkpoint)
and [the receipt](analysis/checkpoints/p2.2-human-model-presentation-20261111.json).

Next is the reactions/damage/ragdoll checkbox at0074D510 and remaining007B0910,
including real impulse consumption and pose/contact retirement. The normal
model consumers are source-bound. Their00755500 interaction callers continue
in the later positive-action checkbox. Shared mounted-control movement sounds,
occupied updates and firing keep explicit required boundaries. Car frame
simulation still rejects populated gun controls pending its actual update
receiver. Both P2.2 checkboxes and mission desktop acceptance remain open.

The research-index docs are read. Before new exports or large-note searches use
`python3 -B /tmp/hd2-research-index/tools/research_index.py --root "$PWD" find ADDRESS...`.
Its tooling branch remains unmerged. Preserve Agent #2's unfinished P22-UI source.

## Continue P2.2

- Continue P22-HUM at the remaining action,
  reaction/geometry, B0/Guard/Area and Player item receivers. Positive ragdoll
  presentation and other007B0910 branches remain required. Existing original
  contracts and precise source progress are linked in
  [Human frames](analysis/human-frame-source.md). Do not switch blocker IDs or
  credit source-only work as integrated/desktop delivery.
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
