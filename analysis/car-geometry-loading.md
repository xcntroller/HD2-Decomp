# Car construction during mission loading

Source-only continuation,2026-11-04. This belongs to the full P2.2 implementation
checkbox, which remains open. Neither geometry fixtures nor a decoded property
block establish an initialized actor, engine acceptance or desktop world delivery.
The game initializer still rejects world activation honestly.

## Original contracts and reconstructed values

`recovered/game/car_geometry.h` composes the ordinary runtime path of004E2E30
in original order. It retains the real model until dependent owners are destroyed,
requires kind9/BODY, extracts exhaust/special/head markers, builds/extracts BODYCOL
trees, discovers doors/seats and wheels, initializes visual axles, extracts CENTRE,
measures remaining model bounds, builds strip connections and deformables, binds
door/window parts, constructs lighting, then applies original default physics.
The containing failed loading stage must discard the modified model/scene;
geometry mutation itself is not transactional. BALLAST now preserves the literal
current-slot insertion/permutation at004E4267–004E42B7; it does not sort by suffix.
The four captured string censuses contain none, so this branch uses a synthetic
fixture and is not runtime acceptance.

| Source | Original behavior |
| --- | --- |
| car_physics.h |004ED270 runtime third-argument-zero application: geometry-derived inertia/centre, suspension, steering, tyres, drive/brake flags, gearing, sound coefficients and ten deformable types; editor repair/remap excluded |
| car_tyres.h |004D6900 version1 registry including develop0;004D6BF0 twelve-float conversion;004EC980 native ownership; actual runtime file service required |
| car_collision_parts.h |004F9E70/004FA0E0/004FA140/004FA190/004FA2E0 hierarchy, points, relative extraction and actual marker removal |
| car_wheel_geometry.h |004F9BE0/004E2E30 L/R axle ordering, measured radius/width/diagonal, DWH parents and wheel positions; half-track has nine pairs |
| car_axes.h |004E6DE0 discovery and00500FD0 visual axle scale/direction; separate from strip physics connections |
| car_deformable_parts.h |004FA520 mesh copies/glass material copy,004FAEA0 variants and004FE6B0 visibility/bumper ordering |
| car_auxiliary_geometry.h |004E2E30 marker extraction, numeric-door filter, suffix swaps, window values, seats/door indices, CENTRE/steering/smoke and bounds |
| car_strip_connections.h |004E59C2–004E618B private strip meshes, reverse wheel/vertex iteration, Z band/below-centre selection, ushort indices/offsets and wheel displacement |
| car_lighting.h |004E7450 families, billboard hiding, type/part binding and paired projectors;004E87C0 single-texture and004E8D80 three-texture metadata |
| car_default_parameters.h |004E6313–004E6CCD zero initialization,187 scalar/byte stores, per-axle columns/flags, texture names and first004ED270 call |

Native values use named fields, owned vectors and indices rather than guessed raw
actor layouts. Original offsets retained by physics are research identifiers.
Non-finite geometry, table/index overflow, missing required pairs and singular
transforms reject explicitly. Parameter defined-byte masks exclude uninitialized
padding. Tiny-vector normalization replaces one component while preserving other
small components, as visible in original assembly; it does not zero them.

`recovered/host/game_car_frames.h` supplies actual guarded LS3DF endpoints.
Enumeration uses1001DE40 and its callback returns0 to continue;004F9CB0 selects
the last match, while frame Find1001DED0 stops at the first callback return2.
Ordinary local-matrix construction004C8140, world update1001E390 and actual
Inverse/Make/SetRot3/vector transform/GetScale exports preserve their explicit
stdcall stack arguments. Inverse rejects singular input. Driver bounds1001AE10
returns padded vector bounds; dummy bounds require actual kind6.

Deformable visuals get private mesh copies and glass material copies. Strip
preparation10036160 clones only shared meshes. Mesh1009CE60 +04 is10031E20;
LOD1009CE10 +04/+08 are10031380/10031480. Vertex count is+54 and actual stride
must be32 (+64); every successful lock attempt has an exception-safe unlock.
Projectors use actual PROJ factory, Graphics texture loads and real light frames.
Single-texture failure keeps the untextured projector as original; three-texture
failure releases partial resources and returns no projector. Native ownership
keeps shared resources alive once and releases them before the retained model.
These delegated engine bodies receive no new function credit.

