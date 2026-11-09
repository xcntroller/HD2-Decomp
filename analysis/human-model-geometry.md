# Human model, skeleton and loading joints — P2.2 source work

This is an ordinary-loading dependency of Human activation, not an activated
Human or containing world. `HumanSceneActor` still has neutral98/A0/A8 words;
its real inline98 controls must replace them together after the remaining live
DD0/230 dependencies are implemented. GameApplicationState still declines loading.
No first world frame, original/DLL execution or desktop verification is claimed.

## Original behavior and source

`human_skeleton.h` uses the exact23 twelve-byte names at00816D18 and the
hierarchy insertion order in0079C320.007A42A0 ignores missing frames; after
storing index/frame/parent, a fourth child cannot enter its parent's three slots.
The original root is forcibly node22 after insertion.007A4D50/007A4DF0 update
the first matching child edge, or all THREE parent-edge constraint copies.
Flags, weights, offsets, lengths and oriented bases are retained as native state.
Node stride440 is not imposed on host pointer sizes. Unknown SIMD padding is
excluded. The graph does not itself assert complete activation when markers fail.

`human_joint_math.h` recovers007A2190 transpose,007A2E10 vertical basis and
007A3E60's nine-entry rotation between vectors, including its double.999999
parallel threshold and antiparallel construction. Frame matrix products and
quaternion conversion use the actual existing LS3DF exports in the i386 adapter;
native fixtures supply these endpoints explicitly. Original007A23F0's identity
input branch supplies the skeleton's oriented bases; arbitrary input/preserve-pose
is not credited as a complete recovered body.

`human_joint_owners.h` owns007C5080 body and007C5D50 point allocations, loading
bind007C52E0 and attachment007C5620, with matching007C51A0/007C6050 tree cleanup.
007C5080 is stdcall(parent,capacity), ret8: the decompiler omitted capacity.
Cold bodies zero280 bytes, initialize20/60 matrices, index190/194=-1,
weight198=1 and nine NaN limit/direction words. Typed pointers replace original
32-bit pointer slots; scalar offsets remain explicit. Native heap overwrite on
bone-capacity exhaustion and leaked fourth-child allocation are guarded.

007C52E0 stores start/end bones, copies world pose, derives length/radius and
aligns forward direction. It forms parent-relative pose with a transposed parent
basis and explicitly zeroes the transposed translation column, computes relative
angle, converts quaternion and saves native pose copies.007C5620 has the SAME
translation-column clearing, confirmed in assembly. Missing required bones fail.
Native point creation links up to three children and four borrowed body members,
stores endpoints/relative coordinates and attaches both body endpoint links.
Bodies close recursively before points, clearing matching endpoint references.
Native guards also reclaim orphaned partial nodes without publishing a world.

`human_loading_joints.h` consumes those owners in007C0D90's normal branch:
11 bodies,16 points, exact bone membership, length/radius scales, signed joint
limits and their complementary square roots, two cross-axis rotations,14 saved
local matrices and six matrix/vector work records. Preserve=true is this helper's
original immediate return; containing6950 preserve-pose recovery is still absent.
There is no solver/contact tick or collision registration in this helper.

`human_model_geometry.h` composes the ordinary geometric work of6950 owner
0079B910/0079BAC0 and loading0079BD30→0079C320→007B85C0→007C0D90. Constructor
and normal bind each consume one SAME client CRT random draw, plus native+1.
The23 bones and model receive unit scale/actual quaternion/flag updates. Optional
camera3RD/cameraFPV references retain/release real frames; hw1/hw2/z and feet
borrow their model frames. Movement directions/position/default offsets follow
007B85C0. Repeated binding releases old joint and camera owners; missing skeleton
markers fail and retract all partial ownership. Untouched native reset/scalar
state is not claimed complete. Mode7's conditional face-animation owner and
preserve-model replacement remain explicitly unavailable. No DD0/230 activation
or simulation consumer is supplied by this model dependency.

`human_look_state.h` implements007AC780/007ACA30 command-state writes, shared
CRT timing and scan-side selection.007AC780 reads PREVIOUS flags on scan entry
and then overwrites that mutation with incoming flags. Its zero-command branch
also overwrites a temporary80000000 bit with the saved prior flags. Assembly
confirms both; they are preserved. Low BYTE80 selects timed look behavior.
`human_look_control.h` supplies E0 constructor/reset/binding/command dispatch
0077DD80/0077DDA0/0077DDC0/0077FD40/0077FC30. NativeAC borrows the ADDRESS of
DD0+258, not the current Human pointer. Its actual DD0 owner is still required.

## Verification and evidence

