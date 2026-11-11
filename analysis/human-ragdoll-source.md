# Human reactions and ragdoll source

P22-HUM's second plan checkbox remains open. Current work continues the same
body/point owners. Application mission loading and desktop acceptance remain open.

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

## Continue the full checkbox

Bind real007A7470 dispatch after completing its required rest-surface and retained-clip branches. The collision, sleep and substep kernels above are in source.
007B3C80's retained clip path and remaining007B0910 reason/source branches,
0074D510 and positive obstruction/contact/fall damage remain required.

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
