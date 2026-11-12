# Human reactions and ragdoll source

P22-HUM's second plan checkbox remains open. Current work continues the same
body/point owners. Application mission loading and desktop acceptance remain open.

Current status and remaining dependency order are maintained in the
[revised Human plan](p22-hum-plan.md), through b132e6d. S01–S14 identify verified
source checkpoints; D0–D12 identify remaining composition/producer dependencies.
Earlier “in progress”, “continue” and “not yet bound” notes below are dated
research snapshots, superseded by later checkpoints. In particular007B3C80,
007B37C0→00787110,0074D510 and Car fuel10/impulse11 are source-recovered; their
containing-path/application acceptance is still open. Do not restart these
kernels from an earlier snapshot. Deferred joint/clip inventory reconciliation
belongs to D12 and does not imply that their implemented bodies are absent.

## Original evidence

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Static evidence is retained under `analysis/work/mission-world-20261103`.
The rollback-only `ExportHumanRagdollABI.java` checks that hash, uses the existing
read-only Ghidra project and never saves temporary calling-convention changes.

- `human-ragdoll-workers-abi` holds007CD840,007CDA90,007CDC90,007C58B0,
  007C77E0 and007B3C80.
- `human-ragdoll-step-abi` holds the initial solver dependencies.
- `human-ragdoll-constraints-final-abi` corrects vector constructor/add/scale,
  matrix RotateZ and vector RotateByMatrix hidden-result ABIs. Earlier constraint
  exports retain incorrect engine signatures and require the final export/raw
  instructions for interpretation.
- `human-ragdoll-distance-abi` holds007CE070 and007CB5D0. The former takes two
  register arguments and six stack arguments. The latter also retains its
  implicit geometry receiver.
- `human-ragdoll-contact-lifetime-abi` holds007CDE00,007CDF10,004F2790 and00787110.
  **6914/6918 are actual SOUND voice handles.** Their manager is004336C0 and
  their request constructor is004374D0. They share the existing sound manager.
- Raw instruction files are `human-ragdoll-step-workers.asm`,
  `human-impulse-consumption.asm`, `human-body-publication.asm`,
  `human-ragdoll-integration.asm`, `human-ragdoll-constraints.asm`,
  `human-ragdoll-postpass.asm`, `human-ragdoll-sibling-constraint.asm`,
  `human-ragdoll-segment-distance.asm` and `human-ragdoll-runtime-contacts.asm`.

## Source progress

The existing HumanLoadingJoints/HumanJointOwners now supply impulse consumption
007CD730, contact-frame motion007CD840, interpolation snapshot007CDA90,
interpolation007CDC90 and body/bone publication007C59C0/007C5A90/007C58B0.
The queued duration at native20 is decremented. Native24 remains unchanged.
The confirmed secondary producer still writes the primary queue's duration,
leaving its own duration zero. Positive elapsed consumption therefore expires
that secondary entry. No correction to that original behavior is introduced.

Further source supplies point integration007C79F0, material restitution007CDFC0,
endpoint/frame rebuild007C80E0, weighted correction publication007CBAE0,
length007CA390, angular007CA470 and sibling007CB510/007CB5D0 constraints,
segment projection007CE070 and containing constraint dispatch007C8B10.
The body1 transverse constraint updates the same shoulder/hip correction slots.

The actual point007CBF90 and body007CC740 collision consumers now perform their
World sphere queries, static/dynamic contact publication, retained frame changes
and positive004F2790 Car impulse forwarding.007CBD50 preserves the all-points,
all-bodies, then smoothed-motion publication sequence.004F2790 reuses actual Car
motion, inertia and settling state, including its independent suppression bits.
007CDE00/007CDF10 use the existing SOUND pool, with audio retirement after camera
references in0079BAC0. The geometry borrows the actual control audio lifetime.
Cleanup scans actual retained point frames if a C++ failure interrupts690C
publication, preventing reference loss before either tree is destroyed.

007CD230 now uses the SAME process HumanPhysicsGlobals for0098AF84/0098AF88.
The second native decay reuses the first decay's modified shared scratch. The
actual control supplies this process owner to geometry.007C77E0 now supplies
native iteration priority, activity rescaling, impulse/integration/constraint
ordering, odd/final collision cadence, sleep and timing publication. Its geometry
adapter binds the actual collision and sleep consumers. The6740 surface producer now calls the actual007B37C0→00787110 chain described below.
Descendant bodies subsequently clear their first-point angular flags in native
order. Original fixed neighbours and existing body/point ownership remain shared.

Native6910 is an upward-contact byte count. The source now represents its full
byte range. SIMD padding without a defined native producer retains a known
input/prior lane. Missing required source pointers produce source errors at
native undefined-dereference boundaries. Native focused checks use explicit
engine-math doubles and establish source invariants, with no original solver
convergence/equivalence or desktop claim.

## Verification in progress

`human-ragdoll-retained-native.cpp` is a temporary ASan/UBSan/LSan fixture.
Current native checks pass, including duration expiry, contact release/wake,
free/contact motion, restitution, upward cap, interpolation, publication,
endpoint rebuild, body constraints, positive torso twist, sibling overlap,
parallel/zero-segment output ordering, weighted corrections and repeated cleanup.
The temporary strict i386 fixture additionally instantiates these receivers
against actual World, control, frame, sound and Car bindings. Strict actual World/control/frame/sound/Car i386 instantiation passes in
`human-ragdoll-host-i386.log`, including the collision, sleep and substep adapters.
Launcher compile/link passes in `human-ragdoll-application-i386.log`. The native substep fixture checks counts3..6, flag priority,
required collision/sleep invocation and interrupted-step state with explicit
callback doubles. It provides no complete mission/trajectory equivalence claim.
No function-ledger/tally credit has been added for these unfinished checkbox
changes. Consolidate credit, receipts and temporary-source removal at completion.

## Earlier dependency snapshot — superseded by later checkpoints

