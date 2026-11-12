# Agent 3 initialization prompt — P22-LIFE

You are Agent #3 for H&D2-Decomp. Implement and source-close **P22-LIFE**:
partial-failure rollback, ordered session finish/file unload, and repeat-safe
ownership over the existing real game/session/World graph. Do implementation,
not only a research report.

## Workspace and coordination

Work only on `feature/p22-life` in your own worktree. At this handoff the branch
and worktree already exist at:

`/home/control/Documents/Projects/HD2-Decomp-p22-life`

They were observed at `49ff7dc12710cd645dc2224f9879b829cf055308`, also the latest
accessible `origin/master` on 2026-11-11. Inspect branch, status, instructions and
history first. Preserve existing changes; do not reset or recreate an existing
worktree. If the worktree/branch is absent, create an isolated implementation
branch from the latest available `origin/master`, without modifying references
or another agent's checkout. Record the exact master SHA you actually inspect.

Agent #1 owns P22-HUM and then P22-GUARD on master. Agent #2 owns P22-UI on
`feature/p22-ui`, currently paused at source checkpoint `d01face`. Do not modify
either active worktree. Periodically fetch and inspect newer origin/master
read-only; do not repeatedly merge/rebase or assume another agent's uncommitted
source is available. Do not push or merge without explicit user approval.

Use `research/p22` read-only for requirements/dependency contracts, and latest
verified master for actual implementation. Research implementation observations
can be stale. In particular, do not recreate the old “missing008AED48 owner”:
the actual blast-wave process/finalizer already exists. Confirm current bindings.

## Read and plan first

Read `AGENTS.md`, `HANDOFF.md`, current P2.2 milestone in `plan.md`, and relevant
current checkpoint receipts. Through read-only Git, read research/p22:

- `analysis/p2.2-critical-path/blocker-matrix.md`
- `analysis/p2.2-critical-path/implementation-order.md`
- `analysis/p2.2-critical-path/reverse-dependencies.md`
- Relevant `evidence/blockers.json` and lifetime/ownership evidence.

Read current master contracts/source for containing World, mission loading,
activation, finish, module lifetimes and blast processing, especially:

- `analysis/containing-world-loading.md`
- `analysis/mission-loading-integration.md`
- `analysis/session-activation-integration.md`
- `analysis/session-activation-loading.md`
- `analysis/module-loading-lifetimes.md`
- `analysis/blast-wave-process.md`
- `recovered/host/game_owner.h` (SessionOwner/GameApplicationState)
- `recovered/host/game_world_loading.h`
- `recovered/host/game_mission_loading.h`
- `recovered/host/game_session_activation.h`
- `recovered/game/session_activation.h`, `session_finish.h`
- Actual module, Human/VM, collision, camera/FPV/HUD, callback and blast owners.

Inspect relevant Agent #2 ownership contracts read-only using
`git show feature/p22-ui:analysis/p22-ui-plan.md` and
`git show feature/p22-ui:analysis/p22-ui-implementation.md`. They are integration
contracts, not authorization to copy its ledger or assume its unmerged source
exists on your branch.

Create `analysis/p22-life-plan.md` early, with checkboxes describing exactly what
remains after inspecting current source. Distinguish completed source, missing
LIFE-owned implementation, required external dependencies, later application
execution and desktop acceptance. Link it from your handoff/main-plan note.
Keep it current; complete whole checkboxes with focused verification. If a
concrete obstacle prevents creating the plan, explain it and continue useful
independent work rather than silently omitting it.

Read the new research-index documentation at
`/tmp/hd2-research-index/tools/research-index.md` (tooling/research-index branch).
Use it before new Ghidra exports or large research searches, for example:

`python3 -B /tmp/hd2-research-index/tools/research_index.py --root '/home/control/Documents/Projects/H&D2 Decomp 4' find 00698F80 0069A3D0 006987E0 --limit 2`

It is a locator, not proof of implementation/equivalence. Verify fingerprints,
source revision and ABI. Do not merge/copy tooling merely to use it.

## Implement the actual lifecycle boundary

Confirm original contracts for **00698F80 finish**, **0069A3D0 file unload** and
**006987E0 destruction**, together with loading006CA780 and activation006989B0.
Recover important behavior from original evidence before coding.

