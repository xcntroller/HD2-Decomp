# Human equipment loading — P2.2 source work

Human230 equipment is still an unfinished dependency of actual Human activation.
This source does not construct an activated Human, containing world or first
world frame. HumanSceneActor98/A0/A8 remain neutral cold fields. Ledger credit
is unchanged; original/engine DLL/Windows execution and desktop tests were not run.

## Holster loading and cleanup

`human_holsters.h` recovers0078CE20 and matching0078CDB0. Fingerprinted
human-holster-data.json preserves80 model names with original addresses,
10 set keys and24 marker names spanning14 groups. The definitions come from
0084F700 (stride0C), sets0084F6D8, marker/group records0084FAC8 (stride08).
007E88F0 compares ASCII names without case;007E5B30 truncates at the FIRST dot
following the last backslash. No invented filename normalization is applied.

The constructor saves model animation track7's clip, integer-truncated weight,
time, flags and enabled state. It obtains the real shared animation `tecko`
through StartupSession's SAME GameAnimations/category3 cache, installs it on
track7 with weight1/flags2 and advances the eight-track vector with only slot7=100.
Actual c/d/o frames are borrowed while the model has that pose. The native14
records are zeroed before the selected set is constructed.

Each nonnegative set loads through the SAME model cache with flags10. Set−1
is a real no-holster definition, including civilian models. Unknown model names
and failed required set loads reject and restore prior track state. Optional
missing part markers remain absent. For each found part, retain happens before
model+80 removes it; parent linking preserves world pose with flag1, then hides
that part. Four frames fit each group; native excess members remain in the
temporary model. Group4 links to d. EN06's marker1 links to o,83/51 to d and
others to c. US03's81/82/83 link to d, others to c. Other models link83 to d,
42 to d only when the definition flag is zero, and remaining markers to c.

The temporary model unlinks/releases after extraction. Track7 weight becomes0,
the same100 vector advances again, track7 is removed, then the original enabled
track is restored with FLOAT(truncated_integer_weight), saved flags/time. This
integer loss is original behavior, not a reconstruction rounding change.
Failure guards restore the track and retire any partially retained frames.
0078CDB0 unlinks/releases14 groups ascending, all four slots regardless of count,
THEN releases global0098ADB0. It is not a permanent pose reference or counted
per-Human lease: a later constructor reacquires through the same animation cache.

## Equipment state and holster counts

`human_equipment_state.h` supplies0078C980 reset,00791290 selection and
0078CC90 marker binding. Typed borrowed model links replace native pointer words;
owning equipment/model producers are still required. Identity54 updates before
selection unless a nonnegative current identity would be replaced by−10000.
Kinds0/1/2/3 select native24/28/20/18 respectively; null models skip their stores.
Kind3 resets timerD4=700. Kind4 writes identity−9 and falls through to the same
1C receiver used by5/6/7/9, including clearing it with a null model. Bindings use
exact HW1/HW2, d/f/u/q names; the two volumes are enabled only when BOTH exist.

00791720 maps the SAME catalogue definition kind/class/identifier/field0C to
14 possible holster groups or14 for no group. Definition field0C is serialized3C,
confirmed by007DFAB0's common serializer.007919C0 counts groups2/3 at three
items per model, other groups at one. Original repeated visible/opacity calls
for those triples are preserved; traversal after all real models are visible
is safely bounded because it has no further native calls. OBJ_ opacity uses
00789C80's clamp above1, leaving smaller values/NaNs unchanged.

## Verification and immediate continuation

Focused native ASan/UBSan passes80 definitions, all10 keys/24 marker bindings,
EN06/US03/default parents, native track restoration, negative set selection,
missing/unknown/repeat cleanup,141 frame endpoint failures, equipment selectors,
holster mapping/counts and opacity. Strict i386 instantiates actual
GameActorServices/GameAnimations and marker binding. Application compile/link
passes. Native frame/matrix/resource endpoints are fixtures, not engine execution.

The temporary sanitizer directory vanished across the latest resume. Matching
16.2.1-3.fc44 x86_64 libasan/libubsan packages were resolved from fresh official
Fedora metadata, downloaded and checked with rpmkeys (digests/signatures OK),
then extracted ONLY to /tmp. Unversioned libasan.so/libubsan.so symlinks in that
directory are required to override the installed compiler's absolute linker
scripts. No packages or original game files were installed/modified.

The ordinary230 activation/attachment scope below now has source. Continue
actual DD0/CD0 animation/C00 composition before DD0/E0/230 in005AB330. The new read-only
HD2HumanEquipmentABI.java improves known stdcall engine/frame signatures;
human-equipment-geometry-abi/ has the latest decompilation. Its explicitly logged
unresolved virtual sites must be checked in assembly before using those cases.
No implementation claim relies on those unresolved signatures.

New evidence human-equipment-activation/ contains00791290,0078DF60,007919C0,
00791720,007931B0,007B90B0,0077F2F0.007DF850's source proves a separate four-part
mask/finL/finR/tank cache only for definition231, not another catalogue. Its
native50 cache now has source as described below; live equipment attachment remains. The attempted
007DFD10 export overlapped an existing body; no claim relies on that address.
007DF850 was separately exported before the error; its body is checked directly.

