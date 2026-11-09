# Actor loading ownership and geometry — P2.2 in progress

Source-only work,2026-11-04. This continues the full unchecked P2.2 loading
milestone; it is not a smaller delivery checkpoint. Concrete actor dispatch,
properties/geometry composition, Car callbacks, loading modules/activation and
first-frame consumers still require integration. No world loading or desktop
verification is claimed. The frozen function ledger is unchanged pending the
consolidated delivery sweep.

## Common actor and storage owners

`recovered/game/actor_base.h` reconstructs00610070's type, IDs(-1/-1), frame,
registration byte, fields70/7C and exact default name “Missing actor name!!”.
00610200 rejects a null frame, retains a real non-null frame, then invokes the
matching storage initialization. The native owner rejects repeated initialization
instead of leaking its old frame. A later actor initializer must still perform
its own model/geometry checks; this base is not a successful actor factory.

00610170 cleans up storage, clears7C, releases the frame, then invokes004CFF70
to remove references from live script contexts.004CFF70 walks the global context
list in reverse. Its00610170 decompilation is a base destructor with the base
vtable already installed; Human's derived cleanup must happen before it.
Native `ActorStorageOwner` owns the matching storage directly and discards it
before frame release. The backend reference-removal and entry-release endpoints
are mandatory, nonthrowing consumers; no host success substitutes were added.

Common virtual+08 is00610F80 for all thirteen required non-Human vtables checked.
Only types4,20,21,34 get one007D7010(kind2) slot owner with first scalar1000.
Other common actors have no storage. Original flag78 becomes1. This is a slot
container, not a clock/timer or a damage controller.

Human's virtual+08 is005A2740 and virtual+0C is005A2A40. It sets flag78=0 and
allocates six owners plus a null array terminator:

| Index | Owner | Kind | First scalar | Second scalar | Flags |
| --- | --- | --- | --- | --- | --- |
| 0 |007D67F0 single-entry holder|0|100|4|2|
| 1 |007D7010 slot array,14 slots|1|100|00863758, cold12|1|
| 2 |007D67F0 single-entry holder|0|100|0086375C, cold8|3|
| 3 |007D67F0 single-entry holder|0|100|8|3|
| 4 |007D67F0 single-entry holder|0|100|8|3|
| 5 |007D67F0 single-entry holder|0|100|32|5|

`HumanStorageScalars` permits the eventual owner to supply changed live globals;
the two mutable values are not silently applied to the literal8-valued holders.
005A2A40 destroys owners in ascending index order, not C++ reverse member order.
Allocation failure also discards the acquired prefix without leaking it.

`actor_slot_storage.h`:007D6690 sets100/100/flags3.007D7010 allocates14 slots
for kind1,32 otherwise, each with its kind byte.007D7220 multiplies each slot's
**first entry** scalar by that slot's stack size.007D7140 releases entries only
when the ownership bit is set;007D75B0 releases ascending slots/reverse entries,
then resets that bit. Borrowed entries survive destruction of the slot arrays.

`actor_item_holder.h`:007D67F0 starts with null entry/field18=0.007D6970 returns
the entry's scalar or0.007D69B0 rejects null(-2), returns the previous handle only
when populated and installs the new one;007D69E0 detaches an equal handle(1) or
rejects(-1), including null==empty.007D6910/007D6A00 always release a remaining
entry and reset both fields. Field18 is not an ownership switch. Both forms use
007DD3F0→007DC8A0: remove a matching entry from the global registry by swap/pop,
then delete through its matching owner; unknown handles are ignored.

`actor_references.h` now supplies that entry-registry removal and004CE8F0's
reverse record removal/swap-pop, fourteen reference slots, and field5A0/564/4E
updates. `ActorContextRegistry` supplies004CFF70's reverse traversal of actual
registered contexts. `host/game_actor_services.h` binds these owners and real
frame retention to ActorBase. Item/context interfaces have no successful default
implementations; actual item and script producers must register matching owners.
The containing world must destroy actors before those producers/services. These
shared services are not an integrated Scene2 factory or loaded mission.

## Ladder and SupplyBox geometry

`ladder_geometry.h` recovers the geometry portion of005EA5D0, before its final
00610200 call. Lookups are first-match `up*`/`down*` mask10020 and visual `*`
mask10001, falling back to the root for visual text. Both markers are required.
Copy their world positions, normalize the root world forward, clear Y and
normalize again, preserving the original tiny-vector branch. A vertical vector
can consequently become Y+1 again. Material defaults25; lowercase user text,
find the first `matid` substring, then apply%d directly after it. An equals sign
does not get skipped. Native parsing rejects int32 overflow.

