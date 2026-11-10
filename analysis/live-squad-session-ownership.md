# Live squad/session ownership — P2.2 integration advisory, 2026-11-10

Targeted P24-F05 follow-through; P2.2 remains active. No preparation UI, equipment
mask, stock/refund, voice producer, reconciliation, save or result implementation
was added. Source loading remains declined and no desktop acceptance is claimed.

The read-only advisory source was branch `research/future-dependencies` at
`433228194815b375e8d524ab50413c748b06fc47`, specifically its
`analysis/future-dependencies/{HANDOFF.md,p2.4-game-handoff.md,p2.4-squad-equipment.md,
evidence/p24-review.json}`. That research reviewed main at dcbef57. It was read
with `git show`; no merge or research-worktree changes were made.

Original008AE910/application+14 owns the mutable live squad.006989B0 rereads
its ascending records and00611C90/006A2C90 mutate the same soldier, including
carried-stack splitting before Human creation. Request copying is useful for
owned mission/handoff metadata, but cannot become a second live authority.

The actual persistent source owner is `StartupSession::pre_mission().squad()`.
It survives preparation menu destruction. `SessionOwner` now REQUIRES that
mutable vector as a constructor borrower; it has no request-owned or independent
fallback. GameApplicationState passes that exact owner, and its `live_squad()`
receiver delegates to SessionOwner's checked mutable accessor. Session destruction
never copies, clears, refunds or transfers those records. Spawn/Players/HUD and
eventual extraction must borrow this accessor; `request().preparation.squad`
remains immutable handoff metadata. No transfer-back synchronization is needed
because live mutations already affect the persistent application owner.

This fixes the current ownership contract before constructing the containing
session. It does not claim the still-uncalled activation/spawn graph is integrated.
The actual HumanStore/profile B8, Items/catalogue, actor identities, Players and
SpawnPointA8 lifetimes are unchanged. Closed session access rejects; Startup's
preparation flow must outlive all session/Human/Items borrowers as required by
the existing application lifetime.

Focused native ASan/UBSan/LSan checks use the actual SessionOwner/request copy
and actual `split_spawn_stacks`: application/owner record addresses match,
count3 becomes three count1 carried records with original/−1 positions, and
identity/voice changes remain in that same vector after teardown, failed
construction and repeated construction. The request snapshot stays unchanged.
HUD/scene factories are explicit construction-boundary doubles. This establishes
alias/lifetime behavior, not desktop spawning or result extraction.

P24-F01 native48 speech producer versus native40 specialization, F02 secondary
stock refund target, F03 spare-record reuse and F06 conditional nonlive restoration
remain P2.4 follow-ups from that revision. No new random fallback was added.
F04 confirms working mask1/head16 and mask2/uniform15; those are unchanged.
