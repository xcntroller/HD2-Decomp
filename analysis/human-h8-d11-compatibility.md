# D11 compatibility policy and source closure

2026-11-16, `feature/p22-hum-h8-4`, after `309d700`.
The user authorized the practical route: “Alright, do the practical route for
now. We need to focus on getting HUM done.” This is an explicit exception for
the four recorded D11 omissions, not permission to guess unrelated behavior.
D11 is source-resolved by compatibility corrections, not original equivalence.
H8.4/H8/HUM and application/desktop delivery remain open.

## Chosen behavior

| Original omission | Compatibility producer | Continuing source behavior |
| --- | --- | --- |
| Rigid part5F9, read/toggled004CB2CD before movement;004C84A0/reset004C8980 leave it unwritten | RigidBody constructor writes phase0 once | First004CB2B0 step toggles to1, considering all14 geometry points. Native subsequent toggles/slow-motion writes and reset preservation remain unchanged. |
| Fresh wheel695;005FDAB0/005FDC00 omit it | DetachedItemActor defaults retire-on-expiry totrue | Real body frame precedes time subtraction; last500ms fades, expiry restores camera amount before deferred removal.005FDC50's explicit part flag overrides the default, includingfalse. |
| EXPL_D record8;004E09D0 allocates without writing it | Every new metadata record starts instance-1 | It owns no particle until004E1F70 successfully publishes the actual identity.004E0DC0/reset/destruction stop only a recorded instance. |
| Smoke definition14;004E08C0 omits it if no model is parsed | CarAncillaryModel defaults smoke definition to-1 | Missing-DM destruction submits invalid fire/smoke definitions; real module17's invalid-definition gate returns-1. Present model PSMOKE and native fallback95 remain unchanged. |

These values replace undefined original reads with deterministic behavior.
They do not reproduce arbitrary allocator contents. No broader zero-fill,
changed collision solver, invented particle resource or successful service is
introduced. Optional guards still reject any explicitly absent invalid state;
normal constructors/parsers now supply the four compatibility values.

Particle ownership is deliberately narrow: unrelated active records survive
fresh metadata reset. Repeated explosion emission retains the original policy:
only the latest handle is stored, while the earlier instance remains owned by
the process. Native fire/smoke publication and cleanup order are preserved.

## Verification

Temporary native audit passes3328 checks over32 repeated lifecycle families
under ASan/UBSan/leak detection, with strict C++17 warnings. Actual source owners
are DetachedItemActor, ActorBase/storage, RigidBody configuration/geometry/motion,
registry, DynamicCollisionBuckets/Exclusions and CarAncillaryModel parsing,
emission-ID publication and retirement. No warm-state5F9/695 seeding is used.

Coverage includes fresh moving parts/wheels, all-point first part query, actual
gravity/frame publication, real body-before-clock failures,500ms fade/expiry,
failed fade without prefix replay, explicitfalse part retirement, borrower/
registry teardown, fresh reset with unrelated active particles, failed emission,
failed stop/retry, repeated IDs, present authored/default DM and absent-DM
positive destruction. Frames/math/visual bounds and common activation endpoints
are explicit engine doubles; particle/SOUND/fade/removal endpoints are explicit
terminal doubles. The actual source rigid solver runs; complete original engine
trajectories, loaded-World execution and desktop behavior are not claimed.

Strict O0 i386 actual nested World/Car/Item12/Blast/DM compile/link instantiates
runtime factory12, both launchers, both frame passes and real body/particle
bridges. Launcher/probe and tracker verification are recorded in the
[receipt](checkpoints/p2.2-human-h8-d11-compatibility-20261116.json).
Temporary audits are fingerprinted and removed before commit. No permanent
tests, new frozen entries, complete promotions or delegated engine credit.

## Remaining work

D11 no longer blocks the selected source paths under this approved policy.
The original omission evidence remains valid in the historical
[boundary contract](human-reaction-containing-boundary.md#source-closure-blockers),
[part motion](human-car-rigid-motion-source.md), [Item12](human-car-item12-source.md)
and [DM](human-car-dm-presentation-source.md) contracts. Their old required-failure
statements are superseded only for these four initialized states.
Continue H8.4's reached actor/material/Car interfaces, full occupied
Human/Player/mounted composition and selected entry requirements; H8.2/H8.3
remain separate work. Application loading/render/user desktop acceptance are
still P2.2 requirements, not implied by this source compatibility checkpoint.
