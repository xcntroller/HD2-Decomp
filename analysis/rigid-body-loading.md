# Loose item and rigid-body loading — source only

P2.2 remains open. These owners are not a constructed mission world. All native
frame/model/sound/network endpoints below are fixtures; Windows targets are only
compiled/linked. No original executable/DLL or Windows fixture was run.

## Original loading contract

00477660 creates loose items as runtime type11 through005B6740/005B67D0,
then005B69B0 binds the item and configures real physics/collision. This differs
from queued static type35 Items004794D0. Vtable00811AA8 and exports under
`work/mission-world-20261103/dynamic-item-loading-receivers/` preserve the boundary.

`rigid_body_bounds.h` reconstructs004C7DB0/004C7E50. Copy root OBJ_ bounds
without applying the root's local transform. Descendant OBJ_ boxes use
local*ancestor matrices and eight transformed corners. A non-OBJ_ descendant
stops that entire branch. This is distinct from the driver's aggregate-bounds
method used by Car. Host endpoints delegate actual matrix product1002D830,
vector transform1002EAB0 and inverse1002C620. The identity constant008106C0
and frame child/sibling10C/110 layout are checked. Hierarchy cycles and excessive
depth/count reject.

`rigid_body_geometry.h` supplies004CD6E0's geometry/scalar calculations from
actual bounds, positive mass and the SAME body's incoming580 coefficient
(cold10). Preserve x87 stores: Z extent stores before centre calculation,
X/Y extents store after adding float.05, and reciprocal Y stores for the last
aspect test. Balanced boxes use the mean of expanded sides for inverse inertia;
elongated boxes scale the longest axis. Outputs include14 corner/midpoint points,
centre90, inverse inertia1F0, gravity220, radius588 capped atfloat.04 and
560/574/578/57C/584 coefficients. Unwritten fourth lanes have no value contract.
`rigid-body-constants.json` and `rigid-body-parameters.asm` preserve constants
and instructions. Invalid bounds/mass and overflowing results reject.

`rigid_body.h` provides loading/registration004C84A0/004C8820/004C8980,
state changes004C8270, actor publication004C8250 and close004C8EA0/004C8770.
Borrow the actor's frame and actual body list. Preserve world/inverse and prior
matrices, known cold writes, motion, sleep callback and reset fields. Bind then
configure then reset inserts the SAME descriptor into DynamicCollisionBuckets.
Descriptor flags40, mask80802 (or801 for parameter40), actual bounds, position,
radius, matrix/inverse pointers and borrowed actor are concrete state. State0→1
invokes the actual callback before changing state; parameter41 enlarges bounds
and radius when sleeping. Unwritten scalar reads reject. Reset requires prior
collision removal rather than losing a stale bucket binding. Populated contact
arrays, queries and simulation remain separate receivers.

Native lifetime guard `remove_for_cleanup` detaches a descriptor before its
owner dies even if allocating the bucket recycle queue fails. An empty bucket
may remain in its actual cell and is reused normally. Ordinary removal keeps
its existing verified behavior. This guard earns no original function credit.

`dynamic_item_actor.h` implements type11 construction/loading/binding/activation
and deactivation against RuntimeItem and these rigid-body owners.005B68D0 calls
the actual frame+54 uniform scale receiver before common frame/storage init.
005B69B0 binds the item's model, mass and incoming velocity, sets coefficient
.8, uses signed definition category007D32A0 for parameter41 (NOT instance
family), resets/registers the body, publishes the actor and caption and initializes
cooldown/lifetime fields. Authority outside mode7 uses real server policy and
shared random: truncate(random*10000+30000); other verified branches use−1.
The selected source mode7 avoids that multiplayer branch.

005B7630's callback filters descriptor40/mask10100; other contacts require real
005B7570 sound and authority outside mode7 requires005B7C50 sleep notification.
These are mandatory services, not successful defaults.005B6920 closes the body,
releases actor frame20, removes the entry through the SAME Items module, then
runs common deactivation.00610510's registered branch does not read a frame;
ActorBase now permits that branch after frame release, while ordinary Scene2
deactivation still requires its frame. Both false/true passes are safe.
GameRuntimeActors composes type11 and35 and rejects unsupported runtime families.
The containing world, Items module/transfer and actual callback services remain
to integrate. Source inclusion alone does not publish loading readiness.

## Verification

ASan/UBSan passes1029 geometry/mass combinations, three bounds-hierarchy cases,
12 motion/mask/registration cases,13 endpoint failures and an allocation failure
during descriptor teardown. Another audit composes actual RuntimeItem, ActorBase,
rigid body and real buckets for ALL24 captured placed records (Tutorial23,
Alps1 one), plus eight mode/random/contact cases and13 binding failures. Model,
frame and sound/network endpoints remain fixtures. Strict i386 instantiates
actual host frame/registry/type11/type35 adapters against required abstract live
services. Windows compile/link and native regressions pass. No desktop or ledger
credit follows from these source results. Verification manifest: `work/mission-world-20261103/item-placement-verification-wip.json`.
The16 temporary audit sources/binaries/objects were fingerprinted and removed.

## Next dependencies

Continue actual007D8B40 transfers and Items module membership/activation queues.
Both holder007D6A00 and slot007D75B0 cleanup call007DC8A0; they must share
the same module index/actual entry owner.007DCB70 frees its raw pointer list
without deleting entries; actual owners must clear entries before destroying that
index. Transfer/storage exports are under `item-transfer-storage-boundary/`;
`item-transfer-entry.asm`, `item-transfer-loading-call.asm` and
`item-module-lifetime.asm` preserve the caller ABI and lifetime details.