## Evidence and verification

Ignored evidence lives in `analysis/work/mission-world-20261103/`. Original client
SHA256 remains8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307;
engine remains12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee.
`vehicle-signatures-v4/004e2e30.c`, `vehicle-geometry-complete.asm`,
`car-parameter-application.asm`, deformable/variant exports and tyre loader
exports preserve the larger contracts. Read-only transient ABI repairs resolve
discarded branches; `car-lighting-typed-v3/` and light/projector assembly preserve
lighting evidence. Allocation-container aliases in those decompilations are
checked against assembly and replaced with native ownership, not copied as ABI.
`car-strip-engine/`, `car-mesh-*.asm`, `car-lod-*.asm/json` and
`car-frame-find.asm` preserve engine boundaries. Original files were not modified.

Temporary native ASan/UBSan results:

| Audit log | Verified boundary |
| --- | --- |
| car-runtime-native-audit.log | Earlier tyre registry/runtime parameter audit; base tyres fixture, not runtime-selected file |
| car-ballast-native-audit.log |Literal permutation, transformed records/removal and36 endpoint failures |
| car-ancillary-native-audit.log |Base DM-name/attribute fixtures:72 zones/nine explosions, seat misses, particle defaults, failed loads and17 endpoint failures |
| car-auxiliary-native-audit.log |22 captured seat-name/two numeric-door-name fixtures, extraction/order/window indices, missing required markers and107 injected endpoint failures |
| car-defaults-native-audit.log | Constants independently compared to original assembly stores;0..12 axle defaults and2..12 axle physics with all ten deformable types |
| car-strip-native-audit.log |Nine-axle strip ordering/selection/offsets, empty matches and248 endpoint failures with unlock |
| car-composition-native-audit.log |Two/nine-axle composition, markers/collision/interior/strips/deformables/default physics, single/shared projectors and962 failures with full resource cleanup |
| car-geometry-i386-audit.log |Strict explicit instantiation of helpers and composed geometry against GameCarFrames |
| car-geometry-windows-build.log |Affected application-launch/engine-window/startup-probe build |

Geometry/resource endpoints are synthetic in native audits; raw model strings
only seed name/attribute fixtures. No original DLL/game/Proton or Windows fixture
was executed. No desktop acceptance, whole Car actor credit or successful world
load is claimed. Temporary programs stay only during active auditing and are
removed after source/evidence fingerprinting.

`car_ancillary_model.h` now reconstructs004E0930/004E09D0/004E19C0: actual
optional cache load of DM+resource, per-seat visual lookup, reverse model-frame
inspection, HOR/VERT planes, D_ZONE HP/material, EXPL_D delay/part and particle
SID/PFIRE/PSMOKE defaults. Base archive fixtures for all four variants are in
`car-ancillary-base-fixtures.json` (not runtime-selected inputs); real runtime
selection remains through the existing cache. No new capture gate is inferred.

## Required continuation

Car actor construction/geometry/COMP/callback binding is now composed source-only;
see the continuation below. Actual containing world/module, physics callback
invocation and particle/item/runtime producers remain required. The tyre registry
must load at the actual004D6900 calls before Scene2/activation. Loading presentation,
collision/effects modules, activation and actual first-frame consumers remain
within P2.2. Ledger stays34 complete/559 partial pending consolidated delivery.

## Car actor and collision callback composition

Source-only continuation2026-11-04: `car_scene_actor.h` now composes004D0B60 /
004D10A0 /004D55C0 with the actual base/storage, geometry and ancillary owners.
`GameSceneActors` now has all fourteen required cold families, including Car4.
Its constructor requires a `CarCallbackServices` receiver in addition to the
Human/Switch runtime; no containing loaded world instantiates them yet. Car's
localized1BF0 name,0073EA50 control-clock sample and0089BC28 registry follow
original construction order. Destructor004D0F20 releases the ancillary model,
swap/removes its last matching registry slot, releases cold lists, then geometry
before common storage/frame/context cleanup. Populated runtime lists/physics
producers remain required; cold values do not claim those bodies are complete.
All nine captured COMP/property blocks now apply to this same native geometry.
The original ignores004ED270's false result at004D55C0; native loading rejects
it explicitly to avoid publishing invalid physics.

