# Human B0 state dependencies — implementation in progress

2026-11-05. Continue full P2.2. Human98/B0 remain cold in the application; no
activated Human/containing world, readiness, desktop result or new ledger credit.
[Previous verified DD0/inline98/230 composition](human-control-loading.md).

## New Human navigation receiver

Human B0's005832F0/0056F200 calls **0051D780**, not the previously recovered
0051CCD0 point-centre nearest query. `MissionNavigation::nearest_volume` implements
0051D780/0051D160 over the SAME decoded table. Distance is max(0, Euclidean
centre distance − prefix13/16), squared. Minimum is squared only when positive,
then original.001f is added. Both minimum and best comparisons are strict.

Band selection shares00524750/0051CBA0. Initial left margin subtracts16; right
is10−left−32. Negative margins force adjacent-band traversal. Candidate Z bound
subtracts16; full-distance bound subtracts another10. These reflect maximum
byte13 radius255/16, not a guessed collision scale. Preserve original pivot,
forward then reverse scan, float best stores and early-zero behavior. Reverse
scan stores X and stores/reuses Y in its product; forward scan keeps those
intermediates extended. NaN/overflow/invalid ordering rejects rather than invoking
undefined traversal. Existing point-centre query is unchanged.

Focused native ASan/UBSan compares3000 queries on both actual Tutorial/ALPS1
captured tables with independent exhaustive sphere distance. It also checks
radius-driven adjacent-band selection, strict lower boundary, empty table,
height gate, nonfinite rejection and original fourth-lane handling in B0 below.
Strict i386 query compilation passes. Constants/assembly/exports and logs are
under analysis/work/mission-world-20261103/human-navigation-volume-*.

## Common B0 loading state

`human_state_base_loading.h` supplies ordinary00582FF0/005832F0/00583690/00584830.
Native pointer44/40/7C/A0/A4/A8 are typed Human/command/input/navigation/record/item
borrowers, not pointer truncations into scalar DWORDs. It uses actual Human B4
command storage, holder3 entry, model+24, shared navigation and required real
registered input. Input lookup is0073F5F0(id,true), not a new per-state registry.
Editor7 explicitly rejects until its own consumers exist.

Cold binding writes flags2104, sentinels60/6C=−1, B0=10000000 (an integer, not a
pointer to00989680),5C=4000, and normalized live model direction into10/20,
including the fourth word. This normalizer's x87 addition is(Y²+Z²)+X², with the
stored float reloaded for sqrt, epsilon1e−8 and original tiny-vector axis fallback.
Copy binding preserves old vectors/flags/sentinels/bytesAC/AD/B0 and masks50 with
3FFFFFFF. Tail resets54/74/78 and derives4C=50&7.

00583690 removes input in ordinary mode and clears Human44. It is an explicit
receiver: base destruction only destroys two vectors. Player/AI+20 receivers do
not implicitly call this base close. Thus switching derived state must follow its
actual vtable receiver, not a generic destructor that removes shared input.
Focused native checks cover constructor/bind/copy/mask/navigation/item/input,
normalization/fourth lane/tiny fallback, shared-input destruction semantics and
four required-boundary failures. Actual host i386 instantiation uses the real
Human and frame adapters, but its input registry is still a required interface.

## Researched full containing boundary

005A3F20 constructs state1/3/4/5/6/7/8, publishes the new pointer, calls its+1C
(Human,old), then old+20 and+18(1). The exact vtable targets are preserved in
human-state-vtable-loading.json. These derived callbacks remain necessary; base
binding is not successful AI/player activation.0056EE90 owns five genuine state
subobjects;0056F0E0 tears them down in49/4A/4D/4C/4B order then base vectors.
0056F200 registers callbacks and initializes real motion/navigation commands;
005478C0 and0053B9A0 have further actual AI/player and task consumers.

0073F2B0 constructs an80-byte input record0073DDC0, assigns id1C, applies actual
manager defaults0073F4B0 and appends.0073F5F0 returns the first matching id, creating
only when requested.0073F530 removes membership in order and destroys the actual
record. Its record owns a real580-byte interpolation/history child0073DA50 and
two lists; clock/frequency, manager callbacks and history setters are real
requirements. No successful input, transport or AI stubs are supplied.
Current exports: human-state-script-dependencies/, human-state-loading-receivers/,
human-state-bind-dependencies/, human-state-query-input/, human-state-owned-input/,
human-input-record-lifetime/, human-input-history/ and human-input-value-loading/.
Latest value-loading export must be checked before use if interrupted.

## Actual script programs available for next work