Focused ASan/UBSan:23 names/22 links, constraints,23 missing markers, basis
singularities and child saturation;11 bodies/16 points/14 saved matrices, repeat,
preserve helper, ordered teardown, bone capacity and146 frame endpoint failures;
model shared random, optional cameras, repeat,23 missing bones and228 endpoint
failures; look target/null/return/scan cases and original flag overwrite.
Strict actual host i386 instantiates skeleton/joint/model owners and real frame
endpoints. Application compile/link is recorded separately. Fixture matrices,
quaternions and marker frames are not live engine/model validation.

Evidence under analysis/work/mission-world-20261103: human-geometry-bindings/,
human-skeleton-records/, human-joint-construction/, human-joint-pose/,
human-joint-angle/, human-control-record-defaults/, human-model-debug/;
human-skeleton-binding.asm, human-joint-allocation.asm, human-joint-pose.asm,
human-joint-cross-axis*.asm, human-look-state.asm, human-skeleton-names.json.
The failed007C5F50/007C6150 exploratory export is not used:007C5F50 overlaps an
existing body. Correct point constructor007C5D50 and actual helper starts were
exported separately with no errors. No claim relies on that failed export.

## Immediate remaining Human dependencies

DD0 bind00785B20 establishes physical/control borrowers, consumes random atDA4,
computes static collision height bounds006FDA00, resets its AC0 movement control
007634F0, binds AA0/C00 helpers, then binds the model. Movement control reset
performs an actual radius.2 vertical ground query00718530→00715510 for stance1.
Required ground/static/dynamic query consumers now have source (below).
Physics init0075B280→0075AE40 calls the actual sound manager+90
with(3,4); audio/state receivers must exist;004336C0 is the existing client sound owner. Continue that call graph and collision
marker/descriptor setup before constructing a live DD0 owner.

Equipment230 construction0078CA00 calls0078CE20 holster/model loading, and
activation0078CA60 calls real item/model receivers. Frame light, holster cache,
item-hand/pack attachments and matching0078DE30/0078CA30 are required. Scene
registration+138/+13C, shared presentation0078A4F0/0078A5F0, initialization
007506E0/00757E10, animation/physics/sound cleanup and25 retained frame records
remain. No new function ledger credit is assigned for these partial owners.

### Collision setup now in source

`human_collision_markers.h` binds exact0084E768 names stand1/2/3, kneel1/2,
lie1/2/3/4, keeps actual kind6 frames and implements0075C070 snapshots and
0075BFF0 posture copying. Missing markers leave holes and reduce the result;
they are not compacted into a fictitious complete posture. Centres, model-relative
coordinates and native half-X-extent radii remain explicit. Physics descriptors,
runtime posture transition state0075BC80 remains. Loading descriptors and shared
query exclusions now have source, described below.

`collision_vertical_limits.h` implements006FDA00 against the SAME collision tree:
10000 triangles use actual retained vertex views; family83 rectangles transform
four local X/Z corners with their stored20 matrix. Empty/no-tree returns original
±FLT_MAX limits. It does not answer a ground hit. Focused sanitizer checks pass
all9 missing-volume cases, kind filtering, posture holes,45 injected endpoints,
local/live-mesh/rotated-rectangle limits and empty/truncated geometry. Strict
actual i386 adapters instantiate both new helpers.

Ground-query research found a concrete existing-loader defect: per-cell+08/+0C
were discarded as scratch.006EE570 preserves both serialized values, and
00715510 reads both to filter cells before007151D0. `collision_tree.h` now keeps
these pairs alongside cells in the SAME native Y,Z,X flattened order. Independent
file extraction and focused ASan/UBSan compare every bit of32768 pairs for each
captured Tutorial/ALPS1 input. This metadata fixture intentionally has unavailable
mesh references; it is not a claim of a live collision load. No other previously
verified subsystem was broadly re-audited. The subsequent required ground-sweep
implementation is described below.

## Required loading ground sweep now in source

`collision_sweep.h` composes00715510/00718530 with real static cells and the
SAME dynamic descriptor buckets. It preserves radius-expanded bounds, native
truncating grid ranges, cell+08/+0C pruning, dynamic-first shortening, original
FFFFFFFF static/dynamic skip arguments, layer/flag exclusions, nearest squared
distance and normalized/negated output normal. Near-vertical side-plane fallback
is double.001 at008106B8. No alternate ray/ground plane or fake no-hit provider.