The callable native slot set contains these actual recovered algorithms:

| Geometry slot | Original function | Behavior |
| --- | --- | --- |
|680|004D4230|Frame userdata word0 determines dispatch; descriptor+48 independently identifies Human victim. Signed wrapped seat timestamps gate non-mode7 hits, then actor+58/+5C and authority/network damage. |
|684|004D4450|Human contact direction normalization and speed/alignment damage gate. |
|688|004D4130 /004F32A0|Human zero response or actual Car angular/linear/mass/inertia response, with matching in-place engine transforms. |
|68C|004D4620|Original type jump table; Human height/speed and Dog speed gates. Third fastcall stack argument is unused output. |
|690|004D4740|Repeated real driver queries, control/role/mode gates, impulse/mass damage, angle/rollover multiplier, descending occupied seats or target−1 event. |
|694|004D4CA0|Typed part masses, detached Item12 production, reverse contact-link invalidation and hide/update. |
|698|004D4EB0|Wheel Item12 production, local/authority damage event routing and hide/update. |
|69C|004D4A10|Actual GEffects+B4 fade then hide, without update. |
|6A8|004D4A70|Script word4 precedes timed word5 and flag8bit2 filtering; shared frame data is reread. |
|6AC|004D4B50|Material rejection, Item interaction005FEFB0, or actual blast/random surface effect. |
|6B0 /6B4|004D49F0 /004D4A00|Genuine empty callbacks, RET10 /RET. |
|6BC /6C0|004D5B10 /004D62C0|Particle texture/configuration and pool reuse described below. |

6A4 is explicitly null;6A0/6B8 remain cold null. The containing native physics
producer must invoke this typed slot set; no raw original function pointers or
client ABI objects are fabricated. `car_callback_services.h` makes all absent
live consumers abstract. `game_car_callback_frames.h` supplies actual common
frame/identity/shared-userdata/control-clock endpoints, while actor damage,
Item12/004770B0, script, effects, network and particle runtime owners remain
mandatory. This does not claim that the callbacks are connected to live physics.

Assembly corrects misleading inferred prototypes:004D99D0 (Car virtual+78)
consumes **one** seat-index argument, RET4. The other pushed zero at004D43AC /
004D49B7 is Human+D4's final hit-kind argument;005A1C90 consumes six stack
arguments, RET18. Absolute stack-slot tracking preserves the actual victim
receiver, point, impulse, driver and zero hit-kind.004D4740 itself consumes seven
stack arguments, RET1C, although only point/flag/contact are read in this body.
0069BFE0 is genuinely empty; no successful diagnostic service was added.
004D5140 clones only frame kind1 into runtime Item12 through00479450, derives
position/normalized world-axis and calls005FDC00 or005FDC50. Other frame kinds
return true without spawning. Its32-byte contact-link records are **not** the
128-byte door records. Native guards reject missing owners and integer overflow.

Numerical stores retain x87 intermediate rounding where relevant: Human height
adds float.3 in extended precision, contact velocity's Z cross component stores
before subtraction, passenger squared length sums Z²+X²+Y², and part/wheel impulse
Y/Z stores follow assembly. Tiny normalized vectors retain their other lanes;
unwritten original fourth lanes/padding are defined native values, not recreated
undefined memory. `S_vector::AngleTo` is delegated to the actual checked export
1002E8C0 with two explicit stdcall pointers, RET8; no new engine body credit.

`car_particles.h` preserves the240-byte table records separately from48-byte
native pool identities. New texture loading passes primary, optional secondary
(length>=4), and flags2/4, then binds/names `Car_particle`, unlinks, applies unit
scale/forward direction, updates/resets and applies the original ordered
properties. Only004D5B10 writes property34 from table+AC. XYZ state writes retain
native fourth lanes. Existing regions clear backwards through+C8; matching table
names add regions backwards; selected four-float region matches select through
+C0. Reuse checks the actual texture name first, then configures, selects the
matching region, updates, sets active/pending and invokes+AC(1). Texture ownership
is scoped across all operations and partial failure. Actual engine particle endpoints, flat table owners and30-emitter pool are now
recovered below; their containing runtime integration remains required.