The original continuation scope was007A7470,007B3C80,007B0910,0074D510 and
positive obstruction/contact/fall damage. Later sections below record their
source recovery. Use the current [D0–D12 plan](p22-hum-plan.md#h2-remaining-dependencies-and-exit-tests)
for remaining work rather than treating this historical list as unchecked.

007CDE00 emits flags5, source Human, relative bone22/group0/entry44 for6914.
For6918 it emits relative bone4/group1/entry55 when geometry62B8 is even,
otherwise publishes-2. Both use default voice group4.007CDF10 looks up the
actual voice frame and calls its78(0). It stops only an existing inactive frame,
then publishes-1 for6914 or-2 for6918.0079BAC0 retires these SOUND handles after
camera references, then resets both to-1. Existing StartupSession/SoundServices
already provide the manager and actual sound_active boundary. Preserve that
lifetime when adding emission.

007B37C0 requests the retained blood/surface effect through00787110.
00787110 consumes a CRT draw, performs the actual clothing ray where required,
then invokes0044F2B0's bounded surface-effect pool and0078AC30's retained position
queue. Their exports are in `human-ragdoll-surface-abi`. Recover their actual
producers/consumers before classifying this as the separate breath/blood particle
checkbox.0079BD30's retained export confirms the native6740 reset.


## Session continuation — stopped by user on2026-11-11

### Latest reaction-entry and clip source

`HumanModelGeometry::begin_geometry_effect` now contains the expanded007B0910
entry. The existing mounted entry delegates with its actual flags8/reason4/source.
`HumanControlLoading` exposes the same bound entry for the remaining reaction
callers.0074D510 has not yet been bound to it.

The entry uses original mode/quality/animation/attachment and camera gates. The
retained-clip branch captures all23 actual local/world matrices, resets both
cursor arrays and time, and captures native6810/6820/6830 state. Reason3 allocates
seven retained contact positions for bones6/4/8/13/9/2/3. Reason2 allocates none.
The other retained-clip branch projects three contact positions with the native
model-up offset. Clips remain borrowed, following the original resource lifetime.
Source scratch padding without a native producer retains a known lane or the
source container's initialized lane.

The joint branch preserves the existing saved-pose restoration and graph. It now
initializes prior-position velocity, its horizontal speed cap and signed elapsed
scale, water/gravity flags, source-null unavailable-limb retirement and original
impulse branches. `initialize_effect_points` replaces the mounted-only helper.
`retire_unavailable_effect_limbs` removes actual body3/4 and5/6 with point5/6
and8/9, through existing owner destruction/detachment.

Raw007B2B8A establishes the explosion flag4000 branch, which older decompilation
lost through alias damage. It normalizes the supplied vector, scales by.2 and
caps450, adds the native one-third vertical component and queues200ms. The other
branch preserves the backward/down-ray projection, .2222222238779068 scale,
150 cap, low combined-motion secondary holds,30 rescale and250ms dispatch.
These branches need the final containing-path audit. Ordinary source loading
currently requires all23 bones, so the native missing-bone1 impulse gate is not
reached by the normal cold fixture.

`apply_human_ragdoll_clip_pose` in `human_clip_pose.h` supplies007B7410. It reuses
the existing key cursor sampler and extracted position interpolation arithmetic.
Both key pairs precede three actual world-matrix reads. Rotation requires basis
square strictly greater than epsilon. Direct parent-position/quaternion endpoints
preserve native flags, quaternion normalization and position lock. There is no
per-bone24 call. The native influence parameter is unused except recursion.
`human_joint_frame_basis_square` shares the repeated reads/spills with007B0910's
existing leaf-body gate. The containing007B3C80 clock/model24 and collision path
remain pending.

`clear()` now retires the retained geometry effect before face/camera owners,
following0079BAC0's6840 position. The existing frame cleanup scan still protects
partial contact publication. These are typed source lifetime adaptations.

### Latest verification and temporary files

Native `human-ragdoll-retained-native.cpp` and its executable remain in WORK.
Latest compile/run logs pass. The extended fixture checks direct clip publication,
last-key/cursor advancement, independent position/rotation gates, no per-bone24,
retained reason1/2/3 setup, mode rejection, full-joint setup, source-null velocity,
unavailable limb removal/rebuild and interrupted query restoration. Existing
solver/contact/Car/sleep/substep checks still pass.

For reaction setup only, the temporary fixture uses `-fno-access-control` to seed
known containing model/Human/control/skeleton/camera fields. It does not call
actual geometry loading. Engine matrices, Slerp/normalization and queries are
explicit doubles. Preserve that limitation in the eventual receipt.

Actual World i386 `human-ragdoll-host-i386.cpp`/`.o` and its empty PASS log include
`control.begin_geometry_effect` and the new clip sampler. The reaction/application build now passes through clip and reaction selection; the later damage additions are rebuilding in `human-damage-application-i386.log`. No original/Windows program has run. No new tally credit is added.

Native compile now requires the additional temporary-fixture compiler option.

```sh
g++ -std=c++17 -O0 -Wall -Wextra -Wpedantic -Werror -fno-access-control -fsanitize=address,undefined -fno-omit-frame-pointer -g -Irecovered analysis/work/mission-world-20261103/human-ragdoll-retained-native.cpp -L.tools/hd2-sanitizer-runtime/usr/lib64 -Wl,-rpath,"$PWD/.tools/hd2-sanitizer-runtime/usr/lib64" -o analysis/work/mission-world-20261103/human-ragdoll-retained-native
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 analysis/work/mission-world-20261103/human-ragdoll-retained-native
```

LSan execution used approved unsandboxed native execution. Ghidra and git writes
also previously used their narrowly scoped sandbox escalations. Do not turn
those implementation details into a new user approval prerequisite.

### Retained clips, actual frame dispatch and rest surfaces

007B3C80 now advances the signed1.5x retained clock, clamps against actual clip
duration, calls007B7410 and updates the model. Reason3 performs all three paired
sweep/floor checks over its seven cached positions. A collision invokes007B0910
on the SAME effect and returns immediately without retaining an invalidated
reference. Other non2 reasons advance the three projected points on odd62B8,
with gravity, the original two sweep iterations, attachment filtering, downward
contacts, orientation interpolation, parent conversion and model publication.
The lower reason3 centre deliberately retains the original weights summing1.2.
Expiry uses strict duration+300 and duration+10000 tests, calls actual00786E50,
and preserves the native common query restore even when no exclusion occurred.
Player camera movement uses the actual FPV target and collision query.

The007A7470 ragdoll branch now calls the actual geometry owner. Full-joint
presentation retains the integer68E8 countdown,100ms step ordering, body
snapshot/interpolation/publication, Human position, actual Scene144 refresh,
shot/light refresh and actual contact/audio retirement. The source Human is
excluded after self and restored after self, using existing inline/DD0 owners.
Primary C++ failures survive secondary query-restoration failures, which are
recorded through the actual cleanup recorder.

007B37C0 samples the actual points0/1/2 and body0/1 radii, chooses/project-normalizes
the contact normal, uses one shared CRT draw and the original quadratic weights.
Raw004CE5C0 is **vector subtraction**, so the radius offset subtracts the normal.
DD000787110 preserves its mode/quality/draw/ray ordering, real184/188 borrower,
and queue publication even when emission is quality-disabled. Kind184/188 has a
typed0075D1A0 setter. Later reference evidence below corrects the initial
authored-producer hypothesis:00761690 restores saved DD0 state.

0089A9B0 is the SAME process ShotMgr already owned by StartupSession, proved by
its0044D900 static initializer→0044D920/module14 registration. It is not Effects.
Its native A0/A4/A8 surface collection now owns actual004546E0/00454940 projection
frames, with texture60=`x_blood_05.tga`, Scene links, light, independent engine
random draws, strict one-step100ms growth and00454C40 retirement. Maximum150
means **field+150**, whose real setup is50 offline7/20 otherwise, not capacity150.
The retained position queue0078AC30/0078AD00 shares existing first/last Human
presentation lifetime and its allocation grows by8. Its actual footprint
consumer0075D210 is now connected, as detailed below.

### Reaction selection and owner correction

**0074D510 receives DD0+AA0, not Look.** The containing call0077DEDD..0077DEE9
loads Look+A4 then addsAA0. Existing HumanControlLinks already owns these links.
Look is the command source. Look+A4/A8 are DD0 bindings; its first word is a
separate borrowed death-source actor set by0077FBA0. The source now keeps these
identities separate and resets the borrowed death source with Look reset.
Human+A8, in contrast, is inline98's230 **equipment** borrower. Its+CC=2 reaction
write belongs to equipment; it is not the CB0 motion-timer owner.

0074D510 now selects the actual queued animation/posture, shared process
009831A0 sequence and0084E744 last mounted reaction. It preserves flag priority,
CRT draws, three shape attempts, original lateral rejection, marker/posture
publication, motion hold00773AD0, exact command/audio/equipment retirement,
shot descriptors, retained clip resource and final007B0910 invocation.0074C6E0
uses the actual posture spheres in reverse and the engine AngleTo endpoint.
0077DE30 now calls this body through the existing control instead of Runtime.

Reached seat removal uses actual Car occupants/mass/cameras/controls and
StaticWeapon seats/control/collision descriptor. Car004D9A40/004D9FC0 and
StaticWeapon00604630/00604A10 preserve signed bounds and existing owner identity.
00604630 is absent frozen and receives no new credit.004C6DD0(NULL) clears the
actual mounted occupant and view. Positive movement SOUND frames now use the actual authored producer/lifetime
described below; no raw DWORD pointer is dispatched. The reached destroyed-Car
00518710(false) transition is also in source; running simulation remains separate.

### Containing damage source in progress

`human_damage.h` reconstructs005A1C90's controller: offline protection/debug and
integrity observations, exact difficulty/hit-kind damage arithmetic, real hit
commands, corpse impulses, wrapping health, limb limits, death-source/credit/
Script ordering, B0 notification and dubbing/speech ordering. It does not provide
a successful health-only fallback. Positive missing receivers still throw at
their native ordering boundary. Obstruction, materials, fall, floor and actual
Car impacts now call it through the SAME containing Human/World lifetime.
Floor damage is native **kind6,1000,zero point/impulse,NULL source**, not kind14.
005A3CC0 has no stack arguments: the caller retains the later D4 arguments on
the stack. It selects an actual bone from visual **a** (00839100), not z.
The temporary damage ABI export incorrectly adds stack parameters to005A3CC0;
use raw `human-damage-full.asm` and the source contract instead.

005A0BE0 now uses actual offline experience/stat and perception owners, current
B0 enemy-killed receivers, then lost/acquired Sight notifications.005A9480 writes
HumanD0, separately from D4 stealth.00573FB0 queues10000003 on the existing first
command queue; Player00542300 requests actual speech, and base/Remote/Active use
their genuine RET4 bodies.0041D920 now dispatches through the existing Script
context/runtime borrower, allowing the original ReturnEnd side effects.

00451B00 is an actual **SOUND request**, not a blood-particle producer. The SAME
ShotMgr D0 request and material definition30 feed the existing SOUND manager.
0044F4D0→0044F520 now uses ShotMgr's actual90/94/98 collection and texture4C
`x_blood_00.tga`.00454C60/00454E20 own the projection frame and motion;
00455240 preserves strict400/1000ms tests, real World rays and ignored return2.
00454FF0 publishes the collided projection, actual light/links and engine random
scale/roll.00455350 retirement precedes A0 records/textures. World query/light
services are borrowed for these records; native module finish must run before
those containing services unbind. Required particle00451C00→00445150 remains
open and is distinct from these two projection-record collections.

### Damage state, inventory and death continuation

0053DB90/00574560 now build the actual kind1 damage command in B0's SECOND90
queue, then preserve seat protection, head/hand drops and current-cache clearing.
Player uses fixed2 speed plusY1; AI uses an independently stored engine draw+2
plusY1. Dead-source10000016 merges into the actual second AI event queue with
50000 lifetime; live kind5 emits40000015 to the first.00590400 retains the
Script interrupt after the common notification. The original005B1540 serializer
is absent frozen: rollback-only research defines it for decompilation, with no
function credit. The writer supports actual command1 bit grammar and packet
rollback, alongside existing command6; other command kinds remain required.

005A3490→00478EE0 now uses markerj/flags400, the actual hand voice, identity
lookup in THIS Human,005A7B70 removal and the SAME type11 runtime registry and
DynamicItemActor binding. Items owns the entry throughout. Removal precedes
runtime creation.005A7B70 preserves holder3/0/2/4/5 priority and reverse entries
within each ascending stack; the last matching stack wins. General007DC2A0
retirement clears holder_actor, notifies actual HUD, subtracts item/pack-content
weight, detaches without deleting, then clears the borrowed pack6 pointer for
kind17.007DBD80 retires actual230 borrowers and places the model before optional
Items deletion. Reload consumption now shares that body instead of another
ammunition-only implementation. Positive nonoffline00588EF0 remains required
AFTER the real transfer, at the original publication boundary.

Raw00791F46/1FA9/200C passes **marker+40 itself**, not its parent: localB0 plus
the existing process equipment offset is transformed by the marker's WORLD
matrix, including its translation. Preserve this unusual arithmetic. Raw
005A3537's release belongs to the diagnostic string in the reused stack slot;
it is not a release of borrowed markerj.

005836E0 now preserves conveyance type4/20/21/34 skip, offline hand-only drop
(excluding definitionkind9), direction+up normalization and engine draw+2,
then dead flag800, actual music-reference retirement and pending-action
cancellation. All eight B0 tables point virtual0C to00584E30. Its failure-only
instantiation omits success-only runtime calls while preserving the actual
command/target/slot mutations.0053BD00's Player tail clears the existing FPV
weapon and resets Player mode; its positive nonoffline objective branch remains
required and is absent frozen.

005A0220 now clears Human154/204, calls actual currentB084, forces Look71 via
005ABBF0, then proximity/Players/Sight/hearing, roster death notification, Car
controls and reverse formation notifications before005A9120 retirement.
0045E790 releases the victim's actual hostile selections, retires its HUD marker
without a flags gate, clears previous30 when matched, and uses the SAME Script
world link/objective owner for0069BED0(false) when no eligible roster member
lives. SOUND+68 is **00436F60 secondary movie/music selection7**, not a voice
group stop; it uses the existing Startup channel state.

Reached Car004F2100/004F2270/004F23B0/004F26A0 request bodies preserve the real
input6FC/700 words separately from filtered throttle, shared724/7C4 ramp state,
8E4 brake,908 desired clutch and710/740/22C7 emergency brake state. The ordinary
pedal dead zone is FLOAT.05; clutch uses0. The actual initialized Car owns these
requests. Wider occupied/running simulation still has its existing required
boundary, and has not been relabeled complete.

Player00545DD0 now perturbs B0+10 with three independent engine draws, exact
easy/conditioning amplitudes, retained-Z arithmetic and four-word00583120
publication (absent frozen).0045F110 uses the actual reverse roster and strict
30m distance to move nearby Active AI14/15 modes into16/17.0049F0B0→004AD380
now borrows the existing seven IShow owners: stop previous, CRT%7 selection,
first-material texture, nonlooping play, then update and reset timers. A failed
open slot preserves native enabled visibility; callback failures retain the
actual owner for ordered cleanup. This does not supply the still-required full
HUD frame update or application/desktop integration.

### Shared particle and actual footprint checkpoint

Module17 now owns positive004451B0 active records and separate00445AF0 placed
records through the SAME Startup process, definition table,256 reuse buckets,
008998C4 point pool and shared008998DC/E0/E4 depth scratch.00440452 explicitly
sets the refill count to1024. Every active/cached visual and point lease retires
before its Frames/Scene borrowers.004455B0 rereads pause for every row;2C/2D
continue ordinary updates while other active instances only publish geometry.
The initialized finish order is active ascending, buckets descending, definitions,
identity=-1, placed ascending, latch clear. Source rollback also retires acquired
pre-activation placed owners; it does not advertise a successful partial load.

00440530/004406A0 own the actual EMTR visual.00440800 loads the actual texture
and initial mode, timers and bursts;00440B20 retains texture/definition on reuse.
00440D80 unlinks, releases its parent and hides.0043F520/0043F730/0043FB40 retain
ordered engine draws, lifetime clamping, phase advance and sprite timing.
004410B0 preserves the single initial500-point gate, integer-versus-float rate
reset, repeat/wait state and spawn preadvance before subtracting each period.
Expired points are returned after publication; the final empty instance expires
on its next tick. Sprite UVs are the emitter+0C table (historical source names
`colour_fields`/`colour_keys`), separate from phase RGB/HSV values.

00441460 uses actual Scene camera depth fields158/168/178/188 and EMTR inverse130.
Depth keys sort as stable UNSIGNED words;006E4910's radix implementation receives
no reconstruction credit.00441A00 writes actual24-byte XYZ/colour/UV vertices,
including native reversed sorted indices,500-row cap, camera billboarding,
orientation/roll, lighting and UV order.004427A0 preserves epoch, simulation,
bounds, publication and point cleanup;00442860 does no elapsed simulation.
All engine frame/texture/matrix/publication calls use actual host ABI endpoints.

00445150 builds Identity/SetDir3 and copies XYZ before actual process creation.
Its final two arguments are orientation then record flag. Damage00451C00 uses
actual Camera68 BEFORE64, strict dot>-.05, and material+3C with orientation1,
flag0. Breath007507A0 supplies orientation1/flag1 and its actual moving bone
parent. Bleeding0044F4F0 draws CRT%3 to choose textures50/54/58 BEFORE shared
0044F520 quality/texture gates; damage0044F4D0 separately uses texture4C. Both
use SAME ShotMgr90 records, real World rays/light and original module lifetime.

Rest-surface positions now reach0075D210 via actual track events4/5/6. The body
preserves foot selection, signed230 drift, actual ground query/exclusion, material
SOUND and scuff timing, far-scale/volume rules, FPV local direction and smoke42.
Retained blood positions set230=10 and the native material substitution selects
one of13 existing texture pairs; negative carryover selects the other native
keys. Alpha is intentionally not clamped. DD0+BCC belongs to **movement+10C**,
not the physical0..AA0 word owner.0078A4B0/0078AA10 acquire/publish through the
SAME projection free pool and primary list, retaining time/HumanID/count/frame.
Prepare/alpha/colour precede hiding the Human, then actual Scene/PROJ transforms,
projection and mode3 append, then Human visibility restoration. C++ failure guards
return the payload and undo acquired scene/count state, preserving the first
failure. Existing first/last-user and Items process cleanup remain authoritative.
Track-event footsteps also preserve the separate CD0+CC timer and pooled sounds.

**Correction to the earlier producer hypothesis:**00761690 root14D child2/3
restores184/188; it is reached only through00784310 at0078447B, whose caller is
Human persistence005AB7A0. The focused reference scan found no references to
standalone0075D1A0. The DD0 range writes184/188 only in normal reset0075B08A/94,
the unused setter, and restore00761A67/00761B03/0E. Thus normal mission reaction
starts at184=5/188=NULL; there is no evidenced missing authored loader to invent.
The complete nested persistence parser remains a separate P2.6 dependency.
Evidence: `human-surface-producer-references.json`,
`human-dd0-surface-producer-range.asm`, and the existing P2.6 actor-state contract.

### Verification and next work

The [particle/surface checkpoint](checkpoints/p2.2-human-particle-surface-20261111.json)
records source hashes, focused audit hashes and28 added frozen client partials,
with two existing partials extended and zero complete promotions. Totals are34
complete/2528 partial. The accumulated reaction/damage bodies are preserved in
this source checkpoint but receive no new ledger credit yet. The second Human
checkbox remains OPEN; reached breath/blood particle source dependency is closed.

New native ASan/UBSan/LSan passes in ignored WORK:
`human-particle-point-native.log`, `human-particle-instance-native.log`,
`human-particle-module-native.log`, `human-footstep-native.log` and the rebuilt
shared `human-blood-native.log`. These exercise actual source classes with
explicit engine/query doubles. Module checks include paused exceptions, reuse,
placed owners and partial-load cleanup. Footprints cover actual pool/list,
native sound flags, surface carryover/alpha and failed-publication ownership.
Earlier focused reaction/inventory/death audits remain as listed above; no broad
re-audit was repeated. Strict actual World/control/particle/footstep i386 compile
passes in `human-ragdoll-host-i386.log`. Latest launcher compile/link passes in
`human-particle-footstep-application-i386.log`. No Windows output was executed.
Temporary audits remain only for this ongoing full-checkbox continuation.

Next work continues the SAME reactions/damage/ragdoll checkbox:

1. Finish005A2FE0 nonoffline inventory retirement and actual00588EF0 kind10
   SECOND90 command serialization/receiver. New repaired exports are in
   `human-damage-network-inventory-abi`; index lookups preceded the export.
2. Correct the earlier packet labels:0072EE60 mutates actual session player/team
   scores and publishes updates;005A0DA0 selects localized hit/death messages.
   Neither is simply a damage-packet leaf. Their actual session/HUD owners and
   nonoffline policy/auxiliary Players/Player-objective dependencies remain.
3. Close reached containing frame/damage dispatch, required failure/re-entry and
   teardown through the full selected checkbox. Commit completed checkpoints.
   No successful missing transport, configuration, score or text substitutes.

Application mission integration and user desktop acceptance remain open.


### Damage presentation and scoring checkpoint — 2026-11-11

Continuation now runs in `/home/control/Documents/Projects/H&D2 Decomp 4`, at the
user's explicit request. All22 pending Human files were transferred byte-for-byte
from the temporary continuation; main's newer GUARD changes were retained.
Before/after hashes and copies are in ignored WORK
`human-return-main-20261111`. The temporary checkout is an inactive backup.

005A2FE0 now traverses the SAME real hand3, shoulder2, pack6 and holsters1 in
native order, preserving ammunition and four engine draws per item. Its loaded
ammunition argument is FLOAT (raw005A3160), not the old C export's int alias.
Actual drops publish00588EF0 kind10 to B0 SECOND90 after runtime transfer;
005B1F10/005B2020 preserve signed identities, direction, optional actor and4-bit
speed grammar. Client005A3750 reuses inventory/model and SAME type11 runtime;
it does not produce a second authority drop or authority-only lifetime tail.

005A15D0 is now the real received-damage path, separate from005A1C90 authority.
Packet-relative hit points remain relative while cosmetic effects add model XYZ.
Presentation uses virtual88, not authority84: seven real B0 tables target00583940
and Player targets0053BE10. Presented death flags/cancels action without authority
inventory/music retirement. Player clears actual Car pedal, FPV/Human state and
uses actual respawn/config/session fields for injury text. The four voices in
00518710(false) are observed before its early return, then real81-slot stop,
selected stop voice, four seek calls and native scalar publication. Destroyed
Car seat removal now uses this path. Start/running Car simulation remains separate.

Nonoffline005A1C90 policy now uses actual configuration224,244,250,254 and source
team before victim team; signed protection time and reflection order are retained.
005A0DA0 now selects real zone/hit/death captions and colours using SOURCE's
local-player predicate. Names use actual HumanStore/model or session record name,
never a copied display roster. The original unknown-zone literal is
`Unknow damage zone !!!!` at0083C718. Damage text retains both formatter passes;
undefined varargs, overflow and missing native owners reject explicitly.
Nonrendered0069BF70 reaches a **console window/process**, not packet transport;
actual0069F1E0/00744870 ownership is still required on that branch.

0072EE60 now mutates SAME process player40..60 and three team464 records under
418; team FLOAT scores44C are separate. These fields lie past the original .data
raw extent and are loader-zero-filled.0072E7E0 resets nine player words and7C
under lock, then teams unlocked; it does NOT reset B8.0072FD60 uses actual10..80
thresholds at00844F94 and appends005470B0 kind16 before publishing record54.
Friendly-kill peer removal happens after unlock. Actual application11/0072A0B0
transport remains required when a real peer must be removed. The native remote-AI
source==record-count bug is rejected before an out-of-bounds access.

ControlStartupRecord now carries its native3C Human as a borrowed typed source
pointer, cleared by the existing initializer/destructor path. There is no DWORD
pointer fabrication or second owner. **The ordinary nonoffline00612A60 peer-slot
writeback producer remains open**; peer slot and Scene2 actor identity are distinct.
Rank command005B0E50/005B0EC0 reads4 bits; serializer005B0E70 is absent frozen.
Player command16 writes actual54 before006A1CB0 stats and0072FAC0 text; the resulting
equal-rank green message is original behavior.005A96A0 copies only indices2,1,4,6,7,8
in native order to current/max arrays.00725B00 uses actual holder4/definition44
when style0 and real3C exists. Remote's different old-rank path is not supplied
by this Player receiver. Independent PACK source already implements broader
wire/discard/Remote notification contracts; reuse it at the next integration.

0045FE10 releases the actual auxiliary member's reverse hostile list. Native
auxiliary+28/+2C records are the SAME selection record shape used by0045E410/
0045E540, proven by005A0220/0045FE10 instruction receivers. The previous opaque
raw vector is replaced by that actual owner, including its shared counters;
post-loading preserves the roster, reset clears it, and both retire selections.
No parallel auxiliary selection or fabricated producer is introduced.

New focused native ASan/UBSan/LSan logs PASS: `human-death-inventory-native.log`,
`human-damage-presentation-native.log`, `human-damage-policy-native.log`,
`human-damage-messages-native.log`, `human-damage-score-native.log`,
`human-score-presentation-native.log`, `human-auxiliary-selection-native.log`,
`human-car-engine-stop-native.log`; actual state/damage audit also passes.
Engine/Scene/queries/platform endpoints remain explicit doubles. The Car stop
fixture seeds actual sound slots with access-control disabled; it does not prove
resource loading. Strict actual World i386 passes in
`human-damage-score-presentation-host-i386.log`; main launcher compile/link passes
in `human-damage-score-main-application-i386.log`. Windows outputs were not run.
[Checkpoint receipt](checkpoints/p2.2-human-damage-presentation-20261111.json).

Next: integrate existing PACK codecs/receiver lifetime checks and bind these real
damage/drop/rank effects; retain Remote-specific ordering. Continue Player550
objective/kind12 metadata and actual nonoffline session/console/peer dependencies,
then containing frame/failure/re-entry/retirement. Reactions/damage/ragdoll and
P2.2 remain OPEN. Application loading and user desktop acceptance remain pending.


**PACK integration discrepancy — exact drop writer:** PACK's zero-multiplier
claim is incorrect. Let S be ESP before005B20AB.005B2099 writes[S+0C]=0;
005B20A7 writes[S+08]=mask[4]=15. PUSH at005B20AB makes ESP=S-4, so005B20B2
FIMUL[ESP+0C] reads[S+08], **15**. The current `(speed-1)*.25*15` writer matches
the original.008119A0 is DOUBLE8, making its calculated width4; the actual mask
at00816398 is15. Keep this corrected behavior when importing PACK source.
Evidence: `human-drop-command-write.asm`, `human-drop-writer-stack-check.json`.
No new export, absent-function credit or original execution was needed.


### Shared PACK effect integration — 2026-11-11

Reused pinned `feature/p22-pack`31b8786 source for23 value codecs, exact
Player/Remote/discard grammars, Scene2/B0 re-resolution, actual input/inventory
queue producers, late inventory/equipment snapshots and notification14. Its
worktree/history remain untouched. This is a source dependency import, not a
merge of PACK's plan/ledger or a claim that its missing positive effects are done.

B0 FIRST80/SECOND90 now own ONE HumanPacketCommand value type. Existing actual
kind1 damage,10 drop and16 rank producers fill that same type; the former optional
parallel payload fields are retired. The containing HumanWorld services bind
actual005A15D0/005A3490/005A3750/005A1C90 and rank effects. Original command17
supplies kind/amount/source/hit with zero point/impulse. Authority still rejects
unimplemented nonoffline forwarding. Nonoffline presentation supports these real
effects; other reached mode-specific gaps retain required failures. Remote16
uses its previous rank for stats before final publication, while Player writes54
first. Module13 routes through the same checked grammar, rechecking Scene2/B0
between every command. Owned decoded values never retain a Human/B0 borrower.

Two original-instruction details supersede the initial checkpoint/source notes:
005B1F10 keeps division/multiply/add in x87 through its final20 FLOAT store; no
intermediate FLOAT division store exists.005A8320's empty user-name14 falls back
to full-name0C, not short-name10. Current source corrects both. The PACK writer's
zero-multiplier claim is rejected by the exact stack proof immediately above.
The shared writer retains the original15 multiplier. No unsupported success or
absent-function credit is introduced by these fidelity corrections.

Focused `human-damage-pack-native.log` passes ASan/UBSan/LSan over production
packet serializer/receiver→actual received-damage body, independent1/10/16 wire
checks, all16 drop codes, native quantizer values, actual Human destruction at
the final callback, thrown callback followed by a later packet, and100 repeated
entries. Human/State/Scene/engine endpoints are explicit doubles; this is not a
full FIFO/application run. Actual World/B0/module i386 compile passes in
`human-damage-pack-host-i386.log`; launcher compile/link passes in
`human-damage-pack-application-i386.log`. No output was executed. Receipt:
[shared packet effects](checkpoints/p2.2-human-packet-effects-20261111.json).

Broader imported codecs receive no NEW ledger entries in this integration
checkpoint; their existing PACK branch claims await explicit reconciliation.
Existing damage entries now point to the shared codec. Full selected checkbox,
application integration and user acceptance remain open. Next reaction-specific
gaps: actual mounted movement SOUND producer/release, Player550 objective/
kind12 metadata and nonoffline peer/console owners, then containing frame and
retirement checks. Other PACK clothing/control/item/special-animation effects
remain required and belong to the following Human action/item boundaries.


### Mounted movement SOUND source checkpoint — 2026-11-11

004C6E50 is now bound into the SAME MountedWeaponControlLoading used by both
StaticWeapon and actual indexed Car gun-seat controls. Native120/124 are owned
kind4 SOUND frames in that lifetime, not DWORDs in StaticWeaponControl. Resolve
`soundm` then actual Startup SOUND catalogue group3. A missing nonnegative first
entry returns immediately, skipping `sounds`; a missing/empty second entry has no
sound. Only the first real variant is selected, with no random draw.004370D0
copies that variant's name. Existing0044A1C0 load-into cache supplies the resource;
its inlined body is delegated and receives no new credit.

On successful load preserve native7C(1), cone3FC90FDB/pi2, movement-only90(1),
variant14 volume, variant08 FLOAT bits/0C ranges and two.5 parameters, actual
horizontal13C parent, direct zero local position/flags and final30(false). Failed
resource load releases that slot then continues. C++ engine failures retain the
acquired prefix for containing retirement; missing frame/null variant source
undefined-dereference paths reject explicitly.

004C6DD0(NULL) clears occupant/view, then, only under14C&400, disables120,
enables124 and clears400 AFTER both calls. A failed second call retains400 and
its actual owners.004C7630 releases120 then124 without detach/disable. This
retirement follows normal pose/item/barrel/shell/pivot retirement in004C2380;
partial source guards still reclaim every acquired sound. No positive occupant,
occupied aiming/fire004C5080 or wider running-Car body is invented.

Research index preceded reuse; original evidence is WORK
`static-weapon-control-required/004c6e50.c`, `mounted-sound-loading.asm`,
`mounted-sound-name.asm`, `mounted-sound-retirement.asm` and
`human-reaction-mounted-occupant.asm`. Raw instructions establish first-variant
08/0C/14 reads and native engine-call order; decompiler register aliases around
the inlined cache are not used as independent ABI claims.

`human-mounted-sound-native.log` passes ASan/UBSan/LSan over actual SoundCatalog/
movement-owner classes: metadata/lookup/load order, early missing-first return,
empty first/positive second, resource failure, acquired-prefix exception, loop/
cone/volume/range/parent/zero/enable order, interrupted release flag and100 owner
cycles. Engine/cache endpoints are doubles. Strict actual Car/StaticWeapon/World
composition and launcher checks are `human-mounted-sound-host-i386.log` and
`human-mounted-sound-application-i386.log`; no Windows output was executed.
[Receipt](checkpoints/p2.2-human-mounted-sounds-20261111.json) adds three frozen
client partials and extends two, with no complete promotion. Totals34/2573 partial.
Continue Player550 objective/kind12 metadata and actual peer/console dependencies,
then the full containing frame/retirement boundary. Selected checkbox remains OPEN.

### Player death objective and application scalar FIFO — 2026-11-11

0053BD00's positive550 tail now reaches the SAME script runtime/objective owner
through MissionScriptWorldLink. Ascending selection finds the first kind12
objective28==1, but compares the FIRST objective's18 side to0069E070's selected
team. Preserve that original first-row quirk. Team selection uses actual game71,
selected78, existing Scene actor and process record64/default2, not00982104 or
the offline specialization mapping. Values are copied before the final callback;
no mission borrower survives it.0053BD00 is absent frozen and gets no credit.

006B8700's one-row producer initializes kind12/status4 and maps selected-map
row4→18,1→1C,0→20,2→24,3→28,5→2C. These are explicit optional network fields,
separate from offline byte18/text24/text28. **Full selected-map parsing, normal
application inline descriptor publication and non7 admission remain required.**
This helper is only a credited producer slice, not a substitute for those bodies.

0069BA60 non7 style2/status1 calls0069B900 BEFORE status application; unlike
offline it does not suppress repeated awards with flag15.00730650 adds to SAME
process team44C only under authority bit1, with signed reward rounded to FLOAT
before addition. Then0069B900 takes418 and traverses named, actual3C Humans with
health>0, B064 false and005A8820 side matching.0072EDD0 takes418 recursively,
adds wrapping4C/60 and conditionally publishes actual B0 rank commands. An
interrupted rank allocation retains the earlier team/player score mutations.
All eight vtables' B064 targets005A4240/005A4400/00524D40 read byte70; the former
non7 kind1-only presentation rejection is removed. All three leaves are absent
frozen and earn no new credit.

0069BC40 keeps status store/flag15 and3/4 early exits, but its completion/result/
clock scan is **offline only**. Non7 bit2 selects the SAME HUD objective icons.
Success uses18; other statuses use00724EA0, a register-argument signed
`(side+1)%2` with wrapping addition, not a local-team getter.0049F0C0 gates on
any nonzero actual HUD flags, then calls the existing16×3 icon owner.

After score/status/HUD,0069BA60 appends application kind10 with scalar ID/status.
0072A0B0's process FIFO now owns reached kinds10/17; no Human/mission/caller-buffer
borrower is retained. Non7 append takes the DISTINCT application2C Win32 lock,
not control418/430 or peerB4. Offline false **also only appends**: the drain is
the separate ELSE in the original, not a fallthrough. Explicit drain flushes
1000 before EACH envelope0072BDE0→0072BE00, writes20/32-bit command into SAME
priority0 broadcast stream, then10's5/4-bit values or17's32/32-bit values and
0072BF90 before retiring the node. The actual application fixed-frame finalizer
now invokes that process owner. Other owning command families and targeted
scratch reject; nonoffline flush remains required and throws BEFORE envelope
production, retaining the scalar node. A later serializer/finalizer exception
also retains it; no native partial-wire rollback/retry guarantee is asserted.

Research index preceded static reads and reuse of existing006B8700/0072A0B0 C
from the research checkout. MAIN WORK `human-objective-reused-originals` copies
and hashes that evidence without modifying the research worktree. Original
instructions/vtables are in `human-objective-score-leaves.asm`,
`human-objective-containing-leaves.asm`, `human-objective-state-vtable64.json`.
No new Ghidra export or original execution was needed for this checkpoint.

`human-death-objective-native.log` and `human-objective-session-native.log` pass
focused ASan/UBSan/LSan: actual descriptors, Player selection quirk, callback
destruction, shared score/status/FIFO/bitstream source, recursive418, rank/HUD/
queue/flush/finalization failures, offline completion regression and100 process
lifetimes. Human/HUD/transport/OS locks are explicit doubles. Strict actual
World and launcher compile/link pass in `human-objective-session-host-i386.log`
and `human-objective-session-application-i386.log`; Windows output was not run.
[Receipt](checkpoints/p2.2-human-death-objective-session-20261111.json) adds nine
client partials, extends three; totals34 complete/2582 partial, no promotions.

Next: reached0072F2F0 scalar publication and actual006C4B40/0069F1E0/00744870
console/application lifetime, then containing reaction/frame/failure/re-entry/
retirement. Ordinary non7 metadata/peer3C/transport remain required. The full
selected checkbox, P22-HUM, application integration and desktop acceptance remain OPEN.

### Reached peer logging and console source lifetime — 2026-11-11

0072F2F0 now uses actual process record00, preserving its genuine absent-transport
return. Authority publishes scalar application17(reason,peer) BEFORE localization
or logging. Reasons0/1/2/3 use22AB/22AA/22AD/22AC; default retains the native
`%s kicked because unknow(new?) reason` spelling. The peer name is read after
publication. This producer does **not** remove/close the peer or its Human;
actual received17 and remote transport remain required. Failure in text/logging
retains the already queued scalar record. Native undefined bounds/varargs reject.

006C4B40 first reads actual config170; true selects console directly with only
the first name formatter. Otherwise the SAME application controller's actual
current state is queried. Absent/non12 skips210, while kind12 explicitly requires
its unreconstructed210 producer. A genuinely absent published game returns;
an existing game invokes0069BF70 with white text, choosing actual HUD or console
by mode2. Preserve this second zero-argument formatter ONLY on the game route;
dedicated console retains first-stage percent bytes. Damage still uses its two
format stages; rank uses its native fixed string assembly. All three nonrender
message paths now reach the same actual console owner, with no terminal substitute.

0069F1E0 lazily publishes a process GameConsole BEFORE actual Win32 creation,
matching reentrant CreateWindow callbacks.00744340 supplies distinct48 lock,
owned output/submitted input and initial `>` at0083686C.00744870 copies caller
text before48, prepends newest output, caps100 by releasing oldest, unlocks then
invalidates the HWND.00744A00 first enqueues submitted input, then copies output,
then resets prompt;007447B0 transfers oldest owned input or genuinely returns
empty.00744800 uses the low byte, Enter/backspace, native50-byte length gate
(including prompt), and input redraw. Empty backspace's native underflow rejects.

Actual00744C60 Win32 class/window/font calls,00744640 window dispatch and
00744B80 GDI paint are reconstructed but absent frozen, so receive **zero** new
credit. Class WndConsole/styleB, title `Hidden & Dangerous II Console`, window
10EE000D/480×360,10-point bold Courier, native scroll-height/thumb handling,
paint's bottom*2-offset-17 and12-pixel upward lines are preserved. WM_CLOSE sets
actual current application status4; it does not destroy/post quit.00744A70 uses
original511-byte space padding/backgroundC0C0C0 and releases its DC. Its native
auxiliary callback vector64 is constructor-empty; no positive producer is exposed.
The window boundary catches callback exceptions and the actual control phase
rethrows them, rather than losing a required failure across CALLBACK ABI.

Source RAII intentionally reclaims acquired HWND/font before retiring callback
dependencies and frees pending typed input strings; native00744490 does not
destroy HWND/font and erases pending input nodes without freeing their buffers.
This ownership adaptation is explicit, not original allocator/SEH equivalence.
The console owner outlives World exit. Actual application controller borrowers
unbind before retirement. Console input parser/containing non7 frame consumers
and kind12/210 remain required; exposing its FIFO is not command execution.

Research index preceded focused rollback/read-only ABI exports in MAIN WORK
`human-console-owner-abi`/`human-console-window-abi`; both export logs complete
without ERROR. Correct0069F1E0 is no-argument/pointer-returning, not the old
decompiler's thiscall(void*,text) prototype.008165E8's base console table has two
slots;00744FA0/00744FC0 belong to the NEXT transport class, not console teardown.
Their exploratory export receives no source/function credit. Raw window dispatch
is retained in `human-console-window-procedure.asm`.

Focused `human-peer-console-native.log` passes ASan/UBSan/LSan over actual typed
console text/FIFO and peer producer: copy lifetime, cap100/order, FIFO/prompt/byte
limit, unlock-before-invalidation, failure prefixes, exact reason IDs and route
format differences, plus100 lifetimes. Win32/GDI/transport/OS locks are explicit
boundaries, not desktop evidence. The changed damage formatter's existing focused
audit also passes. Strict actual World i386 passes in
`human-peer-console-host-i386.log`; launcher AND startup probe compile/link pass
sequentially in `human-peer-console-application-i386.log` with actual GDI import
library. No Windows output or original binary was executed.
[Receipt](checkpoints/p2.2-human-peer-console-20261111.json) adds ten client partials,
extends four; totals34 complete/2592 partial. No complete promotions.

Continue reaction-specific authority packet admission/publication, actual non7
metadata/peer3C/transport dependencies and the containing Human frame/failure/
re-entry/retirement boundary. Full selected checkbox remains OPEN; no application
mission integration or user desktop acceptance is claimed.

### Authority damage and selected-reaction retirement — 2026-11-11

The World receiver no longer rejects EVERY non7 authority packet before decoding.
It now reaches the actual shared grammar and existing per-command receivers:
00540DB0 command17 invokes005A1C90 with decoded kind/amount/hit/source and ZERO
point/impulse;10 invokes actual005A3490 drop.6 performs its actual Ladder action,
ignores that result, then queues to SAME SECOND90 only in non7, after rechecking
actual Scene2 Human/B0 lifetime. Clothing/items/typed control/action/feedback
positive gaps still throw at their own receivers. Their original conditional
peerA8 timestamp refresh belongs after those effects, not a blanket admission
success or timestamp for17/10/6. Nonreserved Remote C4=00583220 is genuine RET4
without reading a tag; reserved Remote still selects authority DISCARD. The empty
leaf is absent frozen and earns no credit.

Original input0 stores both64-word C4/1C4 buffers BEFORE00547460 insertion. The
imported queue-before-copy order was incorrect on allocation failure; source now
retains the already stored inputs when insertion fails. This affects only actual
native buffer/queue ordering, not the no-payload005AC760 command0 decoder.

0053F370/00577B30 lazily open TWO groups: ordinary commands use wrapper boolean1
→priority0; motion4 uses boolean0→priority3, clamped2 in0072BEF0. Both selected
streams append a module13/actor10-bit prefix when first opened. Motion terminates/
finishes BEFORE ordinary. Under authority FIRST, both wrappers select SAME2BC;
the original nested prefix/order and both terminators are preserved, not replaced
by an invented split. No claim is made that such a contrived alias sequence is a
valid live protocol. Source transaction rollback restores every touched cursor/
active field on failure, including a shared alias; it does not undo actual late
snapshot effects. Ordinary reaction1/10/16 payload bytes remain unchanged.

0074D510's existing selected-reaction tail is now one independently callable
source body in `human_reaction_retirement.h`, invoked by actual DD0 with refs to
its SAME Human/physical/face/action/tracks/movement/pool/World/frames/random/global
owners. No new owner or production fallback is introduced. Private B0 publication
is passed as its original containing callback. Pool dereference stays AFTER
equipment event2/face work at actual motion-audio retirement, not helper argument
evaluation. Seat removal precedes Human attachment clearing; step reset precedes
voice; Look masks precede equipmentCC; action/fades precede transfer2/audio;
Ladder removal precedes descriptors; update precedes transfer10/mount retirement;
only velocity X becomes-10000 AFTER consuming all XYZ. Clip lookup/model placement
and the >=7 air-pose/reason2 override retain original ordering before007B0910.

Native `human-authority-damage-native.log` passes ASan/UBSan/LSan over actual
grammar→policy/damage→real Player B0 queue→real serializer→received damage body.
Actual input registry/callback types are reused. The audit deliberately calls
Base::bind for the queue owner; this is not complete Player activation. Access
control is disabled only to inspect private C4/1C4 after forced allocation failure;
no production inspection API is added. Exact motion groups/request alias/two-stream
rollback, actual Remote empty/discard distinction, missing state, required hit
failure, B0/input-before-Human retirement with later-command discard, and100
entries pass. Other World/engine endpoints are explicit doubles.

`human-reaction-retirement-native.log` passes actual Look hit classification→
selection→SAME selected-retirement source, with exact audio/equipment/clip order,
failed seat prefix and re-entry, failed transfer BEFORE pool lookup, mounted random
draws/seat conflict, motion deceleration/X-only sentinel, air override and100
lifetimes. Component/engine/geometry endpoints are doubles; unchanged joint/clip/
contact bodies were not re-audited. Strict actual World authority/presentation and
DD0 closure compile passes in `human-reaction-containing-host-i386.log`; launcher
AND startup probe compile/link pass sequentially in
`human-reaction-containing-application-i386.log`. Windows outputs were not run.

[Receipt](checkpoints/p2.2-human-reaction-authority-retirement-20261111.json) adds
one client partial0074D510 and extends four existing entries; totals34 complete/
2593 partial, no complete promotions. Earlier source-only joint/clip entries retain
their uncredited status. Full selected checkbox remains OPEN pending the actual
normal metadata/peer3C/transport and containing mission/World/frame boundary.

Next metadata evidence: actual GameData inline MP mission is **kind13**, proved by
006B5620 at006B56AF; its source still requires application ownership/admission.
006B8700 resource copy goes to GameData38 (mission-relative24), not GameData24;
GameData80 is selected name. Inline vtable00815AA8 has24=006B83B0 finalize,
28=006B5890 clear and3C=006B5960 objective append. Read their existing/indexed
evidence before recovering that containing producer.00612A60 ECX is separate peer
slot, EDX Scene identity; its late3C/A8 write and remote-AI auxiliary publication
must preserve418 and actual teardown. Never conflate those identities.


### Application-owned network mission metadata/request checkpoint — 2026-11-11

The reached Player death-objective path now has its normal selected-map producer,
not just a one-row fixture.006AD580/590/5B0 resolve/write/destructively replace the
SAME application24 rotations.006AD820/990 preserve strings, five unsigned bytes,
last signed16 scalar, objective count8/74-bit rows and table count8/32-bit rows.
006ADB70/BD0 use row1/0/2/3/4/5 with16/16/16/8/2/16 bits; wide reads sign-extend.
006ADC30/70 use16/8/8 bits. Reader phase order is incremental: a missing second
string leaves the first replaced; each count is read before clearing its old
vector. Replacement retires old maps in ascending order, reads a new local record
before appending, then writes index0. A failure leaves an empty list and the old
index, with RAII reclaiming the failed prefix. Count>255 writers reject at the
reached count phase instead of native wrapped count plus all trailing rows.
Actual SessionPacketStream bounds errors are safety deviations, not atomic native
rollback. Native4096-byte string limit/termination behavior remains in that codec.

006AD6E0/780 preserve416 rule bits: domain174/style224 each8,22 signed16 rule
words, process009820D8/D4/DC each16, with DC seconds conversion. Writes divide
signed milliseconds by1000 toward zero; reads multiply signed seconds by1000.
SAME Startup control owner retains these three process values across Worlds.
Original PE section bounds prove they lie in loader-zero-filled .data; the focused
Ghidra reference export records only packet/advertisement/console-variable users.
This is actual process data, not a transport or runtime input-history substitute.

ApplicationStartupData explicitly constructs ONE MP mission owner before config
loading, adapting original inline GameData+14 to a stable unique owner. The
preparation-only reloaded GameData graph allocates no extra MP mission.006B5620
sets kind13, initializes50/54/58/5C toFFFFFFFF and leaves60/64 undefined until
received. This adds a typed allocation in place of original inline ABI.006B8700
clears the same descriptor through006B5890 (reverse objectives, table clear,
reverse children), sets caption2002, retains selected name into GameData80,
copies resource into GameData38 (mission24), and appends actual kind12 objectives
through006B5960. It maps all six row words, copies three table words, then stores
GameData68-before64,6C-before70,74, retained7C and78. A late undefined scalar
throws at that phase, retaining earlier mutations. Native006BA0F0 copies a fourth
unwritten stack DWORD into each16-byte row; source represents that word as
undefined, with no invented zero or consumer.006B83B0 sets cursorFFFFFFFF then
006B83C0 sets every actual objective status4.

Retirement preserves roots before the MP descriptor; its reverse objective
retirement/table clear/children retirement precedes table allocation release,
name6C and metadata68 release, then base mission storage/strings. Roots vector
allocation retires last.006B7F50 is ONLY an end-cursor clear, **not allocation
release**; raw006B5330 confirms related allocation retires in the base mission.
Saved inline94/full original GameData ABI remain outside this subset and receive
no destructor credit.006B5890/006B5960/006B83B0 are absent from the frozen
inventory and receive no new credit. Typed graph copying now preserves network objective and
mission fields with retained COW strings.

MissionRequest now distinguishes its existing owned offline copy from a typed
borrow of SAME application MP descriptor. Non7 actual process flags plus nonzero
config04 admit a complete sourced mission13 with empty campaign/live squad;
offline kind3 still requires prepared AND live squads. No campaign preparation
is fabricated. Borrowed process/configuration/route references are re-read during
validation; factory requires actual lvalue owners. Existing route owns the adapted
standalone mode; native006B9190 tests that mode first, so this network factory
rejects it before looking at MP storage. Full saved descriptor handling stays
required. Application must outlive request/session/World borrowers; no map or
packet borrower is retained after metadata copy. SessionSelection now chooses the
actual application GameData for a network request while preserving the offline
preparation bridge.006B8EC0 traverses actual container cursors and filtered kind2/
kind3 ordinal order; invalid campaign selection returnsFFFFFFFF, not zero.
Its root bounds precede standalone E4. Network activation still rejects at the
existing required non7 consumer; no UI connection, status9 or loading success is
exposed by these source admission methods.

Research index preceded reuse and new exports. MAIN WORK
`human-network-map-reused-originals/provenance.json` pins copied research C/JSON;
`human-network-mission-abi` and `human-network-selection-abi` are focused read-only
rollback exports, no project edits. Raw `human-network-*` disassembly retains
inline field, clear/finalize,16-byte copy, ordinal and retirement evidence.
`human-network-mission-native.log` passes ASan/UBSan/LSan: independent exact wire,
signed rows/rules, actual config→stable mission13/objectives→Player selection,
map/packet retirement, COW copies, destructive failures, actual loaded mixed-node
selection, offline-empty rejection, same borrowed session descriptor, allocation
failure prefixes and100 session constructions/retirements. Engine/backend/OS
endpoints are explicit doubles. Strict actual World i386 and launcher/probe
compile/link are recorded in the linked receipt; Windows output is never run.

[Receipt](checkpoints/p2.2-human-network-mission-metadata-20261111.json) adds14
client partials and extends006B8700/006B9190, no complete promotions; totals34
complete/2607 partial of18783. This closes the reached metadata/source request
batch, not P2.8 delivery or the full Human checkbox. Next is actual non7 peer3C
creation/configuration/storage and matching borrower teardown, then containing
Human frame/reaction failure, re-entry and retirement. Live transport and actual
application/desktop acceptance remain open.


### Cold peer/Human creation and reached retirement checkpoint — 2026-11-11

00612A60 now has one shared model/Scene2 factory body for existing offline callers
and the explicit session factory. Preserve distinct **ECX peer slot / EDX Scene
identity**, model `_PLAYER_%2d` from Scene identity, current-mode clothing selection
(soldier54 outside7, explicit clothing in7), and factory frame release BEFORE
0059EDC0. The actual host passes optional12-word stats and SAME application,
WorldActors/Items/profile/speech/effects owners; it creates no second Human graph.
The old offline host interface rejects a missing network peer argument.

0059EDC0's peer hook follows kind/personality/HumanStore170/profile lookup and
precedes speech/stat/storage/eye work. Actual B060 stores the byte into SAME B070;
all eight vtable60 entries prove identical byte setters.00730140 bounds BEFORE
418 and stores actual side64 or pending68; flag09 writes actual session78.
0059EB60 retains fixed-slot order0/4/5/2/3 then holster1/pack6, one instance per
record. Non7 creation writes missing WORD identity back AFTER the actual item
returns and BEFORE clothing. Reread the live record/mode at that phase; preserve
an authored/mutated identity and low16-bit wrapping. Existing cold Items/clothing
receivers still own real instances/model transfer; occupied transfers remain
required. Stack counts are split by the containing caller, not expanded here.

Only after complete setup/typed containing-owner construction does00612A60 enter
418, publish SAME Human into peer3C, read actual **timeGetTime** and store A8,
then conditionally append the late record borrower through auxiliary virtual8
(0045FDE0 thunk→00461AA0) for authority/remote-AI. Unlock last. A setup failure
leaves registered Scene ownership/earlier metadata without3C. A later clock/
auxiliary failure preserves the published prefix; no native rollback is invented.
00730460 uses actual local slot with unsigned bounds and returns a borrower,
including genuine null for dedicatedFFFFFFFF. No reverse peer/Scene map exists.

0069E720 now has actual source/World bindings: reverse-find transport00 under418;
clear transport/name without erasing/shifting the peer; if3C exists set A2,
clear matching game78, perform the native presentation refresh/override/spectator
phases, conditionally remove auxiliary roster (0045FDF0→00461C70, no selection
release), invoke required00479700 runtime links, then schedule SAME Human through
00475C50(false)→00476680. Clear peer3C and nine40..60 words only AFTER that returns.
All peer accesses reread the slot after callbacks; record-vector growth while the
recursive lock is held never retains an invalid STL element pointer.

**HUD refresh is not pointer erasure.**0049F0E0 reads primary soldier Human74 at
HUD10394 behind any nonzero HUD flags, not selection6B8F8/FC.0049F100(NULL) queries
the actual selected player twice; if present bind/show that SAME primary soldier
then reposition actual explosive timers. If absent, hide and RETAIN Human74,
flags and old geometry. Supplied-Human branch binds/shows without timer movement.
Read the root AFTER binding.004AF900 moves/updates the actual root, then rebinds
all ten TextState geometries in forward order and stores inset0 AFTER each bind.
Native position wrapper/inlined engine cache equivalence is delegated/uncredited.
0045E8F0 stores control override58 before camera40(0) and actual FPV rebind.

Reached HUD name/rank now supports non7:005A8380 copies all nine40..60 words under
418 and returns copied54 AFTER unlock.004B3890 caches it before later lookups;
style0 selects27A6/27B0 rank family using real holder4/definition44, otherwise
uses actual005A8820 side/configuration policy.005A82C0 kind1 gets real peer name
by **Scene identity**, while other kinds keep profile user14/short10 or Actor
name. Publish SAME TextObject record0/style3A after rank/name retention. No new
profile, peer or HUD state substitutes these owners.

005A7FE0 is now a shared body invoked AFTER actual Scene2 delayed ownership is
published: optional actor160/164 virtual80, remove observer258/target25C/listener
2A4/proximity188 and invalidate each field AFTER its receiver; disable SAME DD0
collisions/visibility; then late B0 FC with current low mode bit; set20C=false
last. This does NOT delete Human or remove its shared input. Missing DD0/posture
and occupied attachment80 remain required failures. Actual delayed expiry still
performs B0/input teardown before Human destruction. Native client-mode lifetime
exposed an external-composition guard defect: client2 has no later authority call.
World now retires its SAME composition after the last actual mode pass; destructor
and failed-prefix World cleanup follow current bit2 then bit1, without inventing
opposite-mode callbacks. Keep the map key published through owner.reset before
erase, resolving the SAME Human loading_lifetime borrower during destructor
callbacks. Reject re-entered owner destruction/new activation. This is typed
ownership adaptation, not another owner graph or original ABI credit.

Actual Remote state7 virtualFC is **00592220**, which forwards ONLY7/8/10 and
clears the whole owning queue. A nonallocating borrowed range now shares that
filter with actual B0 publication, retaining all command objects until successful
serialization; no prepare/snapshot call occurs for discarded kinds. AI/Player
ordinary/motion ordering and existing rollback remain intact.00592220 and the
B060 setters are absent frozen and receive ZERO credit. No new solver/clip audit
or credit was made.00479700 has a real forward ownership-scan boundary, preserving
native increment after swap/removal, but its positive kind2 Human-reference9C
owner remains unrecovered and throws; it receives ZERO new credit.

Important identity/lifetime limits remain: native delayed A2 clear and Human
destructor3C clear use **Scene id10**, while0069E720 clears its actual peer slot.
The focused two-distinct-identities checks retain this quirk; do not invent a
reverse map or corrective cleanup. Full0072E890 entry/input-manager reset and
whole-session failure/shutdown must supply the original containing protocol.
Normal00611C90/006123A0/006125A0 non7 rule/spawn/packet callers still reject before
their unrecovered tails. Application20 control decoder, received2/10/17 routing,
non7 transport and full containing Human frame remain required. These concrete
source factory/retirement methods expose no new UI/connection/loading success.

Research index preceded reuse/exports. WORK `human-peer-lifecycle-abi` is a
read-only rollback export of00592220/005A7FE0/0069E720/00612A60/0059EDC0/0059EB60;
correctfastcall612A60 now retains both saved register parameters. Other peer C
is pinned in `human-peer-lifecycle-reused-originals/provenance.json`. Static
vtable60/FC tables and raw HUD/override/auxiliary/timer leaves retain the evidence.

Focused native ASan/UBSan/LSan PASS: `human-peer-preparation-native` (actual
catalogue/model-factory policy/Scene2 registration/setup/storage/writeback/late
publication,100 lifetimes); `human-peer-lifetime-native` (actual peer policy,
Player Base queue/input boundary only, actual control override/HUD refresh/aux
roster and Scene2 delayed retirement, recursive rank418, vector relocation,
destructive failures and100 normal lifetimes); `human-scene-retirement-native`
(actual Remote bind/filtered queue/codec/delayed owner, failure/re-entry and100
lifetimes); `human-peer-hud-timers-native` (actual timer owner/TextState geometry,
inset-after-bind/failure/re-entry and100 lifetimes); unchanged authority packet
regression passes with the new range interface. Component/model/Items/profile/
audio/HUD/frame/platform/transport boundaries are explicit doubles, not a full
World run. `human-peer-owner-retirement-native` executes the SAME owner-map/mode
guards with actual input/Scene2 ownership for1/2/3/7 x100, client failure,
reset-before-erase callback lookup/re-entry rejection and fresh mode reads;
composition/deactivation endpoints are doubles. Strict actual World/factory/HUD/
retirement i386 and launcher/probe link evidence is pinned in the receipt.
No original/DLL/Windows output or desktop run occurred.

[Receipt](checkpoints/p2.2-human-peer-lifecycle-20261111.json) adds seven client
partials and extends seven, no complete promotions:34 complete/2614 partial of
18783 (2609 client+5 engine partials). Full selected Human checkbox, P22-HUM,
application integration and desktop acceptance remain OPEN. Continue the actual
spawn/application-packet/entry protocol and containing reaction/frame boundary;
source lifecycle helpers are a verified dependency checkpoint, not delivery.


### Application presentation messages in the containing process FIFO — 2026-11-11

0072C0B0 reads envelope20 then32-bit application command BEFORE the current
loaded-World gate; the actual process StartupSession now dispatches it and resumes
this packet afterward.0072C3B0 can drain retained queues in non7 mode independently
of transport production. Authority phase still gates bit1 first, then requires the
actual007239F0 service pump if configuration174==1 BEFORE queue pop. That service,
input-history decoder, non7 flush/send and authority00727800 remain explicit
required failures. Their grammar is not the presentation00728450 grammar.

Reached presentation00728450 branches now bind SAME source owners:

-2 reads transport32 even if authority/no game. For client+game, forward active08/
transport00 lookup0072EC90 uses actual418, then0072EBD0 bounds/name after unlock.
A nonnull name (including empty) causes22CB localization, a SECOND name lookup,
`%s %s` formatting and actual application log. Only afterward requery current game
and reach actual0069E720 retirement. No record pointer crosses callbacks.
-10 reads objective5/status4 unconditionally; client+game directly invokes actual
0069BC40 on SAME Script objective owner, with no0069BA60 score/experience/FIFO echo.
-17 reads reason32/peer32, checks game BEFORE comparing actual local slot. Matching
peer invokes00699A20: result88=4 and8C=game28 precede optional hint22A7/22A6/22A9/
22A8 for reasons0..3; unknown reason still mutates result/time. Query mode AFTER
this receiver, then client invokes SAME0072F2F0 scalar log.17 does NOT remove the
peer/Human. Received2 is the separate departure/retirement route. Earlier wording
that received17 owned retirement was corrected in source.

006C4B40 is now one process method shared with Human World services; existing
actual application state/configuration console route precedes absent-game gate,
then0069BF70 formatting and SAME World HUD message/console receiver. Application
World bindings remain mandatory even when loaded14 is false; a current game
without its required source graph fails explicitly. No new UI/loading/network
success was exposed. Current packet ownership retires on callback failure while
later queued packets remain available, preserving the actual process lifetime.

Research index preceded original reuse. `human-application-reused-originals`
pins research checkout C/metadata for00727800/0072EC90/0072E890 read-only. MAIN
00728450 C and0072C0B0 C remain original evidence; raw
`human-application-local-removal.asm` proves00699A20 scalar ordering, unsigned
reason bound, hint leaves and jump table. No new Ghidra export was needed.
**Entry correction:** actual0072E890 invokes0073F5A0 to CLEAR registered input
records, preserving the manager and embedded30; it does not replace that owner.
The caller006C20F0 retires its previous application state before this branch and
constructs the new game state afterward. Full containing game/global lifetime
ordering still needs matching source integration; do not infer a second manager
or move input clear relative to Human shutdown from older handoff wording.

Focused `human-application-packets-native` ASan/UBSan/LSan PASS: independent wire,
application messages before loaded gate, SAME objective/session/peer state,
no outbound objective echo, late mode/name (record vector growth), local hint/log
failure prefixes, retained later packets, required authority/domain failures,
truncated fields and100 actual scalar FIFO→broadcast→owned process queue lifetimes.
Platform/log/HUD/Scene endpoints are doubles; no full World/application run.
Strict actual World i386 and sequential launcher/startup-probe compile/link PASS;
no original/DLL/Windows output executed. [Receipt](checkpoints/p2.2-human-application-packets-20261111.json)
adds3 client partials and extends5:34 complete/2617 partial of18783 (2612 client+
5 engine partials), no complete promotions. Temporary audit remains ignored for
the ongoing full checkbox. Continue normal non7 spawn/entry/canonical input
ownership, required runtime owners and containing reaction/frame/teardown;
application loading/rendering and user desktop acceptance remain OPEN.


### Canonical process input ownership and entry record reset — 2026-11-12

Source World now borrows ONE process `SessionHumanInputs` handle; the actual
0073EC50 manager allocation and inline20/120/A0 profiles belong to StartupSession.
No second World manager or process-copy record registry exists. Native00723000
first invokes00723220 (delete old manager, then null actual14), constructs and
publishes the replacement BEFORE session setup, regenerates all three profiles,
then binds defaults/getters/embedded local identity/shared relative flag. A failed
published setup remains incomplete and explicit; no atomic rollback is invented.
The stable source handle resolves the CURRENT actual allocation for B0 removal/
contains checks, so Remote teardown/destructors never call a destroyed manager.
Old Player close still REQUIRES its registered record: after replacement it
throws at actual required-input boundary, after controlled-Car exit. No fresh
record is fabricated to hide that original/reached failure.

The stateless Human callback bundle is process-lived, matching the original
callback labels. Default getter closures retain only StartupSession and resolve
actual current Scene2 each time; Car input still requires its original receivers.
World retirement detaches B0's own records/borrowers without destroying all process
records, manager, profiles or embedded30. The source current-game packet/default
getter link now stays published until Human/B0 owner retirement finishes, before
World unbind. This is typed owner-lifetime adaptation, not another owner graph
or additional original ABI credit.

Status9's0072E890 is a DIFFERENT protocol phase: under418 clear peer3C and nine
40..60 words; unlock; clear registered process14 records only if manager exists;
then clear each three-team nine-word block and score. It preserves the manager,
embedded30, defaults, peer-vector slots and unrelated fields. Source method is
bound to SAME actual process state. Full00730A40→0072E890→00731610 caller is still
required. Received-start0 instead invokes00724110/00723000 before old-game
retirement, as the previous section and original initial-sync contract describe;
its optional service/current-state/map/load/sync caller remains required.

Cold source World construction currently acquires/configures the REAL process
manager lazily only if absent; this is an explicit source graph adapter, NOT
credit for full native control-start timing. Ordinary menu startup still has no
manager. Cold/offline control reset accepts ONLY this actual owned process14,
flushes first, publishes mode7 before record clear, then peers/queues. Manager/
embedded30 survive reset. Final0072E460 additionally deletes actual manager
through00723220 BEFORE critical-section deletion; fallback retires acquired
storage while preserving any required reset exception. Live transport branches
remain unavailable; no successful non7 shutdown or application entry was exposed.

Research index preceded original reuse. `human-process-input-originals` pins
00723000/00723220/0073F5A0/0073ED00/0072E890/0072E460/0072E4E0 and00724110 C/
metadata; research checkout copied read-only where needed. No new Ghidra export.
Focused `human-process-input-native` ASan/UBSan/LSan PASS: same manager/default/
embedded ownership, actual Remote full bind/close across destructive replacement,
Player Base plus required retired-input close failure, profile regeneration,
failed publication/getter prefixes, entry peer/input/team ordering,100 lifetimes,
actual cold control reset retention and final input deletion before locks.
Frames/clock/Scene getter/presentation are explicit doubles, not full World entry.
Strict actual World i386 and sequential application-launch/startup-probe compile/
link PASS. No original/DLL/Windows output was executed; no desktop claim.

[Receipt](checkpoints/p2.2-human-process-input-20261112.json) adds3 client partials
and extends3:34 complete/2620 partial of18783 (2615 client+5 engine partials), no
complete promotions. Full selected Human checkbox and application/desktop gates
remain OPEN. Continue reaction-specific non7 Car ownership/direct impulse, normal
spawn/entry callers and the full containing frame/failure/teardown boundary.


### Reached Car authority, Human contact binding and spawn protection — 2026-11-12

004DE800 now has the actual source AL policy, bound to SAME Car seat0 and actual
process local peer3C (not cached driver24AC). Offline7 returns true without peer/
seat reads. Otherwise virtual78(0) precedes00730460; equality includes BOTH NULL.
On mismatch retain repeated seat calls, actual occupant kind19C and late mode:
authority admits non-kind1 occupant or empty seat; other unmatched ownership is
false. A seat disappearing at native required kind dereference remains an explicit
source failure. Do not collapse this into authority bit alone or cache all seats.

007CBF90/007CC740 call004F2790 DIRECTLY after that check in both modes. The old
blanket non7 guard in `human_ragdoll_car_impulse` was unsupported by those original
calls and is removed. SAME Car body velocity/direction/angular impulse/inertia/
mass-change notification now receive admitted ragdoll impulses. Existing actual
cold6A4=NULL contract remains; no positive callback producer was fabricated.
Other Car hit58 non7 paths still require actual004D3D10 event publication and
retain their failure. This checkpoint makes no new solver/clip/kernel credit.

Actual Car callback Human5C now dispatches through the SAME published Human
loading_lifetime to its existing005A07C0 source body, then actual B0 C8. No second
Human/World or copied health receiver exists. Absent B0 returns false BEFORE
source/lifetime requirement. Moving-Car authority damage preserves reverse offline
occupant alignment, point/forward direction, kind13/amount-1/driver, then re-reads
B0 after actual damage. Source contact preserves initial mode snapshot for its
native initial/offline tests; C8 now receives CURRENT mode after damage, matching
its own native global-mode read. Raw x87 `TEST AH,41; JNP` also confirms unordered
speed enters damage: use `!(speed<=2)` rather than ordered greater-than. Required
missing post-damage B0/damage failures remain visible. Unused vectors of the
three verified C8 families stay with their callers.005A07C0/0059C2D0/C8 leaf bodies
are absent frozen inventory and receive ZERO new credit.

0059F2C0's local presentation prefix now reads actual configuration250/rule10
for spawn protection, DISTINCT from respawn interval23C/rule5. After base Actor
activation and before local initialization, non7 presentation bit2/local Human/
positive signed seconds publish4F59 and wrapped seconds*1000 through SAME actual
0049DE50 injury TextState/time/flag owner. No flags gate, new queue or successful
missing HUD receiver. Repeated activation still executes this prefix even if90
already set. Helper/service names now identify protection rather than respawn.

Research index preceded reuse;004DE800 C/raw,005A07C0 C/raw, corrected original
007CBF90/007CC740 and0059F2C0/0049DE50 C retain the contracts. No new Ghidra export.
Focused ASan/UBSan/LSan PASS: all mode0..7/seat/local/kind ownership combinations,
repeated getters/late mode/invalid seat, actual direct impulse100x4; callback
binding→same lifetime→actual contact/Base/Player C8 with damage/failure100 lifetimes;
actual spawn prefix→injury TextState100 panel lifetimes and destructive failures.
Containing owner-map/mode retirement regression modes1/2/3/7x100 also passes after
its source interface update. Policy/frames/damage/lifetime/geometry/fonts/
localization endpoints are explicit doubles. Strict actual World/Car callback/
Human composition i386 and latest sequential launcher/probe compile/link PASS.
No original/DLL/Windows output or desktop test occurred.

[Receipt](checkpoints/p2.2-human-car-contact-20261112.json) adds ONLY004DE800 and
extends0059F2C0/0049DE50:34 complete/2621 partial of18783 (2616 client+5 engine),
no complete promotions. Existing uncredited007CBF90/007CC740/004F2790 source
kernels were reused without new ledger credit in this focused integration batch.
Temporary native audits stay ignored until full selected checkbox completion.
Continue actual non7 spawn/entry/transport and occupied/runtime owners, then the
full containing Human frame/reaction/failure/teardown. Full Human checkbox,
application loading/rendering and user desktop acceptance remain OPEN.


### Presentation spawn, match/HUD and transmit targets — 2026-11-12

006123A0 now runs through SAME World/Scene2/process owners outside offline7.
A client alone calls00612A60; authority non7 looks up Scene identity; offline
reads SpawnPoint A8. A client never stores A8. Missing SpawnPoint frame returns
before creation, missing Human returns before presentation. Local non7 phase1
precedes client activation; mode is reread after callbacks. Camera0/2 precede
roster contains/append. Kind1 writes side through005A8800 using **Human Scene
id10**, not the peer argument; then actual005A0890/select, B060 flag08, Player550
flag09 and0069E290. Kind7 writes Remote2C0 flag09 BEFORE B060 flag08. Final client
voice gate reads CURRENT profile settings byte38, distinct from first-person
preference and camera-inversion byte39. The existing prepared factory is now a
dependency-templated borrowed adapter, shared by activation and presentation;
no model, input, roster or lifecycle owner was duplicated.

006125A0 reads subtype0/peer8/Scene id8/pose/direction/generation8/name/transport32.
Client00475510 stores generation-1 BEFORE soldier decode. Actual Scene registry
setter ignores signed ids outside0..31 and stores the low BYTE. Full soldier
then default stats `[800,80,80,80,80,80,50,30,20,50,0,80]`; non7 reads widths10/7
in index order0/2/1/3/4/5/6/7/8/9/11/10. Fresh bit2 gates0072E940, and fresh low7
gates the final explicit-stats pointer. Truncation preserves the already-written
generation. Authority packet production/00611C90 positioning remain required;
this decoder is not a substitute for those producers.

0069AA50 writes **B8 match phase**, not result88, before HUD. Phases0/1 call
0049F250(1), then B4=-1; phase2 calls HUD2 then Camera2(1,1). Other values only
write B8.0069E290 local peer only: result88==3 suppresses B0 and invokes phase2;
otherwise HUD0 then actual0049F100(NULL). Only afterward90=0,0049F040(false),
then style2/actual005A63F0 optional4F57/5000 injury text. Failures keep prefixes.

0049F250 is bound to real inline HUD components, not a flags-only receiver.
Phase2 flags `(flags&~80)|100` precede inventory hide, speed root, cursor6,
signals false/0/0 (64/68 stores then0049D340(0,5)), roll false, messages0,
hands/timers/status hide, map false, pointer(-1), scoreboard visibility, and
TWO fresh optional objective-root hides.0049F1C0 runs once last. Native owner
mapping:2E50=speed[0],107E4=hands.root,10820=status.frames[0],6B63C=timers.root,
6B628/630=scoreboard root/style,6B564=objectives.root. Phase1 hides status/hands,
shows optional objectives, hides timers; phase0 shows hands/status, scoreboard
according to style!=-1, optional objectives and timers. Other phases only postbind.
0049F040(false) hides actual map then reads late center E0&1 into HUDbyte21.
Map004A5630(false,false) is a real no-op with missing center; otherwise flag24=0,
center/map hide then kind-dependent pointer hide.0049F250 still hides pointer
again even if map center is absent. No optional fake panel was supplied.

Reached false bodies:004A3920 hides inventory/description, clears C4, hides and
updates dragging;004BF4A0 writes64/68 before live controlled-roster selection
clear, then root hide/update;004B27B0 hides root BEFORE clearing30.00496120(0)
writes10/30 first, reaches offline0069B8B0 BEFORE visual hide, then004961E0(false)
clears input and D387/D386/D385. Positive population/input remains excluded.
Offline pause retains actual64 prefix then REQUIRED0071F640/005E8D50/00428F00
failure; normal non7 spawn skips that callee. No new0069B8B0 credit. More than six
controlled soldiers fail at the reached panel instead of native out-of-bounds.

00724570 uses the actual process **MultiplayerServer descriptor4E00.flag88**,
constructed by0072DE00→00724EE0/00724EF0, distinct from configuration.server17C.
Condition order is4E88-or-authority, direct local record3C, wrapper0C. Cold absent
wrapper is a genuine no-op only after the local record access. Query actual
005A8820 side;0069BFE0 is a proven RET. Under418, scan live peer-vector count,
nonzero transport00 and Human3C, query side, then reread current transport after
callback before appending. Unlock BEFORE rereading wrapper0C/object04 and calling
external COM virtual2C(object, count?targets:NULL,count,0); ignore HRESULT. Native
stack has16 targets; source throws at the seventeenth reached append and unlocks,
with no submission. Late missing interface and invalid direct local record also
fail explicitly. No transport/device/wrapper creator success is invented.

0046CB20 commands0/2 use SAME active scene and borrowed spectator frames. Command0
writes26D first, optionally copies active world position, restores own camera
BEFORE clearing26E, otherwise finds literal `spectator` with scene virtual7C
flags4. Final false refreshes current player. Command2 offline7 is a true no-op;
non7 writes26E first, false restores own camera; true counts spectator0..missing
and draws exactly once even if count0, then stores274 before final lookup. Raw
0080FF64 is FLOAT bits38000100, slightly below1/32767. Native FILD→FMUL→FIMUL→
DOUBLE spill→007E6368 floor→FTOL matters: RAND32767 selects count-1. A FLOAT
product incorrectly rounds to count. Source keeps extended product then explicit
DOUBLE before floor. Spectator frames remain scene-owned, no new camera owner.

Index lookups preceded reuse/exports. WORK `human-normal-spawn-leaves-abi-v4`
repairs00612A60 fastcall ECXpeer/EDXScene identity and caller signatures in a
read-only rollback transaction. Some decompiler tail aliases in00611C90 remain
unreliable; authority work must inspect raw assembly, not substitute SpawnPoint
pointer for identity.004BF4A0 arg1/2 and00496120 mode are DWORDs despite provisional
BYTE export parameters; reached all-zero branch is confirmed by underlying raw
stack expressions. Existing `world-retirement-hud-callees/004b27b0.c` confirms
false-body order. `human-spawn-hud-owner-offsets.asm`, spectator-selection/round
assembly/constants JSON, voice-constructor/debug assembly and indexes retain
exact evidence. No original/DLL/Windows output was executed.

Focused ASan/UBSan/LSan PASS `human-spawn-flow-native`, `human-spawn-hud-native`,
and earlier unchanged `human-spawn-camera-native`: modes0..7 x100, actual packet
bitstream/stat permutation/client generation-before-truncation modes2/3/7 x100,
match/HUD ordering and required-failure prefixes, live peer reallocation/late
interface/overflow unlock x100, camera RAND_MAX boundary and100x10x5 selections.
Factory/Scene/engine/HUD endpoint doubles are explicit; actual HUD policy uses
actual dialog/message TextState leaves, while inventory/map/cursor/postbind are
doubles. Not a full World/render/transport run. Strict actual World/Human i386 and
sequential latest launcher/startup-probe compile/link PASS in
`human-normal-spawn-{host,application}-i386.log`. Temporary audits remain ignored
until full selected checkbox completes.

[Receipt](checkpoints/p2.2-human-presentation-spawn-20261112.json) adds14 client
partials and extends3, no promotions:34 complete/2635 partial (2630 client+5 engine)
of18783. Whole selected Human checkbox, application loading/rendering and desktop
acceptance remain OPEN. Continue00611C90 authority positioning/publication, normal
entry/transport, required occupied owners and full containing reaction/frame/
failure/re-entry/teardown; successful source checkpoint does not end that scope.


### Authority spawn, actual zone owner and rejection/map delivery — 2026-11-12

00611C90 now has ordinary authority source, bound from actual session activation
through World services. Offline7 clears SpawnPoint9C, allocates first free reserved
identity and uses authored A4 clothing. Non7 keeps peer as the Scene identity and
clothing=-1. Initial authored pose/normalization is shared with the earlier verified
offline body. Styles1/2/3 use soldier.network_identity's actual **type37 zone**
005ED700, with config240/rule6 cycling. Failure queues real application0F(peer,1)
and returns before clothing/factory. Style1 applies0069E0A0. Other styles attempt
up to31 actual00475840 positions, ending at the last pose even when all are occupied.
Missing selected SpawnPoint is an explicit native required-dereference failure.
Default clothing transfer and stack splitting precede SAME00612A60; A8 publication
precedes local match phase, authority activation, late presentation activation,
Player side/flags, voice, packet/direct presentation and final late Script context
Use(false). A8/mode are reread between receivers; no rollback is invented.

The captured Scene identity is retained for packet/direct publication, even if
native first-free returned-1 and the registry appended an ordinary actor. Do not
replace this local with the resulting Human's id10. Raw caller evidence resolves
Ghidra's misleading tail `this`/stack/vector aliases. Normal wire stats are original
ARG2, after soldier serialization, NOT current Human properties; missing ARG2
fails there with active stream/prefix retained. Direct game1C==1 instead holds418,
optionally invokes0072E940 recursively, then006123A0 with **NULL stats**, irrespective
of ARG2. Peer name/transport are read under418; stream remains actual process-owned
module13. Focused policy tests cover late A8 replacement and fresh mode after
activation,31 exhausted choices, direct recursive lock and required stat failure.

00475840 scans SAME **Scene2 identity10/14 vector**, including32 reserved slots and
holes; it does not scan active actors. Capture count once, count eligible type10/
kind90/flag9C/(side94==side or2), draw once even with no candidates, scan again.
FILD CRT→FMUL FLOAT38000100→FIMUL candidate count→FTOL keeps the product extended,
unlike spectator camera's DOUBLE/floor step. RAND32767 selects the last candidate.
0069D6A0 uses actual shared00715510/007184C0 downward sphere sweep: height2,radius.2,
start at pose.y+height, decrement height by returned distance+.01 BEFORE hit checks.
Continue past other surfaces until no hit/height<=.01; actual frameEC actor kind3/4
with descriptor flags40/C0 reports occupancy. No synthetic empty collision query.
0069E0A0 scans live identities, selects highest signed zone90 for side0, last zero
zone90 for side1, default identity0 otherwise; then flattens and normalizes direction
toward that actual zone. Fourth uninitialized native output lane stays a defined
caller guard. No broad zone simulation is implied.

**Lookup correction:** original00475D50 command lookup differs from00475E00 Human
lookup and00475DD0 physical slot lookup. Presentation spawn and authority zone/
orientation adapters now call SAME existing command resolver, retaining its reverse
pending-retirement fallback for empty reserved slots only. A non-Human result in
presentation or non-zone result for a team spawn fails explicitly, instead of
silently searching another registry. Existing resolver body was already verified;
no additional00475D50 credit or repeat audit was taken.

The actual zone owner005EBED0 is now constructed by the SAME Scene2 factory and
loads native005EDA60 legacy/version8001 fields:90/94/98/A0,three16-byte namesAE,
A4/A8, optionalAC, then E4/9C from98. Bounded invalid strings/truncated files fail.
005ED110 common activation FIRST;118 set before acquiring three scene7C/flags80
named flag frames, actual005ED220-shaped vertex instance, then scene7C/flags20
`<shortname>spawn` frame group. F0/F1 reset after acquisition; presentation sets
98=-1/AD=0 then005EC8B0. No second Scene, animation definitions or vertex registry.
005ED260 clears118, releases flags0..2, unregisters/frees actor-owned vertex, then
releases spawn group; it **does not** invoke common00610510. Separate typed borrowers
stay published through frame-release callbacks and clear AFTER release. Typed null
vertex/borrower guards replace native dangling/unwritten pointers only after their
valid lifetime; they are not additional original constructor defaults/ABI credit.

005EC8B0 optional actual HUD objective gate precedes forward flag visibility/update.
Reread the flag borrower after callbacks. Matching side uses SAME `vlajka` definition/
OBJ_ child and actual MissionVertexInstances: finished→reset geometry→unregister→
bind→register→ping_pong=false, maxLOD=definition.sizes.size-1,duration210,start0,
repeat=true,finished=false. Commit98=E4 LAST. Frame retirement during visibility
fails at the next required borrower instead of using a released cached frame.
005ED550 reads actual retained spawn-group child position then direction WORLD
caches; normalization keeps(Y²+X²)+Z² extended, reloads stored FLOAT for sqrt, keeps
inverse extended for all stores.005ED700 noncycling end returns before child read;
otherwise pose→cursor++→optional modulo fresh count→if exhausted publish neutral
owner98=2 through actual005ED320 module13 subtype2/side2bits→occupancy. At32 attempts
returnfalse even if that last pose was free; style2 stops after one probe and returns
true even if occupied. Native branch ordering and failure prefixes are retained.

Zone non7 decision/presentation005EC060/005EC540 still REQUIRE their containment/
score/frame receivers. Both original functions genuinely return immediately in
mode7; containing actor dispatch now preserves that guard and throws explicitly
for the unrecovered non7 bodies. No005EC060/005EC540 credit. Type37 simulation was
not replaced with a successful generic actor tick. HUD map group/side now borrow
this SAME zone A4/98 through its Scene2 base identity.

Application0F is a scalar FIFO producer: enqueue under actual application2C in non7,
drain writes peer5/reason1 after normal20/command32 envelope.00728450 reads those
fields before comparing local identity, with no mode/game gate before a matching
00699620. Result88 outside0/3 is a real no-op. Otherwise90=2-reason precedes required
first live soldier.network_identity=-1 and0049F040(true). Absent current World,
empty live squad, missing map/HUD/input services and map failure keep their reached
prefixes. This does not enable transport/entry or create substitute live records.

0049F040(true) now calls actual004A5630(true,false), then late center E0&1→HUD21,
then message mode0 (including required offline pause). Missing map center remains
an original no-op inside004A5630, not a fabricated loaded map. Present center:
24=0, show center/map, and kindnot0/1 select SAME004ABAF0 pointer0→004A7180→004A78E0.
004ABAF0 hides old pointer BEFORE index store, publishes actual table borrower,
conditionally stores position20 XYZ/E0 maskFFFFFEDF|40000000 when low-byte sign bit
clear, then shows/updates; -1 retains field00/no update, equal index returns.
Invalid indices fail after the original index mutation. No new pointer/frame owner.

004A7180 forward-updates actual zone markers004A7A70/004A7A20 from SAME zone98:
store changed side before optional allied(side1) hide/show/update and axis(side0)
hide/show/update. Then reverse-match actual first live soldier's positive network
identity; matching nonneutral side positions/tags/shows selection marker, mismatched
owner clears that soldier's network identity; final marker hide/show/update always
requires its actual artwork.004A78E0 updates live goals forward.004A7C60 queries
actual local peer64 separately (neutral2 returns), stores changed descriptor status/
side before visibility/text. Status0 shows root/axis,1 root/allied,2 shows root while
RETAINING faction visibility;4 hides all and clears text. Captions use actual kind12
metadata1C/20 and B63/B64 success/failure append with native45 flags. Missing owner/
artwork/local peer and localization failures remain explicit. SAME process live
squad and control records are borrowed by HudWorld, with no HUD copy of either.

Index preceded every new export or original reuse. WORK
`human-authority-spawn-position-abi[-v2,-v3]`, `human-spawn-rejection-abi`,
`human-spawn-map-zone-frame-abi`, `human-spawn-map-show-receivers-abi` are read-only
rollback exports with no ERROR. ABI caveats:007184C0 arg5 is a normal-output pointer
(provisional DWORD type, reachedNULL), and only Scene/raw evidence resolves caller
stack aliases. Raw authority caller/zone constructor/vtable/pose/owner packet/
occupancy/rejection receiver/map-zone side and PE constant JSON pin exact claims.
0083E564=`spawn`,0080F574=.01,0080F748=DOUBLE2 and0080FB00=DOUBLE1 establish the
neutral-owner command's two-bit width. Existing menu-route-extra00728450 C,
human-objective-reused-originals0072A0B0 C and session-finish-required00475D50 C
were reused. No original/DLL/Windows output ran.

Focused ASan/UBSan/LSan PASS authority/zone/occupancy/packet source x100, map/
pointer/goal TextState x100, actual process FIFO/received rejection and prior
application messages, plus spawn decoder/presentation regression after packet
extension. Frame/vertex-registry/factory/geometry/HUD/localization/peer endpoints
are explicit doubles, not full World/transport/render. Strict actual complete World/
authority Human/factory/HUD/source lifecycle i386 and sequential latest launcher/
probe compile/link PASS `human-authority-spawn-{host,application}-i386.log`.
Temporary audits remain ignored until selected checkbox completion.

[Receipt](checkpoints/p2.2-human-authority-spawn-map-20261112.json) adds14 partials
and extends7, no promotions:34 complete/2649 partial (2644 client+5 engine) of18783.
005EBFD0/005EDA60/005ED110/005ED260 are ABSENT from the frozen inventory: ZERO
new credit despite implemented source. Existing sweep/vertex bodies and delegated
engine calls receive no new credit. Full selected Human checkbox, application
loading/rendering and user desktop acceptance remain OPEN. Continue full control
entry/transport, non7 zone/runtime/occupied owners and containing reaction/frame/
failure/re-entry/teardown; this verified source cohort does not finish that scope.

### Entry pending-input reset and optional voice owner — 2026-11-12

00731610 now borrows SAME process14, registered records and local packet FIFOs.
When requested, reread the current registry/count for each forward record and
reset its first then second list through0073E660(1,0,1)/(1,1,1). Absent/unconfigured
manager fails BEFORE queue selection/lock. A false reset byte never reads14.
Select authority FIFO only when mode bit1 is set, otherwise presentation, BEFORE
430. Unlink/free each node and update its selected count BEFORE releasing payload
bytes, still under430. No flush, other-FIFO clear, manager replacement, record
removal or embedded30 reset is introduced. Startup binds this distinct method;
full controller status9 remains unbound.

The SAME input Record now owns typed pending nodes/messages.0073E5A0 writes20
stamp and increments24 BEFORE node allocation.0073E600 decrements24, unlinks the
front node and returns SAME payload, without deleting it.0073E660 repeats the
SAME retained node until count<1; force deletes irrespective of count, then
0073E6E0 unlinks/advances. The three actual Human message deleting receivers
0053B940/0053B8D0/0053B900 have no resource tail or refcount guard; no new credit
for those bodies. Original0073E4C0 can append SAME allocation to both lists, but
bound Player/AI flag40=false normally prevents the second-list alias. Forced
first-list deletion leaves a stale second-list pointer natively. Source lifetime
tokens reject that later access before UAF, preserving the failing prefix instead
of deferring first deletion. Positive-count reset without decrement would loop
forever natively; source reports this nonprogress with the node intact. Counters
remain wrapping DWORDs. Canonical RAII allocation bookkeeping is not another
native input queue or refcount: source destruction reclaims unconsumed allocations
AFTER native children/second-list/first-list node cleanup, where original0073DF70
would leak payloads. This explicit safety adaptation receives no equivalence claim.
No B0/Car decoder producer was invented: flag20 packet history still requires the
actual decoder and fails.

00724110 now has its separate actual process prefix:5044=1, descriptor4E88 gate,
THEN current profile byte38 gate, optional00724370(false), THEN destructive
00723000 manager replacement. Startup borrows server/client from the EXISTING
ColdControlFrameState native00/04 fields; no parallel transport owners. It is not
yet invoked from received application0's full game/configuration protocol.

00724370 creates actual process0C wrapper00741DC0 with callback00722140/contextNULL
and publishes BEFORE graphics window lookup/initialization. Actual GetIGraph+48
uses existing engine6A240 binding. The initializer HRESULT is deliberately ignored.
Raw caller724418..724429 and RET32 resolve00741E20's EIGHT explicit arguments:
window,server-enable,client-enable,connection,session-mode,compression-GUID,
client-config-pointer,optional-playback-interface. The last two were incorrectly
inferred from decompiler stack aliases in earlier exports; only the v3 ABI export
and raw assembly establish the final source binding. The process creator passes
(0,1,selected00-or04,3,NULL,&[20,28,1,0,FFFFFFFE,0,0,0],NULL) after window.
Source uses actual CoCreateInstance(CLSCTX_INPROC_SERVER=1), pinned original GUIDs
and external COM vtables, with no success service or original execution.

00741EF0 creates temporary VoiceTest, tests default voice playback00819488 and
capture00819478 with flags2, retries only80150183 with10, then Release. SDK dsound.h
confirms DEF00002 playback / DEF00003 capture.00741FE0 publishes server08 before
Initialize(connection,context10,NULL,NULL,0); its0x24-byte session contains actual
mode and supplied compression GUID.00742090 publishes client04 before
Initialize(connection,callback0C,window,NULL,0). Build actual0x40-byte audio config:
playback GUID at08, optional interface18, capture GUID1C, zero2C,window30,zero34..3C.
Connect(audio,clientConfig,1), then SetTransmitTargets({0},1,0), then two AudioConfig
queries at40. Accept first8015001E, zero returned allocation, set its header40,
query again and store wrapper14=(word04>>2)&1 ONLY on success. Keep every failed
COM acquisition prefix for later retirement. Source RAII frees the temporary query
buffer even on negative second query, which leaks natively; undersized result is
an explicit required guard. No settings/config pointer is retained after Connect.

00724450→00741DF0/00741E10/00741E80 retire actual current0C.00742240 calls client
Disconnect(1), rereads04, Release, THEN null04;00742270 follows for server08 with
StopSession(0). Global0C stays published through this deleting receiver and clears
AFTER it. Existing settings00742210 and transmit00724570 now borrow SAME client04.
Repeated creation natively overwrites/leaks old0C; source retains only allocation
bookkeeping and reclaims abandoned wrappers at final process fallback, never an
atomic replacement or early native close. Startup's required cold/non7 reset guard
is unchanged: a live network/voice prefix still reports required unsupported reset;
explicit fallback closes acquired voice allocations before inputs/locks. Full
0072E4E0 positive networking retirement is not claimed.00722140 is actual XOR/RET12
no-op HRESULT0 and ABSENT frozen inventory: zero credit.

Human delayed/deactivation attachment80 now calls the existing SAME Car004D9A40 /
StaticWeapon00604630 seat receiver, before160/164 invalidation. This closes the
previous unconditional service throw without copying occupants/control/cameras.
Existing seat bodies and their native signed bounds/failed-prefix checks are
unchanged; this added containing binding is compile-verified, not a new complete
occupied World/frame audit. Both seat bodies are absent frozen: zero new credit.

Focused native ASan/UBSan/LSan PASS actual process input reset/owned queues across
modes1/2/3/7 x100, pre-lock mode capture, absent-manager gates, same payload take,
wrapping/repeated-node/forced-alias/nonprogress failures. Optional voice PASS x100
and all HRESULT prefixes, test retry, shared settings view, required window failure,
ignored initializer before replacement, both-interface retirement and repeated
publication fallback. COM/window/clock/locks/replacement endpoints are explicit
doubles; existing real manager replacement was verified earlier. Strict full actual
World/Human, entry/reset/voice/retirement i386 source and sequential final launcher/
probe compile/link PASS; Windows outputs were not run. The receipt pins final logs.

[Entry-input/voice receipt](checkpoints/p2.2-human-entry-input-voice-20261112.json)
adds18 frozen client partials, extends0073DF70 and005A7FE0, no promotions:34 complete/
2667 partial (2662 client+5 engine) of18783. Full selected Human checkbox, application
loading and desktop acceptance remain OPEN. Next: actual00730A40 connection6C/334
producer/worker lifecycle (originals reused read-only and pinned), full status9 /
received0 game publication/failure protocol; positive runtime kind2/9C links, non7
zone/frame dependencies and full containing Human failure/re-entry/retirement.


### Containing blast-to-Human damage — 2026-11-12

The actual GameBlastWaves bridge now routes Human material/D4 through SAME
containing World loading lifetime. D8=0059C2A0 is genuine constant66, independent
of hit frame, and absent frozen: no new credit.006DDD40 raw6DDE52..6DDE72 pushes
six D4 arguments: kind(NULL-source?4:2), amount, point, impulse, SAME source1C,
final hit-kindZERO.
Request/Record source1C was incorrectly represented as uint32; it is now typed
Actor*, preserving a full-width borrowed Human pointer. No refcount, identity
lookup, serialized source or new Human allocation is introduced. The source
outlives its original required native use or the host map guard fails explicitly.
World validates target/source by comparing actual Human owner-map keys BEFORE
dereferencing a possibly retired borrower; require actual published lifetime and
reject in-progress/retired ownership. Invoke existing lifetime.damage→005A1C90,
which retains all existing health/policy/impulse/B0 command/effect ordering. No new
health-only damage method or substitute body. The added guard is compile-verified
in actual World; native fixtures use live valid borrowers.

A concrete terminology correction:006DDD40 frameEC+14 (word5) points to SAME
Versions Master, as existing VersionModuleLoading::add and original0043C6A0
confirm.0043C6A0 iterates part14/18, calls0043B000 and rechecks the global master
registry after callbacks. This is not a rigid-body impulse. The historical source
endpoint name `blast_physics_impulse` is retained; its positive missing Versions
part-transition path still throws with a corrected diagnostic. Other derived
actor material/D4, Versions transitions and full producer/application integration
remain required. No new credit for these absent bodies or unchanged blast math.

Focused `human-containing-blast-native` ASan/UBSan/LSan PASS actual process policy
→Human material66→SAME source lifetime gateway→actual005A1C90→real Player B0 command
queue, sourced and NULL modes1/2/3/7 x100 (800 lifetimes). Check original kind2/4,
final hit0, pointer identity, material absorption strict threshold, authority gate,
missing lifetime, failed hit before health/queue mutation, retained-node release,
record's extra retirement tick. Geometry/ray/Versions/other damage leaves are
explicit doubles; Player Base bind is not full Player activation. Strict actual
World source/create/update/material/damage/guard i386 and final launcher/probe
compile/link PASS. No original/Windows output executes.

[Receipt](checkpoints/p2.2-human-containing-blast-20261112.json) extends4 existing
entries, adds0:34 complete/2667 partial, no promotions. Full selected Human
checkbox, application loading/rendering and desktop acceptance remain OPEN.
Runtime kind2/9C research now identifies original Mine factory005FDE30 allocating
0x120, ctor005FDEC0/vtable00812F08, actual scene name Actor_mine. Its timer94 arms
sphere0x42 after signed elapsed reaches2000, with model positionY+.2 and radii
.4 for item71, .15 otherwise; deactivation unregisters99 BEFORE common cleanup.
Full owner is NOT implemented or credited. WORK `human-runtime-kind2-*` pins raw
lifecycle/tick/vtable and final ABI v2 (005FEBB0 takes NO stack args); read-only
reused originals preserve provenance. The derived005FDFB0 exact index lookup was
late (after its export); its raw/vtable evidence preceded that export. Do not claim
that lookup happened before it. Continue actual runtime producers/lifetime, entry
protocol and containing frame/failure/re-entry/retirement.

### Runtime Mine owner and Human9C retirement — 2026-11-12

[Full owner/evidence contract](human-runtime-mine-source.md) now closes the actual
Mine type2 constructor/bind/timer/sphere/deactivation/packet leaf and positive
00479700 source ownership scan. SAME runtime registry/SceneEC/buckets/ShotMgr/
SOUND/module17; no parallel graph. Source actual Car5C and blastD8/D4 recognize
its verified native leaves. Payload native14/1C maps to serialized10/18 after the
omitted vtable; no positive-material ShotMgr branches are invented. Frame/packet/
placement producers still have their original required guards. [Receipt](checkpoints/p2.2-human-runtime-mine-20261112.json)
adds13 partials, extends6:34 complete/2680 partial, no promotions. Native sanitizer
800-lifetime owner audit passes with explicit endpoint/grid doubles; actual host/
i386/launcher compile/link only. Full selected checkbox, containing application
frame, failure/re-entry/teardown and desktop acceptance remain OPEN. Continue
00478B90/resource/ground/wire/module12 producers, entry and full Human dispatch.

### Normal Mine producers and positive Versions transitions — 2026-11-12

[Mine producer contract](human-runtime-mine-source.md#normal-producers-and-module12-envelopes--2026-11-12)
closes normal model/resource/ground/definition-inventory/wire/module12 envelopes
through the SAME existing owners (checkpoint59fee6f,5 partials). [Versions contract](version-damage-transitions.md)
closes actual positive frameEC5/Car/blast damage-stage, lazy category5 resources,
visual/metadata replacement, module7 packets, Script/Physics/SOUND/particles and
ordered Part/Master/descriptor/reload retirement. Native400-lifetime/real cache/
packet/debris producer source sanitizer checks and actual full World i386/final
launcher compile/link pass, with explicit endpoint doubles and no engine/desktop
claim. Adds12 partials/extends8:34 complete/2697 partial. Full selected Human
checkbox stays OPEN. Continue actual armed Mine005FEFB0 and00451990 explosion
producer, full entry and containing Human frame/failure/re-entry/ordered teardown.

### Armed Mine and actual explosion producer — 2026-11-12

[Full contract](human-mine-contact-source.md) now binds Human first-candidate
contact, Car wheel trigger, native source/family gates, unregister/wire/explosion
ordering and actual00451990→shared BlastWave initial damage→reused SOUND. Source
Human9C retirement during the explosion can delete the Mine before its contact
returns; local containing-runtime borrower preserves the original global access
without delaying destruction. Original70/71 data is type2/40bytes with radii5/8,
power2500/25000 and sound2. Source fixtures pass800 armed contacts and separately
800 explosion-to-Human/Player command paths with explicit endpoint doubles; strict
actual World composition and final launcher/probe pass. Adds4 partials/extends2:
34 complete/2701 partial, no promotions. Full selected Human checkbox stays OPEN.
Continue actual CarD4=004DBA10, other required damage/surface receivers, non7 entry
and containing Human frame/failure/re-entry/ordered teardown.


## Reached Car blast damage dependency —2026-11-12

[Car original/source contract](human-car-blast-damage-source.md) and
[source checkpoint](checkpoints/p2.2-human-car-blast-damage-20261112.json) bind
actual004DBA10 to SAME Car part/wheel/DM/body owners, positive zone/occupant
damage and native shared packet production.004E1A20 is embedded24E0 DM, distinct
from physics health. Terminal destruction, presentation/timed-DM and containing
Human frame remain required; this does not close the selected checkbox.


## Car packets and DM effect lifetimes —2026-11-12

[Original/source contract](human-car-dm-presentation-source.md) and
[checkpoint](checkpoints/p2.2-human-car-dm-presentation-20261112.json) bind actual
004DB360, DM particle/SOUND producers, same process00445740/00445800 retirement
and actual puncture sounds. Native unwritten EXPL_D8/smoke14 require honest
producer/empty-list proof; terminal authority/visual/frame consumers remain open.


## Authority Car destruction/nearby Human damage —2026-11-12

[Contract](human-car-destruction-source.md) and
[checkpoint](checkpoints/p2.2-human-car-destruction-20261112.json) bind actual
004DBF00/004DC580 to SAME Car/Script/Scene2/Human/random/packet owners, with
native partial-failure, water/source, wheel and radius-edge ordering. Queued
visual/physical reset, detachment, timed-DM and full containing Human paths remain.


## Queued Car physical/mesh/glass dependency —2026-11-12

[Contract](human-car-physical-reset-source.md) and
[checkpoint](checkpoints/p2.2-human-car-physical-reset-20261112.json) bind actual
004DC780→004E9DB0/004DCD10, immediate engine mesh callback, SAME retained glass
texture exchange/restore and seated Human opacity. Native focused sanitizers and
actual World/i386/launcher pass. Positive fragments/Item12/deformed mesh/timed-DM
and full Human entry/frame remain required; selected checkbox stays OPEN.


## Car glass fragments through actual Physics —2026-11-12

[Contract](human-car-fragments-source.md) and
[checkpoint](checkpoints/p2.2-human-car-fragments-20261112.json) bind actual
69C→GEffectsB4→00485BA0/00484F60 mesh subdivision/publication through SAME module2
Physics/fade lifetime. Engine/math/buffer/Physics endpoints are native-audit doubles;
actual host composition and launcher compile/link pass. Item12/timed/entry/full
Human frame remain required; selected checkbox remains OPEN.


## Detached Car Item12 launch/lifecycle —2026-11-12

[Contract](human-car-item12-source.md) and
[checkpoint](checkpoints/p2.2-human-car-item12-20261112.json) add actual Item12
factory/clone/launch, same rigid-body registry/buckets and expiry source. The
containing004C9000 authority simulation still fails before clock processing;
no lifetime-only success. Wheel695 is confirmed unwritten without a real
producer and remains a required read failure. Native owner/launch sanitizers
and actual World/i386/launcher pass. Continue actual wheel/part motion/contact,
Car timed-DM and full Human entry/frame; selected checkbox remains OPEN.

## Detached Car rigid-body source checkpoint —2026-11-12

[Actual motion/contact contract](human-car-rigid-motion-source.md) and
[receipt](checkpoints/p2.2-human-car-rigid-motion-20261112.json) now cover
004C9000→004C8EE0→wheel004C94B0/part004CB2B0 with the SAME rigid body, World
queries/exclusions, actor cache and buckets. Fresh part5F9 is confirmed
unproduced and fails BEFORE movement; warm-state checks explicitly seed a prior
byte. Wheel695 stays unproduced. Native focused sanitizers and strict actual
composition/launcher checks are source evidence with explicit engine/query
doubles, not trajectories/application/desktop acceptance. Continue Car event2,
timed-DM/deformed restore and full Human entry/frame; this checkbox remains OPEN.

Car wheel2/Human9 publication, actual Scene2 actor20 authority routing and timed
004E1990 now have a [source checkpoint](checkpoints/p2.2-human-car-contact-request-20261112.json),
with [contract/limits](human-car-contact-request-source.md). Fuel10/impulse11,
deformed restore and the full Human entry/frame remain required; no checkbox closure.

Car fuel10/collision impulse11 now have a [source checkpoint](checkpoints/p2.2-human-car-fuel-impulse-20261112.json)
through SAME packet/physical/activation clock owners. [Contract/limits](human-car-fuel-impulse-source.md).
Continue Car6/7/deformed/global backup/occupied restore, full entry and Human frame;
the selected reactions checkbox remains OPEN.