`supply_box_geometry.h` recovers005F98E0's Steny02 OBJ_ lookup and Ammo00..Ammo13
bindings plus005FA6A0's position/world quaternion and corner extraction. Driver
bounds refer to the root; vertices come from Steny02's LOD0/32-byte layout.
The selection threshold is **half the Y extent, without adding minimum Y**.
Take the first four qualifying vertices without deduplication. The original
three conditional swaps are not a general distance sort: for a rectangular
input ordered diagonal/long-edge/short-edge, corner1 remains the long edge.
The native body preserves that exception. Missing four corners and non-finite
coordinates reject instead of exposing original uninitialized storage.

`host/game_actor_frames.h` reuses actual checked LS3DF find/world transform,
bounds and exception-safe LOD locking. World-quaternion construction calls actual
S_quat::Make at1002F220. These endpoints do not supply actor/item/script owners.
The containing SupplyBox owner must still preserve005FA6A0's order: transform
snapshot, parameter/list reset,00610200,005F98E0, then vertex work. SupplyBox's
later resource/effect/item construction is not provided by a geometry value.

## Switch state

`switch_actor_state.h` recovers00613550's numeric variants and006136F0's ordered
state work, with00613E10 properties applied in serialized order. Common frame
initialization remains the containing actor's responsibility. Align variants2..
with variant1's parent/position, then request0, which clamps to1. Other invalid
unsigned state values, including negative bit patterns, also clamp to1.

State application first notifies a present context in mode bit1 unless suppressed,
then finds variant1, its parent and the selected variant. Missing any of these
follows the original early exit after notification. Relink/position/update the
selected frame, hide root children in reverse index order, then show that frame.
Only then emit a nonnegative configured sound from the root world position.
Sound defaults-1; a sound property preceding a state property affects that state,
while a sound property following it affects only later changes. Runtime context
and spatial-sound consumers remain mandatory services awaiting module/audio
integration, not empty successful callbacks.

0041D960, called by the notification, is a context request helper: in mode bit1,
if104 is unset or D8 differs from both DC/E8, and E0!=-1, copy E0 to D8 and set104.
The pushed state argument is unused; assembly confirms RET4. Do not replace this
with a guessed script-variable assignment. The native Switch suppression flag is
defined false at cold construction; original constructor does not establish E0.
No whole-constructor/body completion claim follows from that safer native value.

## Evidence and verification

### Concrete cold actor composition

`host/game_scene_actors.h` now composes thirteen required families with the
owned Scene2 registry. This is source-only construction/frame/property work;
the containing world, real runtime services and activation remain unintegrated.
Car is still explicitly unsupported. No whole-constructor/completion credit is
claimed for recovered cold branches or native definitions of unwritten fields.

| Families | Original constructor / initializer / property body |
| --- | --- |
|Null33|005E2B10 /005E2B60 /005E2BC0 (real no-op)|
|SpawnPoint10|006118D0 /00611950 /006119B0|
|ExplosivePlace22|0060C230 /0060C2F0 /0060CA30|
|FowlGenerator29|005BA4D0 /005BA4B0 /005BB7E0|
|Pike28|005B8D00 /005B8E00 /005B9C80|
|Ladder8|005EA3D0 /005EA5D0 /005EBBD0|
|Door6|005FF210 /005FF3D0 /00601AC0|
|SupplyBox9|005F97D0 /005FA6A0 (005F98E0) /005FC200|
|Dog30|005D4B00 /005D4D70 /005D6630|
|Animation25|005B3740 /005B3710 /005B5AF0|
|StaticWeapon21|00602EC0 /00603040 /00604CE0 (real no-op)|
|Human3|0059BF00 /0059E5E0 /0059E800|
|Switch13|00613450 /00613550 /00613E10|

`actor_initial_properties.h` supplies proven constructor/version defaults before
the readers run; parser value types alone were insufficient. Human's initial
stats, old-version Ladder defaults, Pike angles/direction, Fowl name/scalars,
Animation's zero D0, Explosive flags/IDs and Door angles/direction are preserved.
SoundEmission004374D0 is a64-byte value: zero except word12=4 and13..15=float1.
It supplies no successful playback operation.