## Scuba cache and opacity dependencies now in source

`item_scuba_models.h` implements007DF850's definition231-only native50 cache,
using the SAME selected catalogue and model resolver/resource[2]+actual suffix.
It looks up mask/finL/finR/tank with mask1 and uses existing007DC3A0 frame cloning.
Missing markers stay null; a successful four-slot allocation is cached even when
some slots are absent. Failed model loads are retried, matching the native null
cache. Definitions outside231 still return null after this cache is populated.
007DF430 releases four cached root references directly (not recursive007DC460);
that distinction is preserved. RuntimeItemServices clears a definition's primary30
cache before its50 parts. All other primary-cache behavior remains as previously
verified. Native fixtures cover leaf part roots; arbitrary nested engine model
content and live equipment attachment remain unverified.

`human_opacity_control.h` supplies DD0+DA4 defaults/reset00789CA0 with SAME CRT,
getter00789C80, smoothing00789F60, scheduler00789CD0 and actual sector ambient
selection00789FC0. Requested opacity clamps only above1. Smoothing stores the
doubled elapsed*.001f step before changing current opacity, retaining the native
NaN decreasing branch and extended comparison before clamping. Scheduler uses
thresholds250/500/1000/10000000/1400 by mode and signed wrapping timer addition.
A changed opacity requires the real00486510 module+54 consumer; no default is
provided. Unchanged loading opacity follows the original no-callback branch.
Ambient selection scans actual visible type4 sector lights; strict RGB maxima
fall through to Z on tied maxima, even if Z is smaller. No eligible light yields1;
a missing required sector is rejected. Host reads the real sector/light fields.

Equipment offsets0098AD80/90/A0 are BSS initialized by0078C960/930/900.
`human_equipment_offsets.h` preserves their exact FLOAT bit patterns as hex
literals. Fingerprinted JSON and assembly preserve each immediate MOV; the BSS
was not treated as file-backed bytes. Editor position changes remain outside
this ordinary-loading interface.6950 constructor/normal bind leave62B0 null;
007B85A0's optional call is therefore absent in the supported ordinary branch.

Focused ASan/UBSan passes scuba gates/shared catalogue/exact dotted key/four
roots/lazy/missing/retry/ordered release and18 endpoint failures; opacity shared
random/clamp/smoothing/NaN, six scheduler modes, mandatory module failure and
11 ambient endpoints. Strict actual host i386 and affected application compile/
link pass. These are source/fixture checks, not engine or desktop verification.

## Loading attachment and activation source

`human_equipment_attachments.h` now supplies0078DF60 cases0/1/2/3/10.
The SAME230 scalar/borrowed state, selected catalogue, primary item owners and
separate definition231 cache are used. Cases4..9 reject until their action
producers are reconstructed; the whole0078DF60 remains partial. These source
owners still require a containing DD0/inline98 binding and actual consumers.

Cases0/1 preserve link flag2, local Z direction, original equipment offsets,
OBJ_+A4 opacity and007920B0 refresh. Definition231 clones four instance roots
mask/finL/finR/tank from its separate cached roots. Missing required parts reject
before attachment. Repeated calls reuse those instance roots;28 borrows tank.
The instance roots unlink and release directly, ascending, as0078DE30 does.
Native20's back model preserves backlink/mag1 lookups, local direction Y,
second-row cross first-row rotation axis, exact angle/C4/C8 DWORD constants,
position restoration between the first two rotations, final X rotation and CC
adjustment. It returns without a visibility refresh or extra final update.

Case3 uses HW1/flag6, resets D4, sets face commands00781300/007812F0, and calls
the actual6950 weapon binding. Its SOUND request is mandatory when definition34
is not−1 and005A0890/0069A4B0 does not suppress it: group3, definition native40,
zero position/flags, relative Human model005A2AD0, owner Human. No playback default
is supplied. Case10 preserves timer/life gating, temporary50 bit0 restoration,
unsupported-kind hiding, backlink placement with/without20/28, and the original
mag1/other-back-model rotation variants. The shared transform exports and frame
local-matrix/scale/flag writes are actual checked host endpoints; DC padding is
not represented as a recovered value.

`human_equipment_activation.h` supplies0078CA60's ordinary loading body and
0078DE30's ordinary cleanup. It creates/names the real kind2 special light,
binds volumes/markers, selects actual holders3/2/5/0, resets DD0+C38/C3C through
a mandatory receiver, passes definition virtual0C to E0, sets holder3's deadline
to wrapping game28+1000, and consumes actual holster stacks/counts. Cleanup is
light release,6950 weapon clear, four scuba roots, then holsters. Editor mode7
rejects;007931B0 has no calls in the supported ordinary branch. This activation
body is not yet composed into a live Human/world.

