# Function completion and selective matching — delivery plan

The user approves adding the completion/matching pass before Phase2. The initial
[full-ledger audit and five-function group](function-completion-audit.md) is
complete source-only2026-11-03, after desktop acceptance of the selection fixes.
The gates below remain the policy for subsequent subsystem completion work.

The user expands this policy2026-11-03: complete all functions currently feasible
before moving on. The five-function group is the first checkpoint, not the end
of the pass. Exhaust the [dependency queue](function-completion-queue.md) before
Phase2 planning/implementation. ABI/layout recovery is work to perform, not by
itself a reason to defer a function to another phase. Keep verified milestones
bounded and follow AGENTS.md's stop-after-one-milestone rule.
The user subsequently requests larger subsystem batches with a single final
audit sweep. Apply the queue's verification cadence: quick local compiles during
implementation, consolidated sanitizers/builds/regressions and accounting at the
end, and no repeated full sweep for each small function group.

## Before Phase2 implementation

1. Review the existing564 partial entries against current integrated source and
   linked original evidence. Correct stale coverage/limitations without treating
   a working screen or stale wording as proof of whole-function reconstruction.
2. Choose a small group of self-contained functions with understood dependencies
   and explicit full-body boundaries from the frozen inventory. Record the
   remaining branch, ABI, ownership and verification work for each candidate.
3. Finish and verify that group end-to-end, aiming for the first complete entries
   before world loading begins. Promote only after manual evidence review under
   [existing counting rules](function-progress.md); no predetermined quota or
   guessed date. Native safety/ABI differences must be resolved or remain explicit
   partial exclusions rather than being removed solely to improve the counter.
4. Establish a repeatable static matching workflow on that small group: confirm
   original function boundaries/prototypes and investigate compiler/options,
   calling convention, object layout and relevant runtime assumptions. Compile
   candidate bodies and compare disassembly with relocations accounted for.
   Record exact matches and justified differences separately from behavioral
   verification. Do not require every function to match instructions immediately.

Original-game/DLL/Proton execution remains prohibited for the agent. Use original
static evidence, strict i386 builds, existing tests and temporary native sanitizer
audits; no permanent new tests. User desktop verification remains separate.
Assembly equality of an isolated body does not validate different callers,
resource/configuration inputs or the complete application integration.

## During Phase2 and subsequent subsystem delivery

Each bounded subsystem milestone must include closing understood original
functions, rather than indefinitely accumulating new partials. Verify complete
control flow, flags and signedness, arithmetic/rounding, state changes, call
ordering, object/virtual ABI, ownership and error/teardown behavior. Prioritize
instruction matching for loaders/parsers, save serialization, numerical routines,
state transitions and resource lifetimes.

Track remaining UI/object ABI consolidation explicitly when native containers,
RAII or host glue still replace original contracts. Full game/session/load/save
owners close as their actual Phase2 receivers and dependencies are reconstructed;
network-dependent bodies follow their Phase3 dependencies. Original engine bodies
still delegated to LS3DF close only when those bodies are reconstructed in the
separate engine-replacement work. Reconstructing a client caller does not credit
its delegated callee, and native integration glue earns no original-function credit.

## Evidence gate

A completion proposal identifies the exact original entry and fingerprint,
accounts for every branch and relevant state/data contract, links its compiled
implementation and verification, and states why no full-body exclusion remains.
For matching candidates, also record compiler/configuration and a relocation-
aware comparison. Assembly matching is an additional verification method; the
current ledger's `complete` status is not redefined as byte identity. Run the
existing ledger validator/render workflow after justified status changes.

After the initial pass the ledger has5 complete client functions and559 partial
(554 client/five engine), out of18,783 frozen entries at that checkpoint.
The expanded pass requires completing every currently feasible body and recording
concrete dependencies for the rest. It does not infer completion from native tests
or desktop acceptance alone.