Scripts.dta (ISD0) base archive has3181 entries,146 underSCRIPTS\\TUTORIAL\\.
Read-only extraction uses existing tools/extract_dta_member.py with startup keys
CF761298:0AB4E72D (extractor's byte key packs key2 then key1). Per-member hashes
and previews are in tutorial-script-programs-base.json. These are actual textual
programs containing Block, OnUse, Whenever, If, Delay, Frame/Integer variables,
ScriptAssign and real actor/module commands.114 selected bindings are recorded
separately; bindings are not programs. Patch/rw_data overrides remain authoritative
at runtime. No archive or original file was modified or executed.

Required next: complete actual input/history and derived B0 owners,0059F2C0/
0059FA60 containing Human/Sight, actual compiler/contexts00427730 and world
receivers. Other required actors/physics and containing-world/first frame remain.

## Actual input record loading owner now implemented

`human_input_records.h` now owns0073DDC0/0073DF70 records, their0073DA50 child,
base/angle value loading0073F810/0073FA80/00740410/007406F0, defaults0073F4B0
and registry0073F2B0/0073F5F0/0073F530. StartupSession retains the real process
frequency/quantizer initialization state; FrameClockPlatform exposes actual Win32
performance frequency/counter. First frequency is cached, counters are read for
each record. Each record owns one child id1, two genuinely empty pending lists
and32×three-word rings at native180/380. Runtime producers remain unexposed.

A concrete call-site discrepancy corrected the initial representation before
integration: native04/08/0C and10/14/18 hold **quantization-profile pointers**,
not coordinates or callbacks.0073FB10 passes them to the actual quantizer.
Parent record and history binding setters overwrite their own triples, including
nulls; leaf004C1360/004C1390 preserve existing nonnull profile bindings when a
new argument is null. Angle007405D0/007405E0 are genuine RET bodies and keep their
three fixed shared profiles. The later54/64 fields contain actual manager callback
borrowers. They are not invoked through an invented runtime implementation.

Angle initialization fixes original min/range/step words and uses0073D7E0's
floor(log(range/step)/log2)+1 bit count (10/5/4) and unsigned masks2^bits−1.
007E6368's CW173F/007EDB99 FRNDINT rounds down; quotient/log2 stores remain
double. Scales divide the unsigned mask by float range. The three global profiles
initialize once; value reset still writes shared interpolation step.1f. Fourth
vector lanes unwritten by the original are omitted from this representation.

Default application preserves interval→primary/secondary profile bindings→six-bit
field2→first callback getter→second callback getter order. Angle flags mask out
0003C000. `find` returns the first ID; explicit `make` preserves duplicate IDs.
Removal erases in order before destruction. Destruction retires children in reverse,
then second/first pending lists. Pending/interpolation/packet command producers
and actual manager construction/default callbacks remain required before World
integration; this owner does not provide network or simulation success.

Focused ASan/UBSan checks actual input/history/quantizer composition, fixed/null-
preserving profiles, flags/default order, shared clock initialization, duplicate ID
and ordered removal, with six required-endpoint failures. The B0 fixture now uses
the actual recovered input registry/history plus actual navigation source (frame,
clock and manager getters remain fixture endpoints), including four failure cases.
Strict i386 composes actual Human/frame/B0/input records and Win32 clock. Application
compile/link passes. No engine/Windows/desktop execution. A failed exploratory
export at0073F100 is unused; rely only on validated vtable-target exports and the
saved binding/quantizer assembly. No whole subsystem/function ledger credit.

## Initial look/animation and ground commands

Inline98 now supplies005ABF60's complete nine-mode command5 receiver through its
actual E0 look owner: flags0/86/92/6/12/C2/A2/85/91. Modes0..6 pass null target;
7/8 preserve the original target pointer. Other modes return without a call.
005AC070→007870D0 uses the actual movement-owned DD0+BF0 (AC0+130)=500 and
00757F60's CD0+D0 mode/authority/stance gate. No duplicate delay word is held by
DD0. The conditional00791370(9) is an equipment model getter, not an attachment
producer; its exact kind mapping now resides in HumanEquipmentState.

Focused native ASan/UBSan checks all nine look mappings/invalid no-call/cold
rejection,20 CD0 gate cases, actual getter mapping and movement scalar. Strict
actual host call-site instantiation checks these new DD0/inline/E0/geometry paths.

`human_state_ground.h` supplies0075C4F0's full ordinary reposition receiver.
Actual parent must be absent or kind5. It preserves E0 state2/3/5, falling,
life<=0 and animation159/158/155/111/34 gates. Exclude the SAME physical
colliders, attempt at most three sweeps with Y+(attempt+height), height1/.75/.5
for posture0/other/2, displacement(0,−5,0),radius.3 and masksC8102/230000.
First hit subtracts original.30000001192092896f fromY; commit only when above
cached actorY or vertical difference>.5. Write actual base30 and frame position,
update frame and snapshot actual posture/shot geometry in original order.

The original copies an unwritten fourth hit lane. The native guard preserves the
known actor fourth lane; it is not claimed as a recovered stack value. Actual
shared exclusions clear on no-hit, normal return and frame/sweep/snapshot failure.
Focused sanitizer checks three posture heights/attempts, threshold/fourth lane,
11 gates and four endpoint failures with the actual exclusion source owner.
Physical flag methods0075C770/005AC390 and suspend/resume0075C2D0/0075BC80 must
compose this receiver before derived B0 activation; new exports and assembly are
preserved under human-state-physical-flags/ and human-state-physical-ground.asm.
00781310 is a real face/morph/model transition with random/camera dependencies,
not a flag-only setter. Its conditional posture branch remains required.

Ordinary physical flag composition now supplies005AC390/0075C770 through the
same inline98-owned physical/E0/loaded sweep. Bit8 suspends actual primary/shot
membership and resumes marker/posture/shot geometry, radii, bucket moves and native
shot flags42/mask2(+10000 for dead life). It preserves requested posture0/1/2,
99C/9A0 transition choices, motion/radius gates and ordered descriptor count changes.
Dropping the last bit2/4 calls the actual ground receiver above. The rare00781310
face/morph/model transition explicitly rejects before its unrecovered effects;
no complete0075BC80/body or world readiness credit is claimed. NativeA9C is a typed
borrower whose runtime producer is still unexposed; loading resets it to null.

Focused sanitizer verification composes actual Physical/DynamicCollisionBuckets/
DynamicCollisionExclusions source: repeated suspend/resume, all three postures,
radii/shot masks/dead life, last-bit ground trigger, two endpoint failures and
retry cleanup. Actual i386 types additionally compile inline98→physical flags→
E0/sweep/buckets/ground/frame paths. Earlier unchanged audits were not repeated.

Next owned boundary:0056EE90's five AI subobjects and0056F200/0056F630 bind/close.
Matching0055D8B0/00560800/00563A70 are genuine RET destructors.0055E030 clears its
owned query buffers.0055D630 and0055D2E0 BOTH use the same0055D860 direction
owner at AI+124, not the separate005607D0 object at128; human-ai-bind.asm preserves
receivers. New human-ai-owned-lifetime/ exports succeeded. Direction arithmetic
and binding assembly are saved; inspect these before implementation. Actual
input callback bodies and derived B0/Sight/VM/World consumers remain required.

## AI-owned resources, callbacks and passive loading receivers

`human_ai_direction.h` supplies0055D860/0055D8B0 and0055D630/0055D2E0.
Both setters use AI+124. Current setter normalizes only current0 and may copy
raw target10; target reset copies raw current0 and normalizes only target10.
The two-bit flag aborts write nothing. Horizontal X/Z normalization keeps the
extended sqrt; full normalization stores the square to float before comparison
and sqrt reload. Fourth lanes/tiny-axis fallback are preserved. Nonfinite or
overflowing inputs explicitly reject. These are loading setters, not steering.

`human_ai_resources.h` owns0056EE90's124/128/134/12C/130 allocations and
0056F0E0's124/128/134/130/12C teardown order. Embedded00576AC0 is at C0;
128 uses005607D0/00560810,134 uses00563A60,12C has the inline query defaults,
and130 uses0055DF90/0055E130/0055F2C0/0055E030 with the SAME second mission
dataset.0055D8B0/00560800/00563A70 are RET. Path buffers are empty owners;
their runtime formats/producers are not inferred. Query44 and physical990 are
actual actor identity borrowers, outside scalar DWORD storage. Required allocation
failure throws and rolls back instead of leaving native null pointers for later
unchecked use. No whole AI function credit is claimed.

`human_input_callbacks.h` supplies actual0053B760 factory/sentinel,0053B970
read and0053B990 write callbacks. The latter two transfer B0+54 through owner5C;
this is not a history coordinate. Factory reads two bits for kinds0/1/2;3 returns
null, while its write branch emits3 in two bits and returns null. Common message
construction0073F770 and the three inline constructor stores are preserved;
derived value serialization is deliberately unexposed.0073D000/0073CF50 cursor
operations use the original low-bit-first DWORD masks, including deliberately
unmasked writes. Buffer capacity is a mandatory actual storage receiver before
writes; no network transport or packet producer is invented.

`HumanAIStateLoading` composes the ordinary0056F200 bind with the actual B0,
input history, navigation and inline98 owners. Preserve kind4/8 clear4/add2,
kind3 clear6/add1, other kinds clear6/add4; repeat the navigation query after
physical changes, then set current/target on the same direction owner. Interaction
flag100000 uses005AC2E0's animation106..108/8/14 gate and0051E6D0's reverse
actor references, storing a query only for nonnull actor with exactly two matches.
The original0053B9A0 decompiler loses stack stores: saved bind assembly confirms
its history seed, like AI and remote state, copies all FOUR Human30 words.
0073F860 now supplies that real history seed and00982F94 relative-position flag;
relative mode clears only XYZ at40 and preserves its fourth word.

Native input64/60 and54/58/5C receive the actual callback object/owner pair,
38=7,40=false and3C=0 only outside authority mode. Cold authority bind still
rejects until the actual initial command20/transport branch is present. Editor
mode rejects. Runtime action/job/command producers remain unexposed, so only
0056F630's proven empty loading close is supplied. Destruction clears borrowers
only if they still name that state; a replacement bound before old destruction
keeps its pointers. This is a defined lifetime guard beyond native dangling use.

`human_passive_states_loading.h` adds actual state6 inline construction and
005A42F0/005A4310, plus state7 construction/00592140/00592200. State6 owns AI
resources and clears Human44 on close without removing shared input. State7
owns only two256-byte arrays, disables physical18C, binds/seeds real input and
preserves38/3C/40. Its explicit close removes input through00583690. Do not
replace those different contracts with uniform state teardown. Switching away
from state7 after binding a replacement can retire a shared record: retain this
native lifetime risk when composing the containing switch, rather than silently
assuming transfer semantics. No simulation/update or whole state-family claim.

Focused ASan/UBSan passes: shared direction arithmetic/99 length cases; five
resource allocations/teardown and five allocation failures;1056 independent bit
field/cursor cases and callback factory/value paths; actual AI/B0/input/navigation
composition with physical endpoints as fixtures, all six generic kind branches,
reverse references, history seed/relative fourth lane, replacement retirement,
two physical failures, guards and state6/state7 differing close contracts.
Strict i386 uses actual Human/frame/Win32 clock/input/B0/AI/passive types and the
actual physical interaction getter. Inline physical calls in this new fixture
remain declared endpoints; their underlying composition passed the earlier
physical-focused checks. Application compile/link passed before the passive
header addition; final affected build follows when the current sources stabilize.
No original/Windows/desktop execution or new ledger credit.

The next manager boundary is now traced through0073EC50/0073ED80,
00723000/00723220,00741140 and0072D1C0/0072D1F0. ManagerE4/E0 are initially
null and the session installs actual ID-dependent getters. A concrete signature
mismatch was fixed:0073F4F9..515 loads the record's1C ID into ECX before EACH
getter. Defaults now take that ID. Getter selection uses Human callbacks for
ID<=31 or absent/type3 Scene2 actor, Car callbacks for type4, and null for other
actual actor families. Car callbacks/default receiver dispatch still need actual
composition; no zero-argument fallback is installed.

`human_input_session_profiles.h` supplies00723000's actual three owned quantizer
profiles: minimum−1000/range3000/step.01; minimum−.3/range.6 with per-axis
steps.005/.007/.005; minimum−7.5/range15/step.1. Bind primary and secondary in
native20/120/A0 order: large/small/medium, the SAME order as construction. Defaults use flags3B,
interval.035 and shared relative-position flag mode&1. The generalized original
0073D7E0 arithmetic now supports those nonuniform steps; angle profile order is
independently confirmed by actual ECX globals in saved assembly, unchanged.
Profile owners must outlive every default/history borrower. Manager reset's.08
interval is overwritten by session configuration; it is not the final default.
This supplies profile/default data only, not0073EC50's containing/embedded-record
lifetime or a reconstructed network session.

Focused sanitizer checks original constants,19/7/8 bit counts, pointer order,
flags/interval, both callback getter IDs and their order across five signed IDs,
two getter failures and original fixed angle profiles. Actual host i386 composes
these new profile bindings. Stable application compile/link passes with the new
AI/input source headers; no world integration or desktop execution is claimed.

Next player boundary:0053B9A0/0053BC50 requires actual camera00465EA0 and
00465B40(selector3), initial mode0 through005469E0→005A0890→00491FD0/00492130,
and005A91A0's Human2CC dependent lifetime. The actual first-person-view/weapon,
cursor/inventory/audio/HUD consumers must be recovered, not successful callbacks.
Exports human-player-state-dependencies/, human-player-mode-endpoints/,
human-player-camera-bind/ and human-player-presentation/ succeeded read-only.
Earlier0053B9A0 seed decompiler output is corrected by human-player-state-bind.asm;
it is not an uninitialized vector. Full player state/Sight/VM/world remains open.

During containing-manager recovery, the saved00723000 assembly exposed an error
in the first session-profile binding implementation: its decompiler omits the
ECX receivers. Actual construction is20→120→A0, so swapping the second/third
profiles was wrong. The source now binds large/small/medium. The new manager
fixture checks actual profile ranges per slot against those original receivers;
it supersedes the previous profile-order assertion in the first AI receipt.

## Containing input loading manager

`human_input_loading_manager.h` now composes0073EC50/0073ED00 with the SAME
registered-record registry, an actual embedded30 record and00741140 limiter.
00741180 rewrites only its native vtable; it has no owning cleanup. Cold reset
preserves BC/D4 bytes, C0/C8/CC/D0=0,C4=1,D8=.08,DC=−2. Configuration supplies
the real session profiles/getters, assigns embedded player ID and applies flags3B
to the embedded history. Its profile bindings remain null: native0073F250/
0073F1F0 update only outer defaults and registered records, never embedded30.
The outer setter skips nulls; each record receives the original triple including
nulls; the leaf position value preserves its old profile for each null argument.
0073F660 interval changes all registered histories and future defaults, while the
embedded value keeps its original.035 interval.0053B9A0's authority branch now
has the same manager's C4=0/C8=1 receiver. Registered records retire last-first,
then limiter/embedded history, then registry vector allocation. Profile owners and
process clock state must outlive every borrower. Runtime packet producers remain
unexposed, so this is loading composition, not a network-session implementation.

The actual default getter lookup is **00475DD0 over Scene2 identity slots** at
0089B3A0; it is not004772F0's runtime reverse-ID search. GameSceneActors now
forwards that same registry lookup. Human callback selection for signed ID<32,
missing/type3 Scene2 actor and Car/type4/other cases is preserved in the new
getter assembly/exports. Full Car callback decoding remains Phase3 runtime work,
and its actual receiver representation still must be supplied before manager
default dispatch is installed. No successful Car callback body is fabricated.

Focused manager ASan/UBSan checks actual registry/embedded composition and original
profile ranges PER SLOT against ECX20/120/A0; outer/record/leaf null differences,
embedded profile/interval exclusions, signed callback IDs, authority flags, two
clock failures/getter failure and exact registered→embedded allocation teardown.
Strict i386 uses the actual Win32 clock/manager/Human/frame/B0/AI/passive types
(with physical command declarations as endpoints). Application compile/link
passes. This supersedes the first AI receipt's incorrect profile-order assertion;
unchanged earlier owners were not broadly audited. No desktop or ledger credit.

## Player loading receiver source

`human_player_state_loading.h` now supplies005A3F20's state1 scalar/inline
construction and ordinary0053B9A0/0053BC50 composition. Bind zeros all four
256-byte arrays at C4/1C4/2C4/3C4, preserves constructor4E4=3, command5/look0,
input64/60 +54/58/5C,38=7/40=false, native4D8=3500/548=12 and other original
byte/word stores. History uses the known FOUR-word Human30 seed confirmed in
assembly. Authority mode uses the SAME manager's C4=0/C8=1; other modes reset
record3C=0. Editor mode and previous kind3 camera handover reject until their
actual editor controls exist.

MissionCamera now supplies00465EA0's real direction store, gated by current
configuration flags bit8 and mode260!=10. Constructor264 points at settings row1;
loading activation does not replace that pointer. Runtime configuration-selection
producers remain unexposed. Preserve all four direction words and do not normalize
in this setter. The fixture uses the actual camera/settings parser with default
blocked and loaded allowed configuration rows.

Player bind still requires actual005469E0 first-person consumers and005A91A0
formation cleanup; close requires the actual160/type4/164 driver gate→004D9280.
These are mandatory Presentation receivers with NO application implementation or
successful default. The source does not publish a live player or claim their
bodies recovered. Close deliberately resets callback owners unconditionally,
keeps the shared record, sets38=0, calls actual mode0 and clears Human44. Unlike
AI destructor guards, native player close may overwrite a newly bound state's
callback owner: preserve this separate contract when composing005A3F20.

Focused ASan/UBSan checks player/B0/actual manager/history composition, fields,
ordinary/authority input flags, source call order, four required endpoint failures,
shared-input close/retired-record guard, and actual camera loaded/default gates.
Presentation bodies and physical/look commands remain explicit fixture endpoints.
Strict i386 uses actual Human/GameActorServices/Win32 clock/manager/B0/player and
MissionCamera types with declared required endpoints. Application compile/link
passes. No actual first-person/formation/car exit bodies, runtime player controls,
activated Human/world, Windows/desktop test or new function credit is claimed.

Next concrete dependency remains005469E0 initial mode0.00490740 is the EXISTING
OverlayOwner at Game+5C (actual first-person view); constructor model40/player38
are null and flags58/59/5A/3C zero. Native006989B0 activates this view004909D0
AFTER HUD and module actor activation. Thus cold player bind's00491FD0 does not
have a loaded first-person model yet; its real0045DAC0 player-model visibility
and00486510 module+BC notification still must be supplied.00461410 is an actual
pointer getter(this+1C), not a side-effect body. Existing owner constructor and
matching004908B0 evidence live under analysis/work/phase2-owner-20261103/;
do not broadly re-audit their unchanged row ownership.

Latest targeted first-person exports: human-first-person-loading/ contains actual
00486510/0045D1D0/004909D0.00486510 returns module singleton0089B770.
0045D1D0 uses the control owner's override58, else the selected2C index into
its08 list, not simply global player ID. Camera virtual40 is0046C370, already
exported under camera-secondary-owner/: it selects actual cameraFPV__/camera3RD__
frames and configuration/mode/sector state, and is a real required callee of
0045DAC0. It is not a target-reset no-op. Native cold first-person flag58=true
enters its first-person mode2 branch; mode/target/config consumers still need
full reconstruction.004909D0 calls actual00490C80/004922A0/004920B0 after
clock/reset state; those are next view-loading dependencies. These research
findings earn no completion/readiness credit.

Confirmed module singleton0089B770 is **GEffects**004860B0, already represented
only by stored EffectsEnvironment values; earlier generic control-module labels
do not mean input controller. Original vtable008101F0 gives54=00486DF0,
A0=00486C10,A4=00486BF0,BC=00485B70. First-person notification must call this
actual effects receiver. New targeted exports are under
human-first-person-control-module/ and human-first-person-effects-receivers/.

GEffects virtual+BC is now supplied by EffectsShadowModels::lensflare_visible
and actual GameActorFrames scene+1FC lookup:00485B70 finds `lensflare` through
that frame's+48(maskFFFF), then calls frame+30 with the supplied visibility only
when found.00491FD0 tail assembly passes **!first_person_enabled**; the decompiler
loses that argument. This new small receiver passes strict actual-host i386
compilation and the application build; no engine/desktop execution is claimed.
Existing model_opacity already supplies00486DF0; do not re-audit it unchanged.

Camera target research is pinned for continuation. Original literals are
`cameraFPV*`/`camera3RD*` with wildcard mask10020, not the Ghidra symbol suffix
underscores; see human-camera-target-names.json.005AC4C0 returns DD0+CE4,
the SAME CD0+14 sector already held by HumanAnimationState::sector().
005A9760(0) resets float Human94 only if changed and then calls00786650 when
HumanA0 exists. Both retarget0046C370 and clear00465610 call it for the old
target's model-owner100→actual type3 actor BEFORE releasing the target reference.
Clear restores the saved scene camera, releases240 camera, then retires/releases
248 target. Do not add a target owner without this matching teardown dependency.

0046C370 normalizes target world direction with **(Y²+Z²)+X²**. The saved x87
assembly stores the square using FST (not FSTP), so the unit comparison retains
the extended square, while sqrt reloads the stored float. On target change it
resets2A4/2A8, preserves direction's fourth lane, initializes position/previous
position from target position minus direction×selected distance, distance254,
25C=0 and26C=true; position fourth stack lane is unwritten and needs a defined
explicit guard, not invented native credit. Mode selection also replaces264's
configuration row pointer, so the current00465EA0 row1 assumption must be replaced
by that SAME selected row when target selection is implemented. Ordinary unattached
first-person chooses mode2; third-person chooses modes1/3/4 or5/6 from real sector
names/flags. Attached Car/weapon cases have additional real consumers.

## Camera amount retirement and ordinary target ownership

`human_camera_amount.h` now supplies005A97A0/005A9760 and00786650 through the
same inline98/DD0/230/equipment/effects source owners. Human94 caches the float
BEFORE dispatch; equal values make no call, including after a previous failed
receiver. Null A0 is the genuine store-only path. DD0 always applies the actual
Human model, then only when A30==1 queries00791370(9) and applies its nonnull
model (the original getter is called twice). GEffects virtual50 is **00486D30**,
which calls MODEL+54 with1−amount, without a mode gate or clamp. It is distinct
from00486DF0/virtual54→model+50 opacity and00486E60→model+A8. Actual host model
method+54 is provided. No camera-fade reset is replaced by a field-only setter.

Focused ASan/UBSan composes actual amount-cache/apply/equipment/effects source,
including equality/null/mode gates, exact model order/complement, two required
frame failures, native cached-failure behavior, NaN and unclamped values. Strict
i386 instantiates actual Human/inline/DD0/equipment/effects/frame paths; remaining
unrelated factory/audio services are declared interfaces. Application builds.
The first00486D30 export attempt reused an existing directory and failed; only
the successful fresh human-camera-retirement-effects/ retry supports its body.

MissionCamera now owns0046C370's ordinary **unattached Human loading** target
and its matching00465610 release. Use actual `cameraFPV*`/`camera3RD*` Frame+48
mask10020, with original fallback to that same Human model. Third-person sector
comes from the SAME CD0+14 when A0 exists (including a null sector); only absent
A0 uses the actual scene sector query. Flags `nointerier`/`NOINTERIER` and
`interierclose`/`INTERIERCLOSE` come from sector **F0 property text**, not E8 name.
Modes1/3/4 or5/6 select the actual configuration row; first-person/zoom selects2.
00465EA0 now uses that SAME selected row instead of assuming constructor row1.
Outdoor mode1 resets row1 distance to actual stored2AC. Attached Car/weapon
camera paths explicitly reject until their real receivers exist.

On target change, require the containing world's real model-owner/type3→amount0
retirement callback, retire/release old reference, retain new reference and copy
normalized world direction/position using saved x87 store order. The borrowed
sector and the callback's world/actor/inline lookup must outlive the camera. This
new API has no successful default mapper. Callbacks/frame methods remain fixture
endpoints in native verification; the actual world mapper is still uncomposed.
No current actor publication or successful world readiness is implied.

Preserve original same-target behavior: changing configuration without changing
the frame does not recompute its stored position or acquire another reference.
For unwritten fourth position stack words, retain the known loaded-camera lane;
this is an explicit defined guard, not original recovery credit. Clear restores
saved scene camera, releases240, invokes actual amount retirement and releases248,
even when retirement throws. Target reference cleanup and callback reset still
complete after an endpoint error. Runtime00469300 camera update remains required.

Focused ASan/UBSan uses actual camera/settings source: default/current-row direction
gates, first/third-person modes, property text, wildcard/missing-marker fallback,
normalization/fourth lane, same-target behavior, exact reference order, four failure
boundaries and attached guard. Strict actual-host i386 selection/clear and the
application build pass. No Windows/engine/desktop execution or new ledger credit.


## First-person animation table

`first_person_animation_table.h` supplies00490C80 and chunk/string helpers
00415C90/004476C0/00415D00 through actual Files resolution of
`tables\FpvAnims.sav`. Root12345 contains weapon tags100+index (500 slots),
each consuming exactly13 rows tagged2000..2012, then at most four sequential
variant slots. Variant tags are ignored; empty records still advance the slot.
A populated record starts child1000 with a terminated name, followed by a raw
DWORD weight in its parent. Preserve names verbatim, including `#`. Empty and
omitted entries and cached clip frames remain untouched; duplicate writes win
in source order. Fixed native loops ignore remaining row/weapon payload.

Validate and allocate updates before publishing. Bounds, truncation, missing
termination and strings over999 bytes reject; a64MiB file guard bounds input.
Missing files return without changes; opened handles close on success/failure.
These defined guards do not credit native unchecked malformed-input behavior.
No clip loading, playback or first-person activation is supplied by this reader.

Read-only base Others.dta input is116783 bytes, SHA256
baa65d47ffa8cf49f8e8b87c2b14333f81e3a546e89f86507308573971ead3ff.
It has255 weapons,3315 rows,13260 variant headers and546 populated names.
Runtime loose/Patch precedence remains authoritative: Patch.dta is ISD1 and the
existing extractor supports ISD0 only; this base census does not prove runtime
selection. Inputs/census and focused native ASan/UBSan evidence are retained in
first-person-animation-table-*. Native checks cover actual bytes, names/weights,
empty/cache preservation, duplicates/four-slot behavior, atomic malformed guards,
three file failures and missing/success cleanup. Strict i386 uses actual host
ChunkFiles/OverlayOwner; application compile/link passes. Four temporary audit
files were fingerprinted and removed. Receipt:
`work/mission-world-20261103/first-person-animation-table-verification-wip.json`.
No original/Windows/desktop execution, readiness or new ledger credit.


## First-person item and view model loading

`ItemPrimaryModelCache::first_person` supplies007DF510's separate native20
cache, using native24/serialized resources[0] plus actual0084E8E4 suffix verbatim.
Require resource result exactly zero; find literal `fpv_weapon` with mask1, then
for class1 find `magazine` with mask21 and retain its original frame at native64
BEFORE recursively cloning the weapon through existing007DC3A0. The magazine
is not the clone's child. Missing weapon still leaves the retained magazine.
Only a successful clone caches the weapon. Same selected catalogue identity is
required; absent keys return null and dotted names are never stripped.
007E0560 releases derived64 before007DF430 destroys20 and later30. Retry after
a failed clone replaces an existing magazine reference safely, avoiding native
slot-overwrite leakage; that defined failure guard receives no native credit.

`first_person_model_loading.h` supplies00490930 hands,00491130 weapon/model
replacement,004932A0 track reset and00490B50 teardown against the same supplied
OverlayOwner's flags/vector. It owns actual hands44 and cloned view-model40;
weapon48/magazine4C remain borrowed from the actual item cache. The containing
view/frames and item cache must outlive this receiver. No containing session
activation or live mapping of the old cold words40 representation is claimed.
Hands accepts any nonnegative loader result, names `FPV_HANDS`, links scene204
with flags0 and hides. Weapon replacement removes/reparents old weapon to scene1F8,
then removes children in reverse and hides weapon; likewise magazine, then frees
old view model. New view model copies actual hands, optionally links scene204
with flags1, adds weapon, links flags1/shows, adds children in reverse, then adds/
links/shows magazine. `flash` lookup uses mask1 and hides a found frame. Absent
flash sets4C360=-1; null weapon clears flash and sets the same time sentinel.
Native local position writes only XYZ when E0&80 is clear; preserve the fourth
lane, then actual frame1C receives forward(0,0,1),flags0. Update both borrowed
models, then set view visibility from58 and update it.

Track reset visits7..0: time0, remove clip, null animation/weight1/flags0; resets
vector end to begin and70=0, advances model20 by100, updates, then94=0. Full
teardown removes weapon children BEFORE weapon, unlinks it and magazine to null
(flags0), resets tracks, removes clip0 again, updates/releases model then hands,
and clears player/enable/loading state. This differs from replacement, which
reparents into the real world root. Cleanup continues releasing owned references
after receiver failures and reports the first exception. Defined cold flash and
cleared stale flash borrowers are safety guards, not recovery credit.

Focused ASan/UBSan passes actual catalogue/cache/tree/view-model source with
fixture engine endpoints: exact dotted suffix key, magazine retain/cache/retire,
four loader/missing cases, identity guard, mode/cold gates, positive hands result,
reverse children/eight tracks, replacement versus full teardown and14 cleanup
failures. An initial audit assertion mistakenly compared earlier temporary-model
release events; it was corrected to inspect the cache-clear interval. Strict
i386 composes actual GameActorServices/cache/OverlayOwner and all new frame calls;
application compile/link passes. Four temporary audits fingerprinted/removed by
first-person-model-verification-wip.json.004922A0 player/equipment binding,
00492610 material transfer, animation00492930/00492A90 and containing004909D0
activation remain required. No runtime world/desktop/readiness/ledger credit.


## First-person hands material transfer

`first_person_hands_material.h` supplies00492610. Mode&2 and actual player
presence gate the work. Find visual `a` through Human model+48(maskFFFF), then
visual1E0 mesh+04(LOD0). Iterate its58 groups in reverse; group+14(index) yields
material at+04. Resolve lazy material flags04000000 through material44, then
texture0C/name18 (null name becomes empty). Choose the first reverse match whose
name contains uppercase **ARM** or **ARS**, with case-sensitive007E5A40 substring
semantics. These are texture substrings, not invented lowercase skin/hands keys.

If selected, find the same `a`/LOD0 on view model40 and visit its groups in
reverse. Each ARM/ARS group receives that source texture through material1C;
repeat group lookup immediately before assignment as native code does. Engine
assignment owns any texture reference update. No material clone or independent
texture reference is added. Always update view40, including source-no-match.
Missing required models/visual/LOD/groups/material and overlarge counts reject
instead of dereferencing invalid engine data. The decompiler's piRam0001003f
receiver is corrupt; saved assembly confirms actual view40 receiver and all
lookup masks/order. Literal evidence: first-person-hands-material-names.json.

Focused ASan/UBSan covers reverse source selection, destination lookup repeats,
case-sensitive ARM/ARS matches, null names/no-match update, mode/null-player
gates, required-data guards and seven propagated endpoint failures. Strict i386
instantiates actual HumanSceneActor/GameActorServices; application compiles/links.
Native engine endpoints remain fixtures. Receipt first-person-material-verification-wip.json
fingerprints/removes four temporary audits. No containing player/view activation,
world/desktop/readiness or new ledger credit.


## First-person animation tracks, sound and camera receivers

`first_person_animation_playback.h` supplies00492A90 using SAME view rows and
model receiver's typed70/84 tracks. Require mode2/model; mandatory actual004933C0
notification comes before clip/flash work. For shots2..5 hide `ammo`(mask1) on
item38; optional `flash` gets time50/opacity1, scale RandFloat()*FLOAT.4+DOUBLE.7,
and rotation RandFloat()*FLOATpi only if normalized forward is exactly(0,0,1).
Preserve unwritten DC padding. Same shot ID restarts current slot at0 through
model9C/90, sets weight0/speed5 and updates, without another clip lookup.

Require some positive signed threshold with a nonempty name. Draw is truncation
of RandFloat()*100; choose first signed cumulative threshold >= draw. Do not sum
weights. Draw0 can select an earlier zero-weight/empty slot, resulting in the
native missing-clip path. Resolve exact names (including #) through the SAME
category3 cache0041FC10 and keep owning references in the SAME row.frames.
On an active prior track, append its four fields before weight0/speed5; otherwise
weight1/speed0. Choose first free slot0..2, or3 when all occupied. Preserve stable
fade vector order and set variant/animation/current cached clip. Explicit result
values distinguish gated/empty/no-variant/missing/restart/play outcomes. Invalid
indices/random bounds and unreasonable vector growth reject rather than indexing
past original storage. Actual source-only receiver remains separate from the cold
session words40; no runtime producer or live session view is published.

00491440 advances blend weights only. Exact signed elapsed FILD feeds current
weight; stored float elapsed feeds fading weights. Retain extended comparison
before each float store. Clamp current above1, with no invented lower clamp.
At an initially full weight, clear fading slots last-to-first and return WITHOUT
model update. Otherwise update current, subtract each old weight, stably erase
nonpositive lanes, retain unordered/NaN lanes, then update model. A newly full
current clears old tracks on the NEXT call, matching the original branch.
Focused native checks cover101 threshold draws, cache/empty/signed/draw0 cases,
slots0..3, flash/restart, three endpoint failures, clamp/delayed retirement,
stable erase/NaN and signed-negative elapsed. Model/cache/notification/engine
endpoints remain fixtures; actual i386 source composition compiles.

`FirstPersonAnimationSound` supplies004933C0's request through the SAME typed
view38 player identity, catalogue, camera query and mandatory SOUND+54 emitter.
Gate on either58/59 and IDs2..5 or8. Shots require actual definition kind9 and
payload1..4 virtual18=007E0290/native14/serialized10, group2. Reload uses actual
definition40/fields38[2], group3. Both set flags41/relative null and borrow player
as source; other request defaults come from004374D0. Query actual active camera0
for all four world-position words, ignoring false as native does (zero constructor
position stays). Invalid reload definitions/weapon payloads reject native null
reads. Actual four payload vtables and deserialization starts are saved. Native
checks all tags/gates/requests/defaults, alternate flag, absent camera and both
receiver failures. Strict i386 composes actual view/model/playback/catalogue/camera,
StartupSession SAME sound manager/catalogue/gate, GameSpatialSoundServices and
emit_spatial_sound; containing World actor/portal methods remain DECLARED endpoints.
No successful fallback is supplied for them.

MissionCamera supplies0046CB20 command7 reset294=0/290=param3-present,
00465E50 virtual50 FOV and00465E00 virtual5C parameter78, each applying the
actual frame receiver then updating only if owned240 exists.00465A10 virtual64
queries0(active scene camera)/1(owned240) world position through real matrix update,
2(stored30 direction)/3(stored10 position); absent/unknown query leaves output
untouched and returns false. No full command0..9 or camera00469300 simulation
claim. Native targeted checks cover cold/active/owned/state/output preservation,
four lanes/update order and three failures; actual host i386 compiles.

Research correction: initial first-person-track-advance export included two
unvalidated camera addresses and reported overlap/no-start errors. Only fresh
successful first-person-camera-track-receivers/ export is used. Loaded vtable is
0080FDD0;0080FD40 is its pure/base table. Constants receipt distinguishes the
DOUBLE.7 at00810498 from its meaningless low DWORD float interpretation.
Application builds pass; first-person-playback-camera-sound-verification-wip.json
fingerprints/removes12 temporary native/compile audit files. No original or
Windows execution, readiness/world/desktop or new ledger credit. Remaining
containing004922A0 player/equipment bind,004909D0 activation, actual first-person
mode/zoom/cursor and world actor/portal/retirement consumers still block loading.


## Containing first-person view binding, actions and activation source

`FirstPersonHumanItems` now uses actual Human storage:005A6DA0 at74[4] clothing
and005A2AC0 at74[3] hand holder.005A2B10 repeats holder reads and accepts only
native family0/weapon. Actual GameMissionItems validates same live entry ownership;
its RuntimeItem supplies definition/amount. RuntimeItemServices exposes the SAME
FPV definition cache and retained magazine rather than creating a second cache.
005AC400 is a Human inline98 getter of E0+50 mask1, NOT a definition/item flag;
HumanInlineLoading reads its SAME owned look state.005A80F0→00587530 reads B0+AD
and returns1 or2; HumanStateBaseLoading now supplies that exact getter.

`first_person_current_player` recovers0045D1D0 from actual control override58,
list08 and signed index2C. Override wins even with bad index; -1 returns null;
other out-of-bounds indices reject. The containing world must supply these real
borrowers, not global00982104 as an invented equivalent current-player getter.

`FirstPersonViewLoading` composes004922A0 bind,00492500 weapon refresh,
00492930 actions,00491640 state,00491FD0 enable,004920B0 reset,00492130 zoom,
004921F0 secondary view and0045DAC0 visibility with the existing model/table/
material/item/playback/camera/effects source owners. Players must supply actual
current/inline/B0/attachment/sector/retirement mappings and Cursor actual0049DC60;
no successful defaults. Native fixture endpoints and declared i386 interfaces
are named, and no complete containing world or session view publication is claimed.

Bind state0, notify old attached seat off, fetch current control player and store
SAME typed view38 identity; null returns. Notify new attached seat with58, reset
zoom, select hands from clothing resources[0]: absent or substring `w_default`
uses FPV_hands, otherwise strip last extension. Load actual hands, get actual
weapon, evaluate SAME E0 suppression before track reset; no/suppressed weapon
binds null/-1. Otherwise get FPV model BEFORE reading magazine64, bind, set ammo38
visibility from actual amount>0 and request idle0 (mapped to1 at exact amount0).
Apply Car-driver/secondary/first-person visibility, actual00492610 material
transfer and model update, then B0-derived state1/2 and bob-.3/0. Refresh00492500
intentionally omits the E0 suppression test, material copy and idle request.

Actions preserve original state3/4/5 gates, reload8 refresh,9 zoom-off,20 reset/
unbind/animation-1/state0 early return,11/12 animation exclusion for7/2/3,
21→state3 outside zoom, zoom remap2→4/3→5 and suppressed0/7/21. Argument98
stores after playback, even when its clip is absent. Reset zoom always clears5A,
camera command7,false, copies live gameBC FOV, sets parameter34 from3F91361E,
applies both actual camera receivers and cursor0. Zoom setter returns on equality;
otherwise sets5A, command7 and FOV30, plays11/12, sets34 from3F32B8C3/default,
and invokes actual cursor mode for controlled player mode2. No immediate camera
FOV/78 application is invented in the setter. Secondary59 directly changes FOV,
hides view model when enabling, refreshes current-player visibility/cursor. On
secondary disable, the repeated58 enable call retains its native equality return.

Enable00491FD0 has NO3C loading gate. Mode2/equality and ordinary gameB8==2
can return. Cache58 before actual attached/visibility calls; camera target uses
actual current control player (which may differ from view38). Camera early guard
avoids sector/retirement lookups when camera absent/frozen. Then unlink/reset zoom
or link scene204, set view visibility/update, and actual GEffects BC receives!58.
Attached camera remains rejected by MissionCamera until its real path exists.
Attachment callbacks remain mandatory; they are not successful placeholder services.

004909D0 activation is now supplied on this source receiver. Require mode2 and
cold models/3C, set3C/state0, call actual performance frequency then counter and
DISCARD their results exactly as original stack reuse does. There is no native
persistent timer owner here. Clear borrowers/current/fading/scalars, zero position
XYZ and make actual axis(0,0,1)/angle0 quaternion through the engine import.
Position00 fourth stack word is unwritten: preserve known lane as a defined guard.
Load SAME FPV table, zero offset20 XYZ and copy its fourth lane from the quaternion's
fourth output (that same stack slot is now written). Set6C=1,9C=0/flash sentinel,
bind player and reset zoom AGAIN. Duplicate live activation rejects instead of
native stale-borrower misuse; exceptions release models/reset loading state and
preserve the primary failure. Table missing remains the genuine diagnostic-return
path; malformed input rejects with no successful activation.006989B0 ordering,
actual world/controller publication,00493360 reset wrapper and first render remain.

One combined focused ASan/UBSan check exercises actual view/model/table/holder/
item bridge source: current override/list rules, suppressed/no-player paths, hands
keys/cache call order/empty idle/ammo38/state/bob, mode/phase/equality gates,
8/9/20/21/remaps, zoom/secondary/current visibility/cursor/effects order, three
endpoint failures, discarded clock calls/two failures, quaternion reused lane,
malformed-table cleanup/retry/missing file and duplicate guard. Frame/player/
camera/clip/cursor/effects endpoints are fixtures. Strict i386 instantiates actual
Human/holders/GameMissionItems/RuntimeItemServices/model/table/animation-cache/
playback/sound request/MissionCamera/GEffects calls. Inline E0 getter uses actual
HumanInlineLoading/Look source but declares its unrelated DD0/Equipment/Factory;
Players current world mappings, Cursor and SOUND emission remain declared endpoints.
Actual ChunkFiles/FrameClockPlatform activation call compiles. Application builds.
Receipt first-person-view-verification-wip.json fingerprints/removes four temporary
audits. Full P2.2/loading readiness/desktop/ledger remain unchanged.


2026-11-08: human_player_roster.h now owns borrowed HumanRoster list08 and
HumanSelectionRecords control38. Base00461AA0 rejects NULL/duplicates;
00461C70 swap-pops and reports removed index only on success.004619F0 invalid
unsigned index returns NULL.0045D160 index28 (outside mode7 requires005A63F0)
is DISTINCT from0045D1D0 override58/index2C and0045D1B0 index30. Selected getter
reloads index/list after the actual suppression callback. Original selected
records are16bytes, Human00/refcount04/enabledBYTE08/timer0C.0045EB30 fills
caller list in reverse using refcount!=0, even when enabledBYTE is false.
0045E410 acquire notifies HUD first in mode2, increments existing refcount/reset
timer, or inserts a new owned record at FRONT, preserving native changed-byte
quirks and dead-record partition swap0045E3B0.0045E540 release invokes HUD first,
health/005A6380 gates, decrements positive references; zero sets10000ms timer and
optional hide200.0045E2F0/0045E2C0 ticks timers in reverse, zero means immortal,
wraps DWORD subtract before signed<=0 expiry, ordered erasure and native counter
updates. Reverse reset retains list allocation/changed-byte, resets counters and
notification flag. Optional HUD/actual Human visibility/state consumers remain
MANDATORY feedback declarations, not successful stubs. Focused native sanitizer
and actual HumanSceneActor i386 compile/app link pass; players-roster-verification-
wip.json fingerprints/removes four fixtures. Full player-control/module ownership,
actual frame collection and activation remain required; no new ledger credit.
Successful players-control-owner/,players-control-roster-receivers/ and players-
control-selection-completion/ preserve originals. Ignore FAILED players-control-
selection/ (interior0045F6D0); actual thunk is0045E3A0. Acquisition/release original
assembly saved as players-selection-reference-counts.asm.

**Execution-mode correction requiring implementation:** actual StartupSession
application_flags() is the SAME initialized gate as native00982100.0072E1A0
sets7 before device initialization (already integrated control_startup.h); normal
single-player kind7 reaches006989B0 without a network mode rewrite. Script
activation004270D0 selects Scripts.dta in mode7 and mpscripts.dta outside it;
this agrees with offline single-player, NOT an editor-only mode. Earlier Human
helper labels/guards calling global mode7 "editor" were incorrect classifications.
Their verified non7 bodies remain source evidence, but missing mode7 branches ARE
required P2.2 Tutorial work. Do not remove guards without reconstructing those
branches. Targeted mode7 closure is now the priority: B0 base/input-null, AI/player
bind/close, geometry62B0 face owner, equipment action lifetime and containing
Human/world activation. No broad audit of unchanged non7 paths is needed.


Offline state correction now verified for005832F0/00583690,0056F200 and
0053B9A0/0053BC50. Base performs the same item/model/navigation/direction work,
then mode7 sets input7C NULL; it never looks up or creates network input. AI mode7
still initializes physical flags/navigation/directions, then leaves7C NULL with
NO callbacks/history/authority command20 packet. Player mode7 likewise skips the
entire input/history block but performs actual camera/look/reset/mode0/formation
consumers; state3 handover still requires its separate real receivers. Player
close preserves Car exit first, then mode7 REQUIRED004563E0/0089AC30 virtual34,
mode0 and clear Human44. Non7 callback-reset/input behavior stays distinct.
005A4240 player virtual64 now reads SAME B0 byte70 for selected-player queries.

Base offline kinds4/8 with positive actual EC stat require SAME game special-music
reference0069E920 and set B5 only after that call. Base close checks B5 in ALL
modes and releases exactly once; the actual derived virtual20 decides whether
base close is called. Do not introduce base-close calls in AI/player destructors.
The music owner must outlive those callbacks. game_music_references.h preserves
Game C8/CC/D0/D4 (caller supplies ACTUAL two track IDs), nested DWORD references,
false decrement/only-zero transition, mode2 presentation gate, mode7 C8/CC versus
other modes -1, current-request equality,4000ms fade/position0 and D0=5000.
This is music, NOT a pause/control94 flag. StartupSession bridges SAME existing
primary movie-selection/queue state.006989B0's later explicit D0/D4 reset remains
part of containing activation. GameApplicationState.mode_flags reads SAME startup
gate; it does not fabricate a new game mode or publish loading readiness.

Focused native ASan/UBSan uses actual UNCONFIGURED input manager/clock to prove
mode7 cannot accidentally touch input records, required music-hook failure, real
music owner nested references/wrapping/gates, player callback order/selection70 and
AI directions. Actual i386 uses real HumanSceneActor/GameSceneActors/input manager/
FrameClockPlatform/MissionCamera/StartupSession music. Inline and Presentation
(mode0/formation/Car exit/0089AC30 virtual34) remain REQUIRED declarations.
Application link passes. human-offline-state-verification-wip.json removes four
fixtures; human-offline-mode-map.json and saved original asm/exports preserve
evidence. Source-only, no full derived-function/ledger/desktop credit. Geometry
mode7 face and equipment action lifetime guards now correctly say OFFLINE;
those required branches still need source reconstruction before readiness.


2026-11-08 follow-up: required offline geometry62B0 and equipment retirement now
have actual source, with focused sanitizer/strict i386/app checks. See latest
[face/shared resources](human-model-geometry.md#offline-face-geometry-and-shared-resources--2026-11-08)
and [equipment lifetime](human-control-loading.md#offline-equipment-action-lifetime--2026-11-08)
contracts. Earlier statements that those guards still reject are superseded.
Actual world must initialize/provide the SAME face owner and real profile/mission
settings; full Human/B0/Players/VM/world activation remains uncomposed. Readiness
is still false, with no ledger or desktop claim.


2026-11-09: [Sight/perception registration](sight-loading.md) now has actual source,
SAME Scene2 actor-frame registry and typed Human268/278/288/298 borrower lists.
Registration/markers/cross-links/relationship resets, offline friendship/roster and
005A5250 notification routing pass focused native sanitizer, actual i386/app.
Derived B0 callbacks, alignment005A84F0, Players Dog consumer and Dog feedback
remain required world declarations. Source-only; no active world/ledger credit.
Important next path: offline Human19C1 initially binds B0kind3 via005A3E60(false),
so actual kind3 loading and subsequent player handover cannot be skipped. Sight
AI callbacks use the real shared event pool/queue, now being investigated.


## Script AI state5 loading and Sight callbacks — 2026-11-09

`human_script_ai_state_loading.h` supplies actual005A3F20 case5,0058D810/
0058D8E0 and00590060/0058FFE0 on SAME recovered B0/AI/event resources. Native
constructor005A410C sets inline198 through00591EE0: byte0 and four zero words;
no additional owned buffer.00591ED0 sets helper+4=80 only if helper byte0 is true.
Derived deleting thunk005A42B0→005A42D0 only restores vtable and closes AI;
there is no invented helper destructor or automatic Base.close.

Binding calls actual AI bind, initializes1D8/1DC=-1, helper reset,1B0=0, then
borrows actual modelEC data1 script context into typed1AC. State1B4/1B8/1C0/
1C4/1C8/1CC/1E0 become0;1D4=-1; byte writes preserve unrelated lanes at150/
18C/1BC/1BD/1D0/1D1/1D2. B0 flags50 clears80000000. Native return is whether
context exists;005A3F20 ignores it. The source retains that distinction and does
not manufacture an empty successful context. Context resolution and0041DA20
interrupt are required Contexts methods over actual shared owners.

Acquired00590060 always first appends common00574070 event; dead0058FFE0 always
first appends005741F0. Then B0 flags50 sign and state1B8 masks20/200 gate the
script interrupt. Passing sets1B4 event mask and B0 sign bit, copies actual actor
world position to140..14C and interrupts only a present context. Repeated/gated
callbacks still append the original events. State5 inherits00574130 loss.
Close calls actual AI close then clears Human44, retaining its context borrower
until state destruction, exactly matching0058D8E0.

Focused ASan/UBSan composes actual B0/AI/resource/input/event source with native
frame/Inline/context fixtures: real unconfigured input manager stays untouched
in offline7, scalar/byte/sentinel state, present/missing context return, helper
reset, event-before-interrupt, sign/mask gates and repeated close. Strict i386
uses actual host Human/Frames/inputs, mandatory context/Inline declarations;
app compile/link passes. human-script-ai-verification-wip.json removes four
artifacts. Full VM/context/compiler/program integration remains required. No new
ledger or desktop credit. Successful human-derived-offline-loading/ preserves
actual3/4/5/8 bind/close originals; human-derived-state-factory.asm proves198
helper placement and005A42D0 ownership.

## Guard AI state4 and navigation routes — 2026-11-09

`human_guard_ai_state_loading.h` supplies005478C0/00547CF0 and timer helpers
00547AD0/AF0/BF0 over the same B0/AI/command/navigation owners. Default2104 flags
choose constant timers100000/10000. Radius modes preserve actual CRT random
factor0080FF64, signed stat9 thresholds, cached nearest-volume point190, full
primary route, marked secondary route and reverse-primary kind11 door borrowers.
Close clears those buffers after AI close, retaining Human44 like the original.
ModelEC script context/status and005518F0 handover replay remain required methods.
Binding with previous state drains a nonempty first event queue into the second
with TTL10000; an empty first queue instead requires actual005518F0, including
Human2AC records. No invented empty replay is provided.

`MissionNavigation` now supplies00520830/00520860/00520530 and005208E0/00520910:
FIFO first-discovery traversal, not distance relaxation; four-byte WORD target/
WORD cost links scaled by1/256; extended-precision radius comparison before float
distance storage. Origin is included for positive radius and NaN, zero appends
nothing successfully, negative radius fails. Marking00524B10(7) tests bytes17/18/19.
0051E520 resolves the second sorted WORD reference family, testing last boundary
before first, including duplicates. Existing nearest queries were not re-audited.

Event005ADC50 pops oldest first. Its apparent caller argument1 actually belongs
to subsequent005ADEE0's priority flag. Merge deduplicates reverse by nonnegative
first ID, else nonnegative second ID; neither means append. Replacement keeps
signed maximum TTL, returns the prior lease to the same pool, and only the
second-ID path optionally preserves the entire old kind word when its high
nibble priority exceeds the new one. Focused ASan/UBSan, strict actual i386 and
application build pass. Receipt human-guard-ai-verification-wip.json fingerprints
logs; temporary files were already absent after interruption before they could
be fingerprinted. Active3/player/world integration remains; no ledger/desktop
credit and loading readiness remains false.

## Active AI state3, action ownership and player handover — 2026-11-09

`human_active_ai_state_loading.h` now supplies00524C60/00524E70/00525110's
loading control flow over SAME B0/AI resources, with real kind1 action ownership.
`human_ai_action_pool.h` is the kind1 family ONLY of process008AB048 (manager+60):
00582030 initializes16 records;0058A580 grows by mutable0C;005827B0 writes full
position/direction, radius15,44=-1 and zero-extended WORD pointFFFF.00575820 and
00582C50 case1 return it to that SAME LIFO pool, preserving other words on reuse.
StartupSession retains that one process pool. The other eleven families are not
claimed recovered. AI currentF0 is now a typed lease, never a truncated pointer.

Initial offline Human19C1 binds active3, not player1. Binding preserves derived
scalar/byte writes, positive-health action mode12, actual current/target direction
views0056FBF0/0056FC00, follow=false modes005377F0/00538510, target reset005607B0,
action modes005380C0/00538640/00538550, previous-player/script handover and reverse
enemy replay. Equipment00530D40/005A67C0 and populated-stance00537FB0 remain
REQUIRED Services methods, not successful substitutes. Dead state with no native
initialized action mode rejects instead of crediting zeroed guard bytes. Source
reset0052EF20 sets serial1BC=game2C+1, query50|2000, path40|4 and bytes202/206;
runtime task/formation/vehicle-path producers remain unavailable.

Active acquired0052E5B0 appends common event BEFORE Players lookup/Speech, uses
actual cached actor30 distance against008101E0=400, codes5C/5E and reverse-source
Players0045E410; loss0052E6F0 appends then removes and optionally queues60. Dead
0052E6D0 only queries Players, with NO invented common dead event. Players/Speech
are mandatory actual shared-world endpoints. Native vtable addresses and exports
are preserved in human-active-ai-action-vtables.json and successful checked dirs.
Ignore failed human-active-ai-action-lifetime/ and human-active-ai-reset/ exports;
fresh *-checked/ and direct assembly replace those failed attempts.

Player0053B9A0 now accepts previous3: actual camera virtual68 query3, copies10 to20,
then actual00524E00/005385D0/005385F0 handover flags/posture/action getters. Reverse
player1→active3 uses528/548 and old virtual4C target. Source accessor polymorphism
supports these typed handovers; it does not claim native object/vtable layout.
The required mode0/formation/Car/0089AC30 presentation receivers remain separate.

Concrete camera discrepancy corrected: native10/20/30/40 are position/direction/
saved-position/saved-direction. Existing00465A10 query2 incorrectly returned
direction; it now returns saved-position30. Added actual00465B40 queries0/1 for
normalized world forward (Y²+Z²)+X², stored float square before sqrt, fourth lane
untouched, tiny fallback and native unordered behavior; query2=saved-direction40,
query3=direction20.00465EA0 writes SAME direction20. Earlier camera-query notes
labelling30 as direction are superseded by the original assembly and this fix.

Focused native ASan/UBSan verifies action allocation/growth/reuse/lifetime, event
ordering/distance threshold, bidirectional handover, serial wrap and camera query
failure/output preservation/NaN/tiny vectors. Strict i386 instantiates actual
GameSceneActors/Human/InputManager/StartupSession/MissionCamera with REQUIRED
equipment/Players/presentation declarations; application compile/link passes.
human-active-ai-verification-wip.json fingerprints/removes four fixtures. No
actual world, firstframe, desktop or new ledger credit. P2.2 remains open.

## Shared state publication, area8 and live Sight dispatch — 2026-11-09

`human_loading_state_factory.h` constructs the actual recovered state1/3/4/5/6/
7/8 types with SAME inputs, tables, navigation, action pool, inline control,
camera and world dependencies. `human_state_loading_owner.h` preserves005A3F20:
same-kind return, publish NEW before its bind (therefore005A80D0 sees NEW), keep
OLD alive throughout bind, then old+20 and delete. Failed binding keeps both
owners for explicit retirement and reports no bound readiness. Unsupported native
case/null allocation rejects instead of executing the original null dereference.
005A3E60 offline false→3/controlfalse, true→old3 reset→1→Players0045DA60→controltrue;
non7 uses actual local ID and authority bit.0059F2C0 initial selection preserves
script override and native short-circuit remote-AI predicate. This is the state
selection portion, not the whole containing Human activation.

Actual HumanSceneActor now has one typed borrowed `loading_state` B0 view;
factory publication writes it directly. `GameSightLoading` resolves this SAME
borrower and dispatches its actual acquired/lost/dead virtual source. The former
World B0 getter and derived-callback declarations are no longer required.
View polymorphism is a source interface, not native layout. Player005421C0/
005422B0 now preserve offline7 gates, known/distance400/code5C/5E, speech-before-add
and remove-before-known/code60 ordering with5000ms. They append NO AI events.
Actual Players/00542060 speech remain required Services methods. Player dead
genuinely inherits00583190 RET4; AI3/4/5/6/8 dispatch their distinct source bodies.

`human_area_ai_state_loading.h` supplies state8's00559A50/00559C20. Fresh bind
copies actual position, flags&7 and zeros154/15C/F0. Only a previous-state bind
uses actual HumanB4 area XYZ/radius producer, navigation FIFO, reverse kind10
WORD filtering, hand-item fallback and modelEC context/status. Skill tests are
independent strict >90/>75 on ORIGINAL stats128 (stats_11c[3]), unlike guard4's
current signed statsF8 thresholds. Close clears primary160 route but retains
filtered170 buffer, exactly as assembly. No invented command-area origin is used.

Focused native ASan/UBSan composes the concrete actual derived state factory,
same-Human publication/selection/handover, same-kind suppression, failed equipment
bind cleanup, area8 route filtering and player Sight ordering. Native engine/
equipment/Players/context/Presentation endpoints remain fixtures. Strict i386
uses real GameSceneActors/Human/InputManager/StartupSession/MissionCamera and
GameSightLoading direct B0 dispatch, with actual unresolved consumers declared.
Application build passes. human-state-owner-verification-wip.json removes four
fixtures. Human98/A0/A8 and application world remain cold; source B0 publication
is available but not invoked by GameApplicationState. No new ledger/desktop
credit; full containing0059F2C0/0059FA60 and world/VM/Players remain required.

## Containing Human stats, shared profile and eye materials — 2026-11-09

`human_loading_stats.h` supplies005A8990 difficulty scaling: signed FILD, exact
float factors .7/.9 for difficulty1 and .85/.95 for2, indices0/3..8 only; mode4
sets indices2..8 to100. Current EC stats are copied from original11C later by
containing activation. Original006B6700 first tests006B66B0: remote configuration
26C when true, offline profile34 otherwise, non7 fallback3. The helper receives
that actual difficulty explicitly. Do not apply a menu-preset replacement.

007E5E00 truncates to signed64 and callers keep EAX, so conversion retains LOW
DWORD, not a signed32 clamp. Invalid/out-of-range x87 integer-indefinite has low
DWORD0.0059EA90 looks up SAME shared HumanStore only for nonnegative index;
negative retains existing B8; missing nonnegative clears it. First nine current
stats come from native record48..68 floats, remaining three stay, then all twelve
copy to11C. Actual HumanSceneActor now carries typed `profile_record` B8 borrower.
0059FA60 updates ONLY low byte of record70 for offline kind1/4 mortality, then
clears B8. Source record tail padding stays unchanged.

Important mapping correction:005A8790 reads record1C, the EYE filename, NOT a
display name. Native user-name overlay is14, face18, eye1C, confirmed by original
006A0AA0/006A1D80 literals.005A87C0 copies it into HumanBC and invokes actual
GEffects+AC=0048A5C0 (vtable008101F0), now in EffectsShadowModels::human_eye_texture.
Earlier handoff display-name labels are superseded. human-profile-binding-map.json
and original profile/eye assembly preserve evidence.

Eye replacement lowercases, requires substring e_f0, strips LAST extension and
loads .bmp via actual Graphics+3C with empty secondary string/flags400.0048A3F0
clears its group list at every recursive node, scans all LODs ascending and groups
reverse only for kind1/flag800/SNMG or SMRR, lazily resolves material textures and
matches lowercase texture-name substring. First visual with matches wins; otherwise
children reverse until success. Prepare FIRST group+18, bind its resulting
material+1C texture, retain material DWORD4, share it via later groups+0, release
material temporary BEFORE texture. Actual host frame/LOD/group/material/Graphics
endpoints supplied. Missing model/pattern/groups/texture returns false as original;
containing profile bind ignores that optional result. Bounds reject native64-byte
filename/256-byte material-name stack overflows. No fake eye textures are created.

`ActorStorageOwner::clear_human_entries` implements005A29C0 order0/1/2/3/4/6/5:
actual equipped pack slots6 clear before holder5 releases that pack. It preserves
storage owners and metadata. Focused native ASan/UBSan covers original factors,
low-DWORD/NaN conversion, profile borrower rules/padding, eye traversal/group
sharing/release and a pack destroyed when its holder releases it. Strict actual
i386 Human/StartupSession HumanStore/Effects/GameSceneActors and application build
pass. human-stats-eye-verification-wip.json removes four fixtures. Full containing
Human activation, sound listeners/notifications, formations and world remain;
no new ledger, firstframe or desktop credit.

## Containing Human loading source — 2026-11-09

`human_loading_activation.h` now composes0059F2C0/0059FA60 around SAME Human,
inline98, concrete B0 factory/state owner, StartupSession HumanStore/perception
pool, shared speech cache, eye effects and Sight/SOUND. HumanSceneActor exposes
a typed loading-lifetime borrower; Scene2 dispatches type3 through that actual
owner and rejects if absent. It is not installed into a production world yet.

Common ActorBase activation runs on both passes. The first local initialization
clears BC, reparents a nonsector parent using worldY+.5/scene+E0, resets94 and
known scalars, applies difficulty ONLY to kind2 BEFORE11C→EC, binds profile for
4/5, resolves260 z→f BEFORE inline98, then selects actual initial B0 (modelEC
script overrides).005A6A90 repeats00584830 refresh AFTER state bind. Speech114
is current stats_ec[10], read as signed table index; non1 acquires that shared
definition then positional0 fallback. W lanes copied from uninitialized native
stack are retained guards, without false constructor-value credit. Context2C4
tail reset preserves both list contents/capacity; actual20/40 arrays are typed.

The false pass resets colour1E0 XYZ/200 and has actual required non7 dead-packet
receiver. True resets16C, adds Sight observer15 then target15/enables it, SOUND
listener and006DFCB0 registration, preserving offline7/remote short circuits.
No registered observer can substitute for required World/Players/VM behavior.

0059FA60 removes detached-camera Human reference first, updates profile70 BYTE
BEFORE Players removal, then clears B8/90. B0 close/delete precedes inline98;
command queues, four actor lists, reverse2AC pool retirement, stats restoration
and inventory0/1/2/3/4/6/5 follow. Attachment160 is now an actual typed actor
borrower with signed164; no pointer in a DWORD. Detach+80 precedes formation
detach/destruction. True unregisters Sight observer/target, SOUND and006DFE50,
even when false already cleared90. Base deactivation precedes speech release.
0059EF80 only ensures the shared cache;005B0390 releases. Non4/5 profile index
resets only on true. Partial initialization remains retireable; retry before
retirement rejects, and cleanup collects failures while continuing later owners.
Nonempty raw Human190 still requires its actual typed kind1/4 task producer.

`sound_listeners.h` implements00433ED0/00433E80 on StartupSession's SAME process
SoundRuntimeState: bit1 gates both; reverse search uses null CALLBACK; nonHuman
actor field is null; removal zeroes all three borrowers and retains slot/list
length. Native invalid frame/actor dereference rejects explicitly. Actual host
GameHumanSoundLoading registers its WINAPI thunk through that owner. It converts
the typed HeardSpatialSound payload, routes SAME Human/B0, captures callback
exceptions for rethrow and removes retained registrations before borrower death.

Evidence: human-containing-listeners/ and human-containing-final-dependencies/
successful exports, human-sound-listener-registration.asm and
human-activation-registration-tail.asm. Native ASan/UBSan uses actual
HumanSceneActor/ActorBase/storage/perception/event pool/speech cache with fixture
Inline/States/Sight/Services: both passes, profile/stat/speech order, context-list
retention, attachment, all3 acquisition failures and non7 gates pass. Strict
i386 composes actual state factory/owner/Inputs/StartupSession HumanStore/
Speech/Effects/Sight/SOUND, with mandatory control/Players/formation/registration
consumer declarations. App link passes; human-containing-verification-wip.json
fingerprints/removes four fixtures. No new ledger/world/desktop credit.

Next concrete consumers are Players/current/selected/override/frame mappings,
detached-camera reference list, formation and006DFCB0/006DFE50 pair scheduling.
The latter is a proximity-pair owner at008AED54, not GEffects material/shadow
registration:006E0380 computes pair ranges and invokes005A83F0. New successful
human-remaining-consumers/ exports cover that body and Players getters. Its
initialization can already emit B0+3C/+40, so it cannot be an empty service.

## Players control and actual proximity owner — 2026-11-09

`human_player_control_loading.h` now owns the SAME control roster08, indices
28/2C/30, override58, selection38, collected-frame records60 and Dog listC0.
0045D1D0 override wins; selected/previous keep their distinct source getters.
0045DA60 reverse-finds actual roster, sets2C/28, calls camera40(0) only when
current index changes, then clears30.0045E5D0 still performs first-player FPV/
visibility and HUD notifications for duplicate appends. Only a successful append
collects frames.0045D710/0045D7C0 reuse actual per-Human buffers and enumerate
ALL model+44 mask400 frames, with no marker-name filter or owned frame refs.
The host bridge catches enumeration exceptions and rethrows outside the DLL.

0045E670 swap-pops roster first, invokes HUD removal before changing current/
selected indices, restores replacement visibility, then destroys the removed
frame-vector owner BEFORE swapping/popping the corresponding60 record. Native
last/current removal reads a still-allocated old vector element: source retains
the old last borrower explicitly, avoiding an out-of-bounds access. It does not
invent corrected moved-member indices.0045D4A0 reverse-deduplicates borrowed Dogs;
0045DDE0 clears selected/previous/override/selection/Dogs, keeps collected60 and
sets current0 only for a one-member roster. Complete module activation, FPV/HUD/
camera consumers and populated pending order channels remain separate.

`actor_proximity_loading.h` now supplies actual008AED54/006E00B0 process state
and006DFCB0/006DFE50/006DFFD0/006E0380/006E0760/006E07C0 bodies. StartupSession
owns the SAME typed state across world lifetimes. Ten bucket periods100×2^index,
initial timers period+100×index. Clear frees entries reverse, pairs bucket-reverse,
retains capacities and timers. Fixed tick subtracts EXACT100 and updates only on
ZERO, not <=0. Register uses first ascending empty slot, native BYTE min/max pair
indices and type4/37 filtering. Remove frees matching pairs by reverse swap-pop,
trims trailing empty entries, emits NO invented Human exit callback. Speed table
00843504 is5/0/0/20/0, not radii; only an ordered increase reevaluates pairs.

006E0380 preserves world-matrix order on initial evaluation and ActorBase current
positions on subsequent checks; float component stores, Y²+Z²+X², signed bits
tiny-distance shortcut, near-unit/sqrt, signed gap comparison and stored absolute
float gap. Threshold is7.5 for Car,10 for37, otherwise5. Combined speed0 falls
back to exact float.01; time stores float(1000/speed×gap), strict bucket boundary
search uses following period. Initial-force and inside-flag transitions preserve
original synthesized0/1 callback arguments and actual bucket move independently.
NONZERO→NONZERO callbacks do not become invented near/exit events; that native
quirk is explicitly exercised. Pair moves append then swap-pop the old bucket.

005A83F0 uses the SAME Human command list78/7C.005AE640 adds actorID only after
reverse duplicate check;005AE810 reverse-finds ID then swap-pops. B0+3C/+40 for
ALL supported states1/3/4/5/6/7/8 are genuine RET8 (005830D0/E0,0057A370/380),
confirmed against section-mapped vtables/bytes; no AI event is fabricated.
`GameActorProximityLoading` composes actual Human/GameSceneActors/shared startup
state and callbacks. HumanLoadingActivation now takes this ACTUAL owner directly,
rather than leaving registration as a Services declaration. Runtime destructor
endpoint is named release_human_proximity; previous effect terminology corrected.

Evidence: human-remaining-consumers/,players-loading-owner-dependencies/,
human-proximity-feedback/,human-proximity-tick-clear/ successful exports,
human-proximity-update.asm,human-proximity-b0-map.json and fingerprinted,
section-mapped human-proximity-data.json. Focused native ASan/UBSan covers new
Players/current/duplicate/collection/removal/failure behavior and proximity pair
transitions, fixed ticks, speed/slot reuse/ID feedback. Strict i386 jointly composes
actual Human activation/state factory/input/Startup HumanStore/Speech/Effects/
Sight/SOUND/proximity/Players/compass with required control/VM/FPV/HUD/formation
declarations. App link passes. players-loading-verification-wip.json and
human-proximity-verification-wip.json fingerprint/remove four fixtures each.
No containing application-world activation/desktop/ledger credit.

## Loading weapon selection and inventory command boundary — 2026-11-09

`human_weapon_selection.h` supplies0052F750's ordered priority policy (actual
sector/target/distance context remains required),005304B0/00530930 candidate
selection and00530B30 usability. Hand3 precedes shoulder2; eligible kinds then
search holsters1/pack6, stacks ascending and entries reverse. Kind3/4/5/6/12
never search those stacks. Secondary kind4 reads the SECOND definition payload;
identifier16hex/7/104 is the special-item gate. Nonzero amount includes NaN, or
actual005A6C50 ammunition count is required. Exact squared boundaries400/1600
for priorities,100/225/1600/2500 for usability retain unordered comparisons.
Mask4 tries kind12 before kind4. Bits400/800 are never emitted. Empty priorities
leave the slot output untouched.005A6DB0 now restricts identity lookup to the
requested actual storage slot; it is not the whole-inventory005A6FF0 search.

005A2B40 dispatches B0+74; same-Human actorID becomes-1. AI states3/4/5/6/8
00587BF0 call005A2D50 immediately and only queue an outgoing kind7 packet when
mode!=7. Player1/remote7 queue it instead. Receiver assembly confirms005A6DB0's
ECX is the resolved SOURCE actor, not the receiving Human. Its007D8B40 call is
notify1/override0/retain0/dryRun0. Existing transfer_loaded_item is only the
no-source/module7E4false loading call and cannot replace this inventory move.
Actual00792AB0/00792720 equipment callbacks, pending model borrowers and DD0/E0
command writes are required. Holder+48=007D6890 compares unsigned game28 against
field18; +4C=007D68B0 stores that cooldown expiry. It is not an ownership flag.

Evidence: successful human-loading-weapon-selection/,human-loading-inventory-
commands/,human-inventory-command-receivers/,human-inventory-immediate-transfer/,
human-inventory-equipment-callbacks/ and human-inventory-equipment-retirement/;
human-inventory-receiver.asm and section-mapped human-weapon-selection-data.json.
Focused native ASan/UBSan and strict actual-host i386 pass; app link passes.
human-weapon-selection-verification-wip.json fingerprints/removes four fixtures.
Containing target/sector producers and actual command/equipment integration
remain required. No containing world/desktop/function credit.

## Actual offline loading inventory commands — 2026-11-09

`human_loading_inventory.h` supplies005A67C0/005A6830 callers and the exact
same-Human source1/2→empty hand3 offline AI command composition. It dispatches
only genuine AI states3/4/5/6/8 in mode7, resolves item identity again on the
actual source slot, then performs the transfer and actual230 notification.
Player1/remote7 queued commands and outgoing packets outside7 reject before
mutation. The Human owns a typed loading_inventory borrower; active/area loading
now calls that real endpoint directly. Bind it before B0 initialization and
retire B0 before this owner/inline98. Its HUD/FPV endpoints remain required.
005A6830 accepts2/9/10/11 but always selects kind2, scanning first stack entries
in reverse and retaining the lowest matching stack; field14 expiry is unsigned.

`human_weapon_transfer.h` adds007D8B40's source/no-source weapon paths for slots
1/2/3/6, including cooldown/expiry precedence, dry run, displaced-entry ownership,
retain weight accounting and notifications. It is source infrastructure for
007D9DE0, currently uncalled by its containing swap. The containing swap,
007DA5E0 preflight,007D9B40 fallback and active00530D40/00530A40 context production
remain incomplete. Other-family, cross-Human and world-drop paths reject.
Checkpoint ASan/UBSan verifies real holder/stack ownership, failed return paths,
retained exchange accounting, no-source placement and single release. All250
new headers pass strict i386 syntax compilation; app/startup link passes.
No completed P2.2 checkbox, containing world, desktop test or function credit.

## Active weapon reselection — 2026-11-09

`HumanActiveAIStateLoading` now supplies00530D40→00530A40 through the SAME
Human inventory owner and actual Items/B0/hand storage. Both gates compare signed
DWORD at B0+64hex>0 and command60!=-1. The first reads typed AI128+30, unwraps
Human160 attachment once, and selects active224 only when202==0, target is null
and action220 is25/26/22. Active224 remains a typed borrower; its runtime task
writer is not supplied.00530A40 refreshes actual hand, tests usability before
building priorities, and stores1A0=4000 BEFORE00538770 dispatch. An unchanged or
absent candidate preserves the timer; the debug0069BFE0 receiver is genuine RET.
Initial active00524E70 now calls this source directly rather than a required
external reselection method. Special-hand IDs16hex/7/104hex use the SAME Items.

AI target128 owns `HumanAITargetLoading<SceneActor<Frames>>`;005607B0 binds the
actual actor borrower and resets only scalar00/04/08/10/14/18. It preserves
20..2C and flags34/35. No pointer30 reads/writes pass through DWORD storage.
The native cold null target and active action-mode target reset use this owner.
Runtime targeting/steering and task acquisition remain separate producers.

`human_weapon_context.h` connects0052F750/00530B30 to the actual CD0 sector
through inline98/DD0005AC4C0 and SAME scene1F8 root, without a second sector
query. It reads Car5E0 through the required actual runtime service and actual
Animation actor type19hex+D8==2. Neither special-target read has a successful
placeholder. Usability resolves target/matrices only AFTER item/alternate/
ammunition rejection. A special target skips distance; priority distance is
read only in the root sector. Target matrix is updated before Human matrix.

Assembly corrects the decompiler's expression ordering:0052F750 at0052FB26
keeps extended subtractions and `(Z²+X²)+Y²` in x87 through its comparisons;
00530B30 at00530C24 computes `(Z²+Y²)+X²` then stores FLOAT at00530C4D.
The context retains long double for priority and explicitly rounds usability.
A tested distance slightly above1600 remains above for priorities while rounding
to1600 for usability. Older notes that report decompiled X/Y/Z order are
superseded by these assembly addresses, not by speculation about vector layout.

Evidence: WORK/human-active-ai-actions/00530d40.c,
human-active-ai-action-lifetime-checked/00530a40.c,
human-weapon-target-binding/005607b0.c,
human-inventory-weapon-swap/005ac4c0.c,
human-loading-weapon-selection/{0052f750,00530b30}.c and
human-weapon-{priority,usability}-distance.asm. The original binaries were only
read. [Saved verification](checkpoints/p2.2-weapon-reselection-20261109.json)
records native ASan/UBSan/LSan, strict i386 fixture compilation and app linking.
The fixture uses actual HumanSceneActor, RuntimeItem, ActorBase storage,
HumanActiveAIStateLoading/base bind and command composition, with external
frame/world/equipment receiver doubles. Initial active bind/close is exercised;
no original/Windows program, full DD0 activation or desktop scene was run.
Four eligible bodies gain partial credit; no complete promotions. P2.2 remains
open for formation/camera/Players, VM and full world integration.

## Next formation loading boundary — 2026-11-09 research

Formation source is not yet implemented.005A91A0 owns Human2CC's leader
borrower and allocates the leader's2C8 formation only when missing. Its required
bodies are005949C0/00597880 construction,00597F80 add,00597C50 leader selection,
00597420 mode3 and005977F0 shared scale. Clear removes through005981C0 and
retires a leader owner when count7C<2.005A9120 scans six members in reverse,
rereading Human2C8 after every follower callback because that callback can
recursively destroy the SAME owner. A retained pointer across that call would
be unsafe. The actual formation owner and leader borrower must remain distinct.

The owner allocates six A0 records (00595370/005953C0) with typed member70,
scalar/vector fields and an owned90/94/98 buffer per record. Constructor matrix
811190 and template table8AB1D0 are separate data. The template table is BSS:
range references locate its immediate/static initializer writes around00595xxx
through005974xx, including normalization loops and derived rows. Do not read
zeros from the on-disk BSS or guess a default formation. Native3 selects six
position/direction rows; constructor uses2.5 scale before005A91A0 applies the
shared0089AD40 scale.00597760 direction setter remains to export.

Leader selection00597C50 also updates matrix/direction/positions and invokes
0059B7B0 navigation projection. That calls actual0051D780, fallback0051D970 and
0051D5A0 on the SAME MissionNavigation dataset; preserve radius byte13, type16,
leader height clamp, float stores and failure semantics. Decompiler stack-vector
results in00597C50 are unreliable: confirm its engine SetDir/transform call ABI
and writes in assembly before implementing. Imported math calls receive no
client or engine body credit. A populated formation cannot be represented by
successful empty cleanup services.

Successful local read-only exports: WORK/human-formation-owner/ and
human-formation-storage/. Full table-write and call references are in
human-formation-template-references.txt. Ghidra jobs ended successfully; no
original binary execution. Next work is implementing/testing this owner boundary,
then detached-camera00463040 and Players module activation.00463040 scans its
8-byte records in reverse, resets actual Human camera amount005A9760 BEFORE
swap/pop, and copies only the last record's actor DWORD into the removed slot;
its producer/second DWORD still require recovery. Existing human_camera_amount.h
already supplies the actual amount setter; reuse it rather than a new callback.

## Formation loading owner — 2026-11-09

`human_formation.h` now supplies the required mode3 loading owner and actual
Human2C8 ownership/2CC leader borrower.005A91A0 creates an owner's six A0
records, adds leader/follower, selects the leader, restores mode3 and applies
the caller's SAME shared scale0089AD40.005A91A0(NULL) removes first, checks
remaining count<2 and retires the leader owner, then clears the follower borrower.
005A9120 rereads the owned pointer after EACH reverse follower callback; those
callbacks can recursively destroy the owner. No cached pointer or recursion
suppression replaces that original behavior. A full six-member owner still
publishes a follower's2CC even if00597F80's add fails.

005949C0/00597880/00594A70 use matching owned records/buffers.00595370 is absent
from the frozen inventory and earns no function credit. Per-record90/94/98 is a
vector of WORDs, confirmed by005953C0 assembly (not actor pointers or owned
nodes). Typed member70 and leader74 never enter scalar DWORD storage. Leader
selection/removal swap/shift MEMBER pointers only, retaining scalar/vector/buffer
identity for each slot. Native constructor stack W and unwritten fields use
zero guards, with no equivalence claim for undefined original bytes.

Mode3's six positions/directions come from005965E0 at8ABF50, copied to8AB410
by005971E0. Both are static source data; no function credit for these data or
for unspecified initializer entries. Native00597420 mode3 and13 close are
supplied; runtime cycling/order modes and populated runtime steering remain
excluded.00597760/005977F0 rebuild the SAME matrix through actual engine
SetDir export1002C7A0, scale its nine basis entries and rebuild records in reverse.
005978E0 uses actual vector transform1002EAB0, stored normalization length and
the original signed FLOAT-bit tiny-distance test. Exported math is delegated,
receiving no engine body credit. The shared scale must come from the actual
containing process owner; constructor2.5 is not substituted for that live value.

Assembly00597C50 confirms three leader matrix reads, normalized forward and
explicit XYZ translation writes, six transformed destinations30, then a full
XYZW position copy20 for slot0. This replaces the decompiler's false uninitialized
XYZ values.0059B7B0 projects through SAME MissionNavigation:0051D780 first, then
0051D970/0051CE70 type0F-only X/Z volume query ignoring height. The fallback's
lower bound accepts equality and reverse X uses extended×stored-FLOAT. Unchanged
3D volume-query semantics are retained. Projection clips to byte13/16 radius,
sets actual navigation index7E and flag88; type0F clamps Y to max(pointY+.5,
leader's actual cached positionY). No point on either query returns false and
preserves the original position. Undefined projected W remains a guard.

HumanLoadingActivation retirement and HumanPlayerStateLoading offline bind now
call these actual formation bodies, replacing their required external clear/
retire callbacks. Formation owners and borrowers must retire before their Humans
and SAME navigation/Frames owners; Human destruction alone is not world cleanup.
No containing mission or interactive readiness changed.

[Saved verification](checkpoints/p2.2-formation-20261109.json): focused native
ASan/UBSan/LSan passes actual HumanSceneActor ownership, mode3 constants, leader/
member identity, full-add quirk, nested ownership, recursive destruction,
actual navigation fallback/projection and89 injected frame/matrix failures.
Strict i386 instantiates actual GameActorServices/GameSceneActors Human types;
app compile/link passes. Engine math in the native fixture uses boundary doubles;
actual exports were compiled/linked but never run. Temporary fixtures are hashed
and removed. Seventeen eligible client partial bodies, zero complete promotions;
no desktop test or completed P2.2 checkbox. Next: detached camera and Players.

## Detached-camera Human references — 2026-11-09

Correction to earlier boundary notes:8C/90/94 belongs to detached controller
00462650, returned by MissionCamera004650A0. It is not a Players module vector.
`detached_camera.h` now owns that actual kind3 frame and typed Human/FLOAT records;
MissionCamera owns the SAME controller and forwards its speed and frame. Its
source template now requires the actual Human type. Frame release precedes record
buffer destruction, as004626E0; destruction has no invented amount callbacks.
HumanLoadingActivation now borrows this actual Camera and calls its remove path
before DD0/E0/230 retirement, replacing the external remove-reference declaration.

00463040 searches reverse, invokes005A9760(0) first, then copies ONLY the final
actor pointer into the removed slot before popping. It deliberately retains that
slot's old amount.00463620(false) resets amounts in reverse then clears records,
retaining capacity; true makes no call.00463890 walks selected control38 records
in reverse, then roster08 in reverse, with no deduplication between those passes.

00463780 computes the amount before lookup. Equal cached amount makes no call.
A changed existing row notifies BEFORE testing its OLD cached amount: old zero
erases the row with ordered shift, otherwise it stores the new amount. A newly
zero value therefore remains cached until a subsequent different value erases
it; that invocation does not reappend it. A missing nonzero row is appended
BEFORE notification. Native callback failures do not roll back these prior stores.
These quirks are retained instead of inventing ordinary erase-on-new-zero logic.

00463690 reads head marker260 matrix before Human model matrix, builds a segment
from head to bodyY+0.1 and measures from detached20.00581210 returns SQUARED
distance (not Euclidean distance); threshold constants are2.4 and0.4. It compares
extended distance>float2.4 before the FLOAT reload for the lower threshold and
linear fade1-(storedDistance-0.4)*0.5. Segment delta, dot, clamp and interior FLOAT
stores/order are backed by saved assembly. Degenerate/endpoints sum Z²+Y²+X²;
interior0041CFD0 sums X²+Y²+Z². Only known XYZ are used; undefined original W
lanes remain explicit source guards.

Evidence WORK/detached-camera-reference-producers/,
detached-camera-amounts-and-players/ and detached-camera-distance-players-reset/;
saved detached-camera-{amount,segment-distance,region}.asm and
camera-controller-tail.asm. Ignore detached-camera-human-list/ overlapping
address probes; they are not body evidence.004650A0 is absent from the frozen
inventory and earns no credit. Six eligible client partials are registered;
existing00462650/004626E0 coverage extends without duplicate credit. Engine/frame
calls remain actual required boundaries, not reconstructed engine bodies.

[Saved verification](checkpoints/p2.2-detached-camera-20261109.json): focused native
ASan/UBSan/LSan uses actual HumanSceneActor, Players roster/selection and Human
amount setter; tests segment branches, cached-zero/pointer-only quirks, call
order, reverse traversal, failure publication and camera lifetime. Strict actual
GameActorServices/GameSceneActors Human/Players/camera instantiation and app
compile/link pass; all Windows programs/DLLs remain unexecuted. Native tests use
frame and DD0/presentation boundary doubles. Temporary fixtures hashed/removed.
Detached motion/toggle/input/00469300 simulation and containing world/VM
remain separate. Players loading owners follow below. P2.2 checkbox remains open; keep implementing it after commits.

## Players module and process — 2026-11-09

`players_process.h` reconstructs known00456780/00456440 state at CRT singleton
0089AC30, separate from module11 control1C.004563F0 produces actual0089AD40
formation scale: FLOAT(signed120) ×0.75 +1.5, initially3.75 XYZ. Reset preserves
all five group buffers and separate0089AC20 delayed-speech records. StartupSession
owns these process lifetimes; they are not new per-world copies. Unwritten
constructor/W fields remain guards, with no full-constructor promotion.

004564A0 expires wrapping timers30/34/9C and clears associated3C/40/98 only on
nonzero expiry. It clears all five retained group buffers before selected28 lookup;
no selected Human means no speech work. Actual roster is traversed reverse:
positive-health, other, B0kind3 Humans use extended position differences and
FLOAT((Z²+Y²)+X²), strictly below1600. Groups0/1/2 retain native pending-order and
202 gates. Loading exposes no populated250 order producer; unknown raw writes
reject rather than silently count as empty. Due speech compares signed due time
to wrapping game2C+elapsed, calls required005383D0(127,1,300,0) reverse BEFORE
ordered erase, retaining prior stores on failure.00456EF0 is a genuine RET4.
004563E0 is only the singleton getter: actual virtual34 is004593E0, which walks
fresh ascending roster, resets state3 followers through005377F0(false,188), then
clears their actual2CC formations.004593E0 is absent inventory and earns no credit.

`players_module_loading.h` supplies actual00461430/004614E0 ownership and
00460AA0/00460AE0/00460B30/00460C20 gates. It embeds SAME control1C and separate
auxiliaryE8. Control virtual18 is0045DDE0 post-load, not0045D3A0 clear: roster
survives. Auxiliary0045FC80 clears roster and reverse-retires raw2C allocations;
0045FD80 preserves roster. Its record type remains opaque without an invented
populated allocator. Module authority finish drops subgroup pointers without
calling their destructors; explicit reset destroys nonnull subgroups reverse.
The real subgroup/save producers and mode9 formations remain separate. No
subgroup append API or successful replacement receiver is supplied.

Evidence: WORK/players-process-ownership/,players-process-consumers/ (all listed
valid entries except overlapping0045DDA0),players-auxiliary-records/,
players-owned-record-producers-correct/,players-loading-records-final/ and saved
players-process-tick.asm. Discard overlapping00460170 export guesses. Fifteen
eligible new client partials; absent00460B30 and004593E0 earn no new credit.
Native ASan/UBSan/LSan uses actual Human/Active AI/input/formation/Players owners
and tests reset preservation, grouping boundary, expiry, speech order/failure,
follower cleanup and registration lifetime. Strict actual-host i386 and app
compile/link pass. Frame/presentation/speech boundaries use explicit doubles;
no original/Windows program or DLL executed. Saved checks share the camera
[checkpoint record](checkpoints/p2.2-detached-camera-20261109.json).

2026-11-10 continuation: [Human/world composition](human-world-composition.md) now owns SAME DD0/E0/230/B0/inventory/activation lifetimes, source type3 dispatch and per-Human SOUND factory routing. Actual precompile005A3DF0 and postcompile005907C0 use source family contexts with scoped pending EC lookup. The application remains unloaded; continue containing VM/session/FPV/HUD/SOUND/module owners.

2026-11-10 continuation: [player/first-person loading composition](first-person-loading-composition.md) now supplies005469E0, actual inline action17/18 receivers, application+0C preference, same-overlay FPV ownership/reset and source current/inline/B0/sector/retirement lookup definitions. Actual containing presentation/HUD/SOUND/session and guard publication remain; application still unloaded.