Definition virtual0C is100 for basic definitions007E0420, otherwise derived[0]
via007E0520/007E0680. Holder virtual4C is007D68B0, writing native18.
`human_equipment_command.h` and `HumanLookControl::equipment_command` supply
0077F2F0 on the E0 owner (not DD0+C00): actual DD0+A20/C38 gates, overrideB0,
changed54, request40 bit100 and deferredC8 bit100. Pointer slotsA4/A8 remain typed.
Face command setters write native78/64 respectively without inventing transitions.

## Weapon binding, shadow models and visibility

`human_weapon_binding.h`, integrated into `HumanModelGeometry`, supplies
007B90B0/007B9260. It retains one actual item-model reference, borrows LW/RW/BUTT,
keeps same-model selection unchanged, maps kind to62CC, preserves6304/62D8 masks,
and selects/toggles actual hw1/hw2 parents. First-hand placement uses RW only;
second-hand placement falls back to LW. The offset is transformed by the hand's
transpose rotation, with original row/read and arithmetic order. Missing required
hand markers reject. The ordinary007B85A0 face-flag branch performs no call because
62B0 remains null in the already supported ordinary binding.

`effects_shadow_models.h` supplies GEffects004894E0/00486DF0 and CD0's
00758BF0/00758C60 dependencies. It uses StartupSession's SAME category4 cache
008AEBDC and existing B700 environment. Enabled mode bit2 selects real kind15
shadow templates (`shdpuppet` for Humans, otherwise `s` plus model basename).
Names truncate at the first dot; the exact shadow name prefix is `._shad.`.
Owned old shadows are removed; new instances copy the cached template, choose
owned marker `a` or the model parent, and enter the model's actual frame table.
Direction, dimensions, range, scale and distance use real environment words;
sine-squared uses QWORD00810390. Human refresh retains the returned frame,
overrides distance to20, then applies model visibility. Opacity changes call
model+50, distinct from OBJ_+A4. Disabled mode/missing model follows the original
no-call branch; an unavailable optional shadow template returns null.

The shadow lifetime intentionally performs at most two releases after hide/unlink:
two for signed E4>1, one for E4==1, none for E4<=0. Targeted original engine
ownership evidence confirms model+7C100341C0 retains, model+8010034260 removes/
releases only positive counts, Release1001C370 dispatches destruction at zero,
and shadow1004E7A0→base1001C1F0 removes itself from its model. This resolves the
model-table lifetime without inventing a separate shadow registry. Shadow owners
must close before their borrowed Human models/frames/cache.

`human_equipment_visibility.h` supplies007920B0 with real shadow refresh as a
required consumer. Temporary weapon names are sm/me/lo/panzer; bag size<=30
uses smlbag, larger/unordered uses bigbag; head payload selects helm/brig/lod.
Names and Human visibility restore before shadow mode selection. Exception guards
restore mutations too. The original skipped-save paths restore an empty equipment
name. Final visibility preserves the previous visibility/player-view override,
mode thresholds and animation identity159 exception.

Focused ASan/UBSan checks pass170 attachment endpoint failures,13 weapon-binding
failures,22 shadow/visibility failures and27 activation failures, plus native
holder/stack, mask/toggle, name/threshold, mode, ownership and repeated cleanup
cases. One initial shadow fixture closed its borrowed model too soon; ASan caught
that fixture lifetime error. The fixture now closes the shadow before the model,
as required by the source contract, and passes. Strict i386 instantiates actual
GameActorServices/RuntimeItemServices/GameMissionItems/GameAnimations/Human actor
interfaces. Actual engine calls and playback remain unexecuted fixtures; this
is source-only progress, not activated-Human or desktop delivery.

## Inventory model handoff — 2026-11-09

`human_equipment_transfers.h` now supplies00792590/00792720/00792AB0 inventory
notifications on the SAME230 state, holsters, attachments and DD0/E0. Native
4/0C are typed equipment-peer borrowers; they never occupy native DWORD slots.
Alive shoulder2→hand3 retains the source model until007913D0 event3 removes
source kind2 and attaches hand kind3. Holster1 changes actual model counts through
00791910/007919C0; an underflow clamps the count and returns before visibility.
DD0 DC8/7C4/A20 stores and forced E0 B0=1→0077F2F0→B0=0 are explicit. Events0..3
and inventory removal cases0..3 are supplied; action-model producers/events
remain separate and reject.007B96C0 uses FLOAT631C×200 (0080F6F8), signed64
truncate/EAX6314, sets62D8 high bit only when enabling, then6304 operation bits.

Focused native ASan/UBSan and strict actual-host i386 checks passed before the
PC crash; their logs remain in WORK/human-equipment-transfer-*. Temporary fixture
files were lost in that crash before their hashes were recorded. Do not describe
them as fingerprinted removals. The strict check instantiated actual Human,
GameMissionItems, GameActorServices, attachments, holsters and equipment services,
with required DD0/world/Players/presentation declarations. No DLL was executed.
The subsequent checkpoint header and app compile/link checks pass. No actual
animation tick, whole Human activation, world integration or desktop delivery.