`collision_sweep_candidates.h` supplies007151D0's ordered candidates and static
traversal. The first triangle whose first X exceeds maxX terminates its sorted
cell. Actual retained vertex views remain live. `collision_sweep_triangle.h`
recovers00704D60 and007041D0: winding/two-sided flags, immediate face overlap,
original ordered edges/vertex fallbacks and negative edge-overlap best=-1.
`collision_sweep_box.h` factors00706900's six face branches, preserving their
X/Y/Z order, conditional face gates, native twelve-edge masks/order and eight
vertex fallbacks. This is the original rounded-sphere/box operation.

`static_collision_sweep.h` and `collision_sweep_rectangle.h` supply007101B0's
required captured80/81/83 branches, with actual stored inverse/world transforms
and local Y=0 rectangles. Captured shape counts are Tutorial1987/22/25 and
Alps11762/0/6 respectively;82/84/85/86/87 are absent in these captures and still
explicitly unavailable. Rectangle parallel-plane dispatch and two-sided normal
selection retain their original branches. Native fixture transforms are explicit;
actual host vector rotation uses LS3DF export1002EC80, fingerprinted in
sweep-vector-rotate-export.json. No DLL/original function was executed.

`dynamic_collision_sweep.h` implements00702810's required40/41/42 consumers,
coarse sphere/range filtering and Z/X traversal against actual bucket descriptors.
Other dynamic primitives remain unavailable. Sphere42 intentionally preserves
startY during initial overlap, uses normalY=.01 then, and clamps later hit normalY
at zero. These unusual original rules are not replaced with generic sphere physics.
`dynamic_collision_exclusions.h` implements006EFA70/006EFB10 against descriptor
MASK+08 bit8. Replacing a list clears previous bits, sets new bits, and updates
receiver410; descriptors must survive the exclusion window. Valid membership is
snapshotted to guard native array relocations. This is the shared query owner.

`human_movement_control.h` uses that concrete query for00763780/007634F0's
DD0+AC0 bind/reset.006FDA00's maximum goes to118; minimum only overwrites an
unused caller stack argument. The actual sentinel is C61C4066(-10000.099609375),
separate from integer-10000. Stance1 temporarily excludes the actual current
Human volume list, casts radius.2 downward1000 from modelY+1, applies squared
400/maximum-height gates and sets DD0+A90 only on the original failure branch.
The exclusion list restores even on an endpoint/unsupported-primitive exception.
Nonstance1 does not touch this query/exclusion state.

Focused sanitizer checks pass triangle face/edge/vertex, all six box directions,
rounded edges/corners/overlaps, finite/nearest travel, local/world/rotated rectangle,
static/dynamic composition and exclusions, and movement ground/no-ground/far-ground/
height/exception branches. Strict i386 instantiates concrete static/dynamic/query
owners and binds movement to the ACTUAL Human/frame/resource types (physical
control contract remains a fixture until DD0 is complete). Loading world and
actor dispatch still do not call these owners; no desktop/readiness claim.

The earlier note calling0075AE40→004336C0+90 a script dependency was wrong.
004336C0 is the existing client SOUND singleton; final vtable0080F7B8+90 resolves
to004324C0, already implemented as frontend::set_sound_voice_limit. It sets group3
limit4 on the SAME startup sound runtime. +5C is00437340 stop_sound_voice, also
already recovered. StartupSession/GameHumanAudioLoading now expose those real
owners; no duplicate runtime or new function credit. Mission script VM/context
work remains a separate overall P2.2 dependency. Physics model/morph/animation
and230 equipment receivers remain required before live Human construction.

## Face tables and physical loading now in source

`human_morph_tables.h` recovers00781B00's once-only `Tables\\morph.sav` read
through the SAME startup ChunkFiles. First coefficients read61F8, then metadataBE0:
95 records, coefficient stride108 and metadata stride20 (hex). BSS is initialized
by this loader, not read as PE bytes. Missing/open/read failure preserves the
original zero-table branch; malformed present lengths reject and retain failure.
The base Others.dta fixture is28120 bytes, SHA256
3d25678e15dbb1ba4ba3f586ef3a004b2e3905c061f8598a7c191eb884dcfc8f;
only indices19..37 are loaded. This fixture is not runtime override equivalence.

00782C80 initializes the25 twelve-byte bone names at0098A348 from actual PE
strings.00780850/00780880 initialize delay globals0098A340/0098A33C to3000/857
by truncating4200/float1.4 and1200/float1.4. Static-loading-data JSON preserves
source/destination addresses, names and binary fingerprint. No guessed BSS values.