Ladder retains only after marker geometry. SupplyBox now splits its transform
snapshot from geometry completion so its owner resets lists/parameters and runs
00610200 between them, as005FA6A0 requires. Dog retains before requiring model
kind9, and resets all0x45 shared DWORDs008AC4B0 on every construction. Animation
also requires model9;005B3930→005B4790 restores retained-frame visibility and
updates it before common release. Native cleanup attempts both operations and
records the first exception for the containing world to rethrow. Its later
animated-model/context producers remain unreconstructed.

StaticWeapon's004C19D0 subowner is at **actor+160**, confirmed by assembly at
00602EF9/00602FF9; +150 is an independent byte. Native vector destruction matches
00602FC0→004C1AA0 before common storage/frame release. There is no fabricated
property payload:00604CE0 and005E2BC0 genuinely return1 without reading.

Human owns two16-byte-aligned cold control allocations.005AE450/00533880's
empty-queue branch clears list ends and10/14 without command-pool calls; populated
command producers remain unavailable.0059E3E0 calls that clear twice, then frees
the first owner, invokes005A2A40, and frees the second owner before ordinary list/
string/base cleanup. Effect188 and reserved-player cleanup require mandatory
runtime consumers. Native unregistered negative IDs avoid the original invalid
negative player-array index. Script-context storage is not inferred from this
second neutral Human control allocation.

`SceneActorRuntime` has only mandatory abstract consumers, no success defaults.
Human/Switch factory composition requires a live instance. Both captured Switch
records set state1 before sound=-1, so their actual serialized loading path emits
no sound. Switch sound prepares group1/entryD8, flags1 and a world position;
sound-manager vtable0080F7B8+54 resolves00433FA0 (pointer0080F80C). Its4558-byte
producer is now exported as evidence, not implemented or credited. Required
nonnegative runtime sound requests still need that actual producer.

Temporary ASan/UBSan audits now additionally pass77 simple-family records,
31 Door/Ladder records (11 native portal bindings),23 SupplyBox/Dog/Animation/
StaticWeapon records, and74 Human/Switch records from the two captured missions.
They cover22+19+72 truncated property records,11+29+26 geometry/initialization/
cleanup failures, matching storage/frame/opaque lifetimes, localized names and
actual native property bodies. Engine/runtime endpoints remain fixtures. Strict
i386 explicit instantiation covers all thirteen owners against GameActorServices;
runtime methods remain abstract. No Windows fixture is executed.

Logs: `actor-initial-properties-native-audit.log`, `simple-actors-native-audit.log`,
`geometry-actor-owners-native-audit.log`, `model-actors-native-audit.log`,
`human-switch-native-audit.log`, `scene-actors-i386-audit.log`,
`model-actors-i386-audit.log`, `human-switch-i386-audit.log`,
`concrete-actors-windows-build.log`, `human-switch-windows-build.log`.
Additional evidence: `actor-cold-subowners/`, `actor-cold-lifetime/`,
`actor-module-cold-dependencies/`, `human-destruction-order.asm`,
`human-empty-queue-clear/`, `car-callback-owners/`, `spatial-sound-producer/`.

`door_portal_binding.h` supplies00600A50's required Door property refresh. Read
the actual cached sphere centre (frame+30, updated through+58 when needed), find
the first sector ancestor or use the real scene point query, and search its first
portal list. Only when that list is empty use the second list. Choose a portal
within its radius and10 units, with later equal-distance candidates winning.
No match/empty list preserves the previous binding; disabled clears both fields.
The second field is the neutral raw portal+19C attachment. The real bridge uses
engine portal lists/centres/radii and the actual scene, without fabricated sectors.
Nineteen injected endpoint/data failures and selection boundary fixtures pass
ASan/UBSan; i386 instantiation passes against GameActorFrames.

### Scene identities and opaque data

`scene_actor_registry.h` implements the immediate Scene2 branches of004759C0,
00475C50 and00475EC0:39-type descriptor range,32 reserved identity slots, native
matching factories, optional active list and reserved-slot replacement with byte
generation wrap. Absent descriptors do not remove existing actors. Registered
factory-null replacement does remove the previous actor, as in the original.
Frame publication precedes ID assignment and the caller's separate initialization.
Identity/active removal swaps the last element into the removed index and updates
the moved actor's ID. No replacement of this registry with004770B0 is allowed.