Evidence added under the ignored work directory: `car-actor-contact.asm`,
`car-passenger-contact.asm`, `car-contact-trigger-callbacks.asm`,
`car-part-contact.asm`, `car-particle-callbacks.asm`, `car-vector-angle.asm`,
`car-contact-*-dependencies/`, `car-passenger-dependencies/`,
`car-final-cleanup/`, `car-callback-uses-all.json`, and independently extracted
`car-contact-filter-case-evidence.json` /`car-actor-parameter-fixtures.json`.
Original client/engine fingerprints are unchanged. No original code was executed.

Temporary ASan/UBSan: contact audit passes960 original-table filter cases,
authority/network/seat/threshold/event contents, shared userdata precedence,
response math/effect order and26 endpoint failures. Particle audit passes texture
flags/release, property order/regions/reuse and79 endpoint failures. Composed Car
audit passes nine captured properties/36 truncations,113 constructor/geometry/
ancillary failures, cold clock/null/optional-DM cases and cleanup order. Engine,
frame/runtime geometry and module endpoints remain fixtures. Strict i386 callback
instantiation and actual GameSceneActors/Car application builds cover signatures;
no Windows execution or desktop verification. Current fingerprints, log paths and
temporary audit removal record are in `car-callback-verification-wip.json`.


### Emitter ABI correction and pool/table owners

Engine evidence newly resolves the earlier region-operation ambiguity:
EMTR vtable1009C050 slot+C0 is1001BAF0, a setter for selected region+204;
+C8 is1003BED0, which removes a record with tail replacement. Source now calls
`select_car_particle_region`, keeping all matching records and selecting the
lowest matching index after descending traversal. The earlier particle fixture
incorrectly modeled+C0 as removal; its79-failure log is historical and superseded
for that behavior by `car-particle-engine-native-audit.log`. Native mode/failure/
texture/property claims remain, while the corrected audit checks duplicate region
selection, property order,30 owned emitters and1488 setup/initialization failures.

Actual EMTR factory is driver+50→10019E30→10019750(index9). Texture+9C is
1003AF50, property+A0 is1003AF80, reset+A8 is1003B8A0, activation+AC is1003BFB0,
region+C4 is1003BA20. Host calls validate these exact targets with explicit
stdcall arguments and matching texture/frame release. Texture-name lookup reads
emitter+1F0→texture+18;13-word region descriptors contain only known values.
`game_car_callback_frames.h` supplies these actual endpoints; other live actor,
Item/script/effect/network receivers remain abstract. Evidence is in
`engine-emitter-*` exports and `car-vector-angle.asm`.

`car_particle_pool.h` composes004D5A50/004ED0C0/004ECFF0: clear/unlink/release
old frames backwards, create at most30 real EMTR visuals, try definitions1..end
until a texture loads and retain each successful visual. Empty definitions
produce the original zero-visual sentinel with index−1 and returnfalse; a single
definition or all texture failures leave an empty pool and returntrue, as original.
Partial native failures release local/prior owners; destructor records cleanup
failure after attempting all frame releases. Strict i386 explicit pool
instantiation uses actual GameActorServices and mandatory CarCallbackServices.

`car_definition_tables.h` reconstructs0071F9B0's ordered actual file loads:
`tables\hdmaterials.def` first, then `tables\effects_car.def`. If the second is
missing the first table has already changed. Fixed256/240-byte records ignore
trailing bytes as original; native short reads reject.004D57A0/004ECD60 construct
response values, clamp the seven scalar inputs, retain three DWORDs and the
special48/51..55/64/65/95 flags, clear the shared transient queues, then replace
canonical responses and the255-entry lookup. Empty material input gets the real
original default response. Activation must preserve the original call timing;
Scene2 invokes the response rebuild before0071F9B0's post-load table population.

Base archive fixtures (read-only native extraction, not runtime-selected data):
98 material definitions /22 Car particle definitions. Native ASan/UBSan verifies
clamps/flags/lookup/default, trailing data, missing-file publication order and
short-read close in `car-definitions-native-audit.log`. Production must still use
rw_data overrides. No original/DLL/Windows execution or new ledger credit.
