# H8 Active AI presentation and Car camera source checkpoint — 2026-11-12

H8.2 and full H8 remain OPEN. This source checkpoint binds presentation only;
Active authority00526550 still reports its required receiver failure. No actual
application activation, Windows/original execution, desktop acceptance or push.
[H8 plan](p22-hum-h8-plan.md), [receipt](checkpoints/p2.2-human-h8-presentation-20261112.json).

## Original boundary and ownership

Final Active table008109F8 +24 is00525DA0. Bit800 returns before timer194;
on-foot/non-Car states only subtract elapsed194 and clamp a signed-negative
wrapped result. They do not copy model facing. Uncontrolled Car seats query
Actor8C bit10, then Actor90; a nonnull camera produces TWO fresh world-matrix
forward reads, each normalized before actual direction0055D2E0/0055D630. Raw
B010 is copied to Active160 afterwards. Controlled seats instead gate model
facing on byte230 or target absence, velocity-square below0080F948 and zero198.
This branch publishes normalized model forward to rawB010 and actual direction;
it does not overwrite160. Target checks retain short-circuiting and unordered
velocity rejects. Timer194 follows all reached presentation work, so failures
retain its prior value. Arithmetic/normalization order is checked against raw
x87 instructions, including the extended comparison and stored FLOAT square.

Final Car00810778 +90 is004D9E10; +94 is the distinct004D9FF0 name-index lookup.
Seat38 is a name index, not a frame.004D9E10 bounds-checks actual geometry seats,
then actual flags8C. Nonzero passenger seats without bit10/control find under the
occupant model. Other seats unlink the current occupant model, choose cameraFPV*
or camera3RD* via actual World0069A4B0 and ActorCC, find under seat marker2C,
and on miss re-evaluate the same key before searching marker.parent108. They
re-read the current occupant for relink/update before returning even a NULL
camera. Lookup owns no model reference, additional seat array or duplicate Car.
RAII also attempts relink/update on a thrown reconstruction endpoint and records
cleanup failure; native engine endpoints did not expose this exception API.

World binds SAME Startup FPV58/59 and existing H3/UI occupied-seat presentation
for8C/98/CC. Same Human factory owns the Active state and direction owner; Scene2
uses the concrete World presentation overload. No unnamed outer Runtime receiver
or replacement Human/state/input owner is used. Required invalid-owner cases
remain failures, with finite direction constraints inherited from the existing
actual direction owner; no complete native ABI equivalence is claimed.

## Verification and remaining work

Temporary strict native ASan/UBSan/LSan passes5800 checks/100 lifetimes with actual
scalar/direction owners and explicit Human/Car/engine/World endpoint doubles.
Covers on-foot timer-only, bit800-before-Human/timer, camera freshness, real
locks, raw10/160 distinction, controlled230/target/velocity/198 gates, NaN
velocity, countdown wrapping, missing camera and failure-before-timer, passenger
lookup, FPV short circuit, fallback key re-evaluation, relink/update, reentrant
occupant replacement and exception restoration. It does not execute a live
World or real engine. Strict entire Human frame + Scene2 bridge i386 FULL LINK
uses actual host World/types with EMPTY outer Runtime; no fixture execution.
Launcher/probe rebuilt; fingerprints and logs are in the receipt. Temporary
audit sources/binaries are removed at this checkpoint. Two frozen functions gain
partial source claims; delegated engine calls gain no credit.

Original source/static receipts in WORK (`analysis/work/mission-world-20261103`):
`frame-human-state-common-updates/00525da0.c`,
`human-h8-active-presentation-and-tail.asm`, `human-h8-car-seat-direction-vtable.json`,
`human-h8-order-seat-owners/004d9e10.c`, `human-car-frame-field-uses.asm`.
Research index receipts precede exports. Original image SHA is retained by the
composition receipt; no types or original assets changed.

Continue Active authority00526550 through its actual pending-order producer,
current-action families, decisions/follow, query/navigation/aim, movement,
trigger and command retirement, with self-treatment cadence last. Pending-order
manager+18 is distinct from current-action kind1 manager+60. Existing order
worker exports and full raw authority source are retained; they are not credited
by this presentation checkpoint. Hazard/water, reached Player/Script/control,
async ShotMgr, Car/OWN/FRAME/render and user acceptance remain H8 dependencies.