`human_face_control.h` recovers the loading state of00780990/007809C0,
00780AD0's four ordered blend groups1/0/4/3 and00781250 blink scheduling on the
SAME CRT random stream. Only direct children are searched for visual SMMR;
marker f is borrowed. Metadata feature bytes are0D/0C/10/0F, respectively.
A selected unloaded record aborts all later groups; the second group skips
identical selectors. Force applies a zero-duration group at ratio1; nonzero
ratios still use their actual elapsed/duration, even when force is set. Fourth
blend decompilation misidentified stack locals; assembly confirms ordinary
70/74 ratio and second coefficient-table pointer. Host invokes the inline
morph receiver at frame120, rather than the visual's primary vtable. Runtime
speech/voice7C and state-transition producers remain separate unfinished work.

`human_physics_loading.h` supplies0075AE00 binding,0075AE40 scalar/audio/reset,
0075B280 loading descriptors and0075B870 shot spheres. Nine optional volumes
are snapshotted before audio group3 limit4 is set on the existing client SOUND
manager.004336C0/+90→004324C0 and+5C→00437340 are SOUND, not VM.
DD0+770 uses the SAME0077FD40 reset as E0. Unwritten scratch/SIMD padding and
native pointer words are excluded from scalar storage.

Four primary/four secondary sphere descriptors have flags42/mask101 and borrow
the real Human/model. Ordinary loading registers three ZERO primary descriptors
through the SAME dynamic buckets BEFORE filling centres/radii; no invented move
repairs that original order. Secondary descriptors start unregistered. Required
shot1/shot2 volumes supply two42000042/mask102 descriptors, filled before insert.
Signed HumanEC<1 halves their radii and shifts centres by normalized separation
*.4/+.15; relative centres are saved before adjustment. Original tiny direction
fallback and x87 X/Y/Z spill differences are retained. Missing standing volumes
would enter native negative descriptor indexing; reconstruction rejects this
malformed input and rolls back. Disabled collision skips standing registration
but still constructs actual shot data.

The25 borrowed bones are found in original order, face state is bound atDD0+610,
blink is scheduled, stand3-relative offset740 is saved, and four CRT draws use
moduli200/300/400/600 before incrementing009832B0. Query descriptors follow
0075C960's primary→shot→secondary order with native positive counts. Rollback
removes every actual registered descriptor and never increments a failed owner.
This dependency does not supply runtime posture changes, producing sound commands,
DD0 activation or its animation teardown; containing controls remain required.

`HumanSoundPool::give` adds00785400's actual two-slot return: tagEC identifies
(group,index), +38 unlinks the sound frame, +30 disables a valid variant, full or
invalid cache entries release ownership. +38 is a frame unlink, not a sound-stop
receiver. Actual adapter endpoints and the existing startup voice methods are
reused. No new sound manager or private random stream is constructed.

Focused ASan/UBSan pass95 morph records,68 transition pairs, once/missing/truncated
reads, blink timing, direct-child lookup and endpoint failures; physical loading
passes original registration order/shared buckets, health spheres, exclusions,
repeat/disabled loading, five missing volumes,99 frame/five registration failures.
New sound return passes two-slot saturation/invalid tags/null/detach failure.
Strict i386 instantiates actual HumanSceneActor/GameActorServices/SceneActorRuntime,
face/audio and real bucket types. Application compile/link passes; no Windows,
original, engine DLL or desktop execution. Ledger remains34 complete/559 partial.

Next animation evidence:00750710 is a thunk to007506E0, which resets DD0+CB0
10/14 and consumes rand()%8000+5000 at18;00750720 is an actual empty body.
00757E10 uses the compact native12-byte animation table, not serialized28-byte
stride: table65C is row135's animation pointer. Recover all needed CD0/C00
control, frame callback and shared presentation owners before activating DD0.

`human_presentation_textures.h` supplies0078A4F0's13 records/26 texture references
with original left/right suffixes and graphics flags402. Native mode bit2 gates
acquisition; partial failures roll back without readiness. Matching texture-tail
cleanup0078A5F0 releases records ascending, L then R. Actual runtime presentation
mark/model pools remain unfinished. `human_motion_state.h` supplies007506E0's
three scalar resets and shared rand()%8000+5000; binding four actual owner links
is still required. Focused checks pass all26 acquisition failures, texture names/
flags/ordered teardown, repeat/mode and motion timing. Strict actual i386 and
affected application compile/link pass. No whole presentation/DD0 claim.

Human230 holster loading/cleanup and state/selectors now have source and focused
verification. [Equipment contract and immediate geometry continuation](human-equipment-loading.md)
records80 definitions,10 sets,24 markers, native pose restoration and141 failures.


## Offline face geometry and shared resources — 2026-11-08