1. Build a concrete, callable lifecycle composition over SAME SessionOwner,
   GameMissionLoadingSession/World, accepted request, mutable live squad,
   Players/Human/Items/VM, HUD/FPV/camera/text/audio, process blast and existing
   engine Scene. Reuse their real cleanup operations; allocate no duplicate
   persistent owners or competing readiness/clock/roster state.
2. Record exact construction/loading/activation completion boundaries and original
   failure tails. Track completed prefixes accurately. An early failure may
   require the cold module loading tail before a collision tree exists. Do not
   call the fully activated finish path indiscriminately on a partial prefix.
3. Implement rollback for constructor, load and activation failure, false returns
   and exceptions. Preserve original error propagation while still retiring
   required completed owners. No successful empty cleanup callback or blanket
   exception swallowing may substitute for a missing mandatory receiver.
4. Preserve actual finish order: supported live-squad snapshot before Human
   removal; FPV/light restoration; reverse presentation modules; dubbing;
   reverse authority modules; HUD; collision retirement and process/resource
   tails. Verify details against actual source/original evidence rather than
   assuming this summary is the complete contract.
5. Bind `GameSessionTransientFinish` to the actual existing process blast owner
   (`006DF390(0,true)`), distinct from proximity008AED54. Preserve retained frame/
   actor lifetimes and definition/emitter/face/navigation cleanup order.
6. Preserve finish versus file-unload versus destructor semantics. Handle
   status1's required finish path before unload; retain original status/loaded14
   publication points. Retire actor/VM/module/collision users and buckets/sweep
   links, frameEC/object references, borrowed movement paths, camera targets,
   UI callbacks/pages and process borrowers before their actual owners or the
   parent engine scene are reset. Keep process overlay versus mission HUD fonts
   and their independent lifetimes intact.
7. Make close/rollback/repeat behavior coherent after each supported prefix,
   including already-finished/already-unloaded states. Implement original
   idempotence where present; use explicit source lifetime guards elsewhere,
   documenting differences instead of inventing native behavior.
8. Provide the real typed lifecycle interface needed by P22-OWN. Bind existing
   owners concretely now; do not deliver only declarations compiling against
   missing methods. Small application shutdown/ownership seams may be necessary:
   minimize/document them and check latest master before handoff. **Do not
   enable successful application loading/activation or take over P22-OWN**, the
   full frame coordinator, rendering, Human simulation or integrity producers.

Preserve Agent #1's Human B0/DD0/Guard/Area/input/combat work. Do not implement
those mechanics to satisfy lifecycle tests. If a genuine producer is missing,
identify its exact owner/interface and affected prefix; keep failure explicit.
UI ownership changes must remain compatible with Agent #2's real-owner boundary.

## Verify and close honestly

Use existing project conventions. Keep heavy work sequential. Run focused
native ASan/UBSan/LSan failure-injection/lifetime audits, strict i386 compilation
of the actual concrete lifecycle composition, and existing application link
verification. Cover each completed prefix, false/throw cleanup, normal finish
and unload, repeat cycles, SAME World identity, callback/actor/frame/page
borrowers, collision detach-before-tree-reset and exact cleanup ordering.
Distinguish native engine doubles from actual engine/desktop execution. Do not
run original EXEs/DLLs, Windows fixtures or Proton. Fingerprint then remove
temporary audits; do not add permanent heavyweight tests.

Update the LIFE plan, detailed contract, checkpoint receipt, handoff and frozen
function ledger only as applicable, after verified implementation checkpoints.
Only actual implemented bodies in the frozen inventory earn partial/complete
claims; delegated engine calls and research alone earn no credit. Keep handoff
to latest result and next work. Commit verified subsystem/whole-checkbox
checkpoints on `feature/p22-life`; no push or merge.

Source closure requires actual LIFE-owned mandatory cleanup/rollback receivers,
positive behavior and real owner binding, not compile-only success. Report
remaining Human/UI/FRAME/RENDER/OWN dependencies precisely. Application execution
and user-operated desktop failure/Main-return/re-entry tests may remain pending,
but must be separately listed. Do not claim a rendered mission frame, desktop
acceptance or source closure while LIFE-owned work remains unimplemented.