00475FB0 captures the original physical identity count and visits slots ascending,
even though immediate deletion changes the logical end. Old tail pointers remain
available until cleanup ends. The native container keeps physical storage and
logical size distinct, with independent matching owners; it does not index past
a popped std::vector end. A65-owner audit confirms reserved-first/original-slot
cleanup order, plus repeated clear/recreate, generation wrap, replacement failure,
identity/active ID updates and four allocation/one binding failures. Concrete
factories/frame endpoints are fixtures in this audit. Deferred/timer/runtime
registry branches and the actual containing module remain outside this header.

`actor_frame_data.h` supplies an eight-word native opaque-data owner for006E9AD0/
006E9B20, bound to actual frame+EC by GameActorServices. i386 size is32 bytes;
the empty-text sentinel is native storage corresponding to original00832CDC's
pointer00867988, not an address into the original executable. Copy all eight
words; bind/clear word0; preserve blocks with used words1/2/4/5/6. The original
free predicate ignores words3/7, including nonzero values. Retain frames until
their native blocks are cleared, preventing a dangling published pointer during
partial loading. Unknown-allocator blocks reject; matching owners alone free
their blocks. Other item/script producers must share this owner. Sector sound
metadata has a separate12-byte owner (`frontend/sound_runtime.h`) and must not be
miscast or adopted as actor data. Native copy/sentinel identity, shared use,
retention, foreign-owner rejection and allocation/retain unwind audits pass.
This native lifetime guard itself earns no original-body credit.

Ignored evidence resides in `analysis/work/mission-world-20261103/`:
`actor-common-owner/`, `actor-storage-release-values/`, `human-frame-initialize/`,
`human-storage-verified/`, `actor-slot-*`, `actor-base-release/`,
`actor-observer-removal/`, `actor-geometry-abi/` and corrected
`actor-geometry-abi-v2/`. Assembly preserves ambiguous owners, Human stores,
SupplyBox threshold/swaps and vector helpers. Constants/strings are section-mapped
from client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
The V2 SupplyBox export corrects005F98E0 to a no-argument thiscall/RET and visual
type to a one-argument stdcall; the old stack aliases must not be copied.

Temporary ASan/UBSan audits pass for slot owners; holder replacement/detachment,
all40 dispatch inputs and seven allocation failures; common actor retention,
eight allocation failures and ordered cleanup; Ladder materials/directions and
all24 rectangle permutations plus41 geometry/data failures and lock cleanup.
Switch adds serialized notification/visual/sound ordering and57 injected failures.
Reference audits check repeated record removal/payload order, reverse context
traversal and item deletion after registry removal. Geometry/runtime/item/script
producers in native audits are fixtures. Strict i386 explicit instantiation passes
for common ownership against real GameActorServices and geometry/Switch frame
operations against GameActorFrames; Switch runtime declarations remain fixture
interfaces. Full application/startup builds pass. No Windows execution.
Logs: `actor-slots-native-audit.log`, `actor-storage-native-audit.log`,
`actor-base-native-audit.log`, `actor-geometry-native-audit.log`,
`actor-storage-i386-audit.log`, `actor-geometry-i386-audit.log`,
`switch-state-native-audit.log`, `actor-references-native-audit.log`,
`door-portal-native-audit.log`, `scene-actor-registry-native-audit.log`,
`actor-frame-data-native-audit.log`,
`actor-loading-windows-build.log`.

Scene2's factory is004759C0, followed by a separate actor+2C initialization in
006CC760;004770B0 is a different runtime registry and must not be substituted.
The new container/opaque-data bodies still need composition with concrete family
constructors, initialization/failure cleanup and the actual module owner.
Constructor00475690 is verified in assembly;00475660 is an unrelated function
and supplies no constructor evidence. No integrated successful actor factory or
whole module reconstruction follows from these source-only containers.

Car continuation2026-11-04: the fourteenth required family now composes the actual
base/storage/geometry/ancillary/property owners and callable native callback set.
The registry requires explicit Car runtime services; actual world integration
remains open. [Car composition, callback contracts and verification](car-geometry-loading.md#car-actor-and-collision-callback-composition).


2026-11-08: virtualDC world-corner source now recovered for14 confirmed loading
vtables, including common/local aggregation, StaticWeapon exclusions and Human
visual "a". Exact contracts, native bounds/transform checks and actual i386 source
composition are in [HUD selection contracts](hud-loading.md) (latest section);
actor-world-corners/ and actor-local-bounds/ preserve originals. Flat descendant
bounds are accepted. This does not activate actors or complete other actor families.
