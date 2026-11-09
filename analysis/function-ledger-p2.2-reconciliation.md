# P2.2 source ledger reconciliation — 2026-11-09

The prior ledger stopped at unloaded game-owner integration (`f9674b3`). The
subsequent loading-source checkpoints through `f72ca56` were committed without
registering their implemented function subsets. This update reconciles those
sources with the **unchanged frozen inventory**, separately from delivery.

The reconciliation checkpoint brought the ledger to **34 complete client functions and 1,440 partial functions**
(1,435 client/five engine), against 18,783 frozen entries. This adds **881 unique
client partials**, extends six existing partial rows, and promotes **zero**
functions to complete. The complete percentage remains **0.18%**. It measures
whole original bodies, not source size, playability or time remaining.

[Explicit body/source selections](function-ledger-p2.2-reconciliation.json) record
each included original entry, source, supported subset and contract. Shared
entries such as007D8B40 and00610200 are merged into a single ledger row. Coverage
is deliberately partial: native typed borrowers/STL/RAII differ from original
object/container/allocator/SEH ABI; wider runtime branches and required service
consumers remain excluded. Pure tables/data receive no independent function
credit. Small source wrappers count only their actual implemented policy, never
the delegated callee body.

The mapping records33 source entry selections absent from the frozen inventory
or classified as thunks, and10 empty/vtable-only bodies without new credit.
At that checkpoint it also lists141 referenced addresses without new registration.
These are dependency/caller/library/diagnostic references or lack a sufficiently
explicit body mapping in this update. An address comment is not proof of an
implemented body. None of those references becomes a completion claim. No new
engine entries or changed denominator are introduced.

Included source covers loading presentation/text, Scene2 readers and resource
owners, common/derived actors, item/collision/rigid-body and vehicle loading,
Human geometry/control/equipment/state/input/perception, first-person source,
HUD, Sight, proximity and sound. The linked subsystem contracts retain specific
branch exclusions. In particular00427730 VM compilation/interpreting,
00530D40/00530A40 active weapon reselection,007D9DE0 containing swap, live mission
objectives and00469300 first-camera simulation received no new body registration
in this historical reconciliation
merely because the callers/interfaces mention them.

[Saved verification](checkpoints/p2.2-20261109.json) records strict i386 app/startup
compile/link, all250 new-header syntax compilation,64 relevant Python tests,
existing native menu text checks and focused weapon transfer ASan/UBSan.
Subsystem notes retain earlier focused receipts. These checks support the
source subsets; they do not certify whole-body equivalence. The application
still declines mission loading, the P2.2 checkbox remains open and no new mission
first-frame/desktop acceptance is claimed.

Validation uses the existing `function_progress.py --render/--check` and
`test_function_progress.py` suite. Original fingerprint, inventory structure,
uniqueness, thunk exclusion and repository evidence membership remain enforced.
Historical audit/queue totals retain their checkpoint meaning; the postponed
completion queue is not expanded or marked exhausted by this ledger update.

Subsequent verified [weapon swap](human-equipment-loading.md#same-human-weapon-exchange--2026-11-09)
and [active reselection](human-state-loading.md#active-weapon-reselection--2026-11-09)
checkpoints register eight and four additional partial bodies respectively. The
current ledger is34 complete/1,452 partial/18,783 frozen. The JSON mapping here
remains the original reconciliation snapshot; later body claims live in the
status ledger and those checkpoint contracts. No whole-body promotions follow.