Mode7 is the required normal offline single-player path. `human_face_mesh_pool.h`,
`human_face_resources.h` and `human_face_model.h` reconstruct geometry62B0:
00796430/007967A0 shared initialization/close,007968A0/00796B90 mesh leases,
00796D80/00796E90/00797020 data ownership,00797A50/00797AA0 adjacency,
00797330 extrema,00796C90/00796D50 texture slots,00798200/00798250/007982C0
binding and00798E40/00799080/00799170/00799270/00799350/00799540 masks.

The source uses the SAME model134 resource key, case-sensitive reverse lookup,
DWORD references and swap-pop retirement. Each record owns seven0x128 native
material groups represented by recovered fields: owned nodes/edges, borrowed
endpoints and signed WORD triangle comparisons. Pairwise coincidence preserves
reverse node order and x87 comparison semantics, including unordered values;
threshold0080F520 is float1e-8, not a guessed geometric epsilon. Extrema are node
indices at30/34/4C/3C/6C/AC. Zero-triangle groups would dereference no native
node and explicitly fail in the reconstruction. Vertex reads lock actual LOD0
once across all seven groups; actual engine triangles are WORD triplets.

Shared0098ADF0 owns base stainNull.bmp/stainNullA.bmp flags200004, hshadow.bmp/
hshadowA.bmp flags200084, eight camouflage A/B/C/D textures (two RGB variants)
and eight slots of seven stain textures. Driver state13 is saved, set0 while
loading all66 textures, then restored. Missing required texture acquisition fails
and releases partial ownership. Slot reuse searches existing borrower, then first
empty, then strict unsigned minimum epoch; UINT_MAX-only occupancy produces no
slot. Eviction restores actual shared materials before replacing the borrower.
The containing world must initialize this owner before Humans and retire Humans
before its007967A0 clear; no independent fake texture service is supplied.

`human_face_patterns.h` contains exact section-mapped WORD pixels from the original
PE, encoded as constexpr runs. Eleven256-word stamps at0084FEE8..00851338 and
nine4096-word overlays at00851540..00861580 retain all authored values;8-byte
inter-table padding is excluded. These are native constants, not generated art.
`human-face-pattern-evidence.json` records all20 addresses, hashes and encoding.
The compiled C++ arrays were compared with hashes independently read from the
original PE in a temporary native sanitizer fixture.

Important binding correction:00785B20 passes operation0 to0079BD30, forwarded
through0079C320 to007982C0. This is NOT an unconditional operation2. Initial0
prepares seven masks and scratch;2 releases/reacquires pooled data and only
reuploads already private masks.1 refreshes private textures. UV allocation uses
actual LOD4C; copies vertex24/28 into8-byte pairs and calls LOD38(0,0,0), distinct
from the mesh-cache read lock's08 release. VisualA0/mesh18 precede UV work;
LOD64 receives native flags and visualC4 follows material setup.

Camouflage uses actual Human19C==1 and mission0098A6B0. Its shared WORD counter
increments before mapping low2bits into1..4 or5..8. Clothing comes from actual
holder0 entry virtual2C; >.1 enables shadow.00799350 refuses shadow changes while
variant59 is nonzero. Both overlays preserve exact4096-pixel ARGB4444 alpha-max,
alpha-weighted RGB arithmetic. Scratch restore, shared-material2C/30(6)/34(5),
private texture14 lock/18 unlock and contiguous height*pitch copy match source.
An oversized engine pitch explicitly fails instead of reading beyond its mask.
Partial slot acquisition is also retired during reconstruction failure cleanup.

`HumanModelGeometry` now owns the actual face object before retained camera
retirement and routes007B85A0 to it. Its offline binding requires initialized
shared resources, actual current quality-byte3 and Human/clothing data.
`HumanControlLoading::bind` supplies typed Human19C/holder0 values and requires
world services for the real shared owner, profile flag and mission camouflage.
`StartupSession::human_face_quality_enabled` reads SAME selected profile each
call. No world service supplies a successful default. Quality-disabled mode7
legitimately skips face construction and clothing reads.

Focused ASan/UBSan covers mesh adjacency/cache/refcounts/failed groups and all20
compiled pixel arrays, masks/variants/slots/rebind/failure cleanup. Strict i386
instantiates face resources/model and geometry with actual GameSceneActors.
The full DD0/world services remain uncomposed; this is source recovery, not a
loaded Human. App compile/link passes; receipt human-offline-face-verification-
wip.json fingerprints/removes temporary artifacts. No ledger or desktop credit.
Evidence: human-offline-face-music/,human-offline-face-dependencies/,
human-offline-face-pool/,human-offline-face-adjacency/ and saved face asm under
work/mission-world-20261103. All Ghidra exports completed without ERROR.
