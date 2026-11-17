# H2 reactions/damage/ragdoll — source checkpoint 2026-11-12

**H2's assigned source boundary is complete.**
[Receipt](checkpoints/p2.2-human-h2-source-20261112.json).
This closes the Human reaction/damage/ragdoll segment on its actual owners.
It does not close H4/H6/H8, P22-VIS, application loading or desktop acceptance.
H2 is based on `41f766f`, with completed aggregate `201adbf` already merged;
no active H4 source was imported.

## Owned boundary and source changes

`0077DE30` now shares its exact reaction gate/final snapshot with
`consume_human_look_reaction`/`finish_human_look_frame`. The containing frame
still clears B4 and runs the Player direction prefix before the gate. Selection
that throws does not publish request44 or capture770. A non-consuming false
result resumes ordinary continuation, as before. `007A7470` shares its original clock/seed/motion-sample prefix and
reaction branch with `present_reaction_frame`; normal presentation runs after
that same prefix only when the branch is not consumed. No new original-function
credit belongs to these source helpers.

These segments make the H2 path executable independently of H4's unrelated
action handlers and H8's full frame scheduling. The native check uses actual
Human, passive AI B0/input resources, inline98 factory, E0, DD0,230, physics,
movement, action, animation tracks/tables, geometry, skeleton/joints, shared
collision buckets/exclusions and Effects owners. It does not replace selection,
retirement or geometry entry with a successful callback.

The shared Human World service header now includes its own Car target/actor
and first-person inventory dependencies. Its native compilation no longer
depends on the launcher's prior include order.

Earlier verified damage/death/inventory/policy, contact/fall/blast and
surface/SOUND bodies remain in the linked [Human source contract](human-ragdoll-source.md)
and S01–S14 receipts. The new graph verification replaces the earlier gap where
separate selection/control/geometry endpoints or an absent230 prevented the
actual H2 owner sequence from executing.

## Focused verification and limits

ASan/UBSan/LSan pass **100 runs of eight families**:

1. Real factory/owner activation, Look hit and movement-command admission,
   `0077DE30` reaction gate →`0074D510` selection/retirement →`007B0910` full-joint
   entry →`007A7470` sampling/ragdoll frame, finite point publication and teardown.
2. Retained reaction resource through actual animation-table ownership, positive
   key publication and odd-frame three-position retained-clip motion.
3. Reason3 seven-position checks trigger replacement by the actual full-joint
   effect; the old effect/contact vector is not accessed after replacement.
4. Reason2 strict expiry retires6304, refreshes shots/light and reaches the
   actual Effects model-opacity receiver.
5. Two distinct Humans/inline/control/equipment owners in the SAME grid,
   process physics, animation, event-pool and Effects lifetimes; non-null source
   entry and source query exclusion/restoration use the actual second owner.
6. A missing required bone aborts initialization, releases partial ownership,
   and permits fresh initialization of the SAME inline/factory.
7. The actual loaded-World point-query guard interrupts reaction entry AFTER
   earlier mutations. Request44/snapshot publication does not occur; exclusions
   clear, owned state remains available for teardown, and fresh entry succeeds.
8. `005A1C90` on the actual Human reaches the same Look and passive AI damage
   notification body. A later corpse hit queues real joint impulses and the
   following actual frame consumes them. The authored active-speech observation
   suppresses the separate speech-start branch; it is not a speech playback test.

All source-retained frame references, callbacks, registrations, descriptors,
pool leases and presentation references balance after teardown. The process
event pool outlives both Humans. No private-field seeding or access-control
override is used by this new audit.

The native environment supplies authored model/bone/marker/clip/grid data and
explicit engine math/refcount/OS/cache/lookup observations. Face quality is
disabled, the optional shadow cache is empty, and the sector light list is
authored empty. Unselected required receivers throw. This is not original asset,
engine/allocator/ABI/solver-equivalence verification. The registry facade borrows
the real inline/B0 instances; it is not a running `HumanLoadingComposition`,
Scene2 registry or application/World. AI decision scheduling and fresh-death
World-wide notifications are not newly runtime-verified by these eight families;
their existing source contracts and separate checks remain authoritative.

Strict i386 compilation additionally instantiates the actual nested host
World/Human/control, complete Look/control/presentation frame templates and
damage/source entry. Its53 other Runtime declarations always throw and are
compile-only, not installed successful handlers. Latest application launcher
and startup-probe compile/link pass. No Windows output, original EXE/DLL or
Proton was executed; no desktop test was performed. Temporary new audits are
fingerprinted/removed; dedicated logs/snapshots and old frozen inputs remain.

## Corrected dependency ownership

The earlier broad D0–D12 plan predates the user's H1–H8 branch split. It must
not require finishing H8 or a whole Car/network simulation before H2 source
delivery. This is a correction of ownership, not an original-state fix:

- Human damage `005A1C90`, selection `0074D510` and geometry `007B0910` do not
  call detached-part/wheel motion. Ragdoll contact forwards directly to Car
  `004F2790`; it does not call collision deformation `004F2B60→004FF8B0`.
- Blast→Human delivery and Human damage reception are H2 interfaces. Separate
  Car damage/destruction/debris, continuing occupied/non7 Car frames and their
  upstream producers remain containing-world integration cases.
- Status9/received0, DirectPlay admission and shared entry/reset are entry
  callers, not callees of the unmounted offline Human reaction. Their completed
  source is preserved; unfinished admission/entry work remains explicit.

The original omissions are **not resolved or initialized**: fresh part
`004CB2CD` still reads5F9 before motion; wheel `005FDD10` still reaches695 after
lifetime/body mutation; EXPL8/smoke14 remain required failures on their selected
Car paths. Their [exact failed prefixes](human-reaction-containing-boundary.md#source-closure-blockers)
and all guards remain. H8/OWN/Car integration must account for these cases;
H2 source closure does not authorize silently running them successfully.

H8 retains full cross-branch Human/World dispatch, scheduling, failure/repeat and
retirement verification. H4/H6 retain their own required consumers. P2.2 and
P22-VIS still require their actual integration and user desktop acceptance.
