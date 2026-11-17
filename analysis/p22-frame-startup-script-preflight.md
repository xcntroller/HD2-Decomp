# Tutorial startup script preflight — 2026-11-17

User requested checking ahead after202 HUMAN_SETMODE_Stand. Earlier single-error
fixes missed other admitted scripts and automatic predicates. Actual d60751d1 run
completes activation/Scene78/fixed/input/update then exits result1 on202. Archived
exact executable/logs:WORK/first-frame-script202-return. No desktop game acceptance.

The read-only resource resolver selected the CURRENT missions/tutorial/scripts.dta
and all114 named script resources, retaining origin/size/SHA in startup-script-
inputs.json. A temporary native utility uses the actual MissionScriptCompiler and
exports full instructions, pools, expressions and events; all114 compile. No
original/DLL/Proton/Windows code is executed. This is static inspection, not a
runtime with successful dummy command services.

The initial control-flow pass starts at actual start PC/0, follows jumps and both
If branches, skips event definitions and stops at the first delay/park/return.
It does not claim engine frame admission or predicate truth.13 distinct missing
command dispatches are potential startup dependencies: HUMAN_Move, FRM_SetAnimation,
Car_SetFuel, Stand, plus9 weather setters. Full-program/event occurrences are
recorded separately, not all declared necessary before entry. Automatic Whenever
expressions additionally require _SeeEnemy, _FrameInRange and _IsInInventory;
_PlayerInRange, _LoadGameValue, game type and team-dead receivers already exist.
Broader delayed cutscene/speech/subtitle/hint/actor-state dependencies remain
visible in the full report. Dispatch presence alone does not close conditional
receiver/ownership guards. Raw report:WORK/startup-script-coverage-before.json;
predicate census:WORK/startup-script-predicate-census.json.

Batch boundary: complete known startup dispatch plus its reached positive
receivers, native checks and containing build before requesting another desktop
run. Keep known later/event-only work distinct and honest. Do not patch a required
failure to success or disable weather/AI merely to pass this pass. No extra plan
checkboxes; this is the active first-world-frame dependency boundary.

First implementation group supplies HumanC5/C6/C8/C9/CA/CB and CD/CE mode requests,
with exact native1B0 masks and existing state-kind5 gate.202 Stand and StandFast
clear1D1 BEFORE actual inline action1D/26 when set, then return without rewriting
1B0; otherwise clear low bits3/set4. Native0058F390/0058F420 proves this distinction.
Walk/silent/lie/crouch and aggressive/defensive share existing real state consumers.

Actual HUMAN_Move1F performs005AF210→0058E8A0/0051E1A0 on its own navigation
owner (FIRST table; corrected in the gameplay-controls checkpoint below),
missing name advances; valid name invokes005AF1D0→0058E780 and parks PC. That body
refreshes alarm action BEFORE reading current point, updates real query54/request
point, publishes1D8 PC before optional0058C400 readiness callback. Equal point
sets query2000 only when10000000 is set, invokes SAME optional context PC-match
completion, then1D8=-1. Existing query/path/frame/completion owners are reused.
No substitute graph, duplicated state or successful missing point is introduced.
290 native ASan/UBSan/leak checks exercise actual Stand/navigation policies with
real query/table owners and explicit action/readiness/context endpoints, including
positive callbacks and failure publication order. Production wrappers/VM/factory/
World are separately compile-verified. This does not finish the whole batch.

Evidence: index before new exports; original client SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Bounded raw Human mode bridges/states plus new read-only0058E780/0058E8A0 C/ABI
exports are under WORK. Keep compiler and Ghidra sequential. OWN source/artifact
and the uncommitted interactive review overlay must remain preserved. No matrix
or function-count promotion. WORK=analysis/work/p22-frame-closure-20261117.

Second group: automatic predicate/frame/fuel receivers

The source now supplies all previously missing automatic Whenever predicate
TYPES:0D SeeEnemy resolves actual context frameEC Human and SAME state5/268 list;
0F FrameInRange preserves null1000, nonoffline dead/control/suppression gates,
context-before-target world getters, FLOAT square/tiny/near-one sqrt and native
truncation;13 IsInInventory reads the real signed definition and searches SAME
reverse Players roster/Items inventory, with fresh Human after health. No fake
boolean is substituted for actual gameplay. Added nearby0A language predicate
(string pool then real startup language comparison) and12 Human alarm type via
state5/1B4; positive Dog alarm remains an explicit required boundary.27 native
ASan/UBSan/leak assertions cover predicate policy/gates/order with actual context
and explicit frame/Human/World endpoints. Actual production wrappers and World
compile/link separately; full engine execution is not claimed.

Car_SetFuel62 now resolves actual slotEC/type4 before reading the integer, sets
SAME initialized Car loading/core through existing004F2670, then guarded-advances.
Original rawFILD multiplies signed input by FLOAT0080F550=.10000000149011612,
with a single FLOAT result spill. It is not a percent/clamped integer or float
input conversion. Existing capacity-only clamp and negative behavior are unchanged.
Nonoffline004DB310 publication remains an honest post-set required boundary.
Missing/wrong actor takes original shared advance/true. Full actual O2/Werror
application/probe compiles this containing dispatch; no added Car simulation.

Current initial dispatch gaps after these groups: frame animation and9 weather
setters. Automatic predicate dispatch gaps are closed for the inspected114
programs, conditional positive receivers remain revision-qualified. Do not stage
or ask another desktop run merely because those TYPE gaps are gone: GEffects
positive weather splash/actor paths and frame-animation cleanup must be supplied.

Third group: model animation lifecycle and weather

FRM_SetAnimation5D now supplies the actual model-kind9 gate, copied string/slot,
cache3 clip mode1, model84→clip Release→model9C→snapshot8→registration→model24
order. Short/failed clips take9C→8C→84(null)→unregister; they do not snapshot or
claim an animation.00424260/00424310 retain/deduplicate Script87C and parallel88C
flags, separately from actual World48 ticking.00776070/C0 use the exact literal
`a`, actual parent-model EC actor gate and callbackF4 gate; positive unbound
markers reuse SAME existing00774D80 registry/SOUND owner, not an original address.
World animation ticks now propagate that callback's captured failure immediately.
0069A360 raw proves Release then replacement by last entry/pop, not ordered erase.

Finish restores snapshot8 BEFORE contexts Exit: remove slots6..0, enumerate
004281D0's external immediate attachments, copy parent names before detaching,
reload actual model134, build SAME GEffects shadow, update, then reverse reattach
to matching reloaded parents/fallback model and replace existing retained snapshot
parents. Context destruction precedes Script animation queue retirement.77 native
ASan/UBSan/leak checks verify actual dispatch/queue/snapshot policy with explicit
engine/World/marker endpoints. No delegated engine body credit or desktop claim.

The9 selected WEATHER commands now write actual module4/priority0 broadcasts;
the containing decoder reads opcode once and consumes real payloads. Speed/size
use signed FILD with extended clamp and final FLOAT spill; direction preserves
Z's retained FILD precision/near-one/tiny behavior. Colours clamp and pack actual
RGBA bytes. Mode writes input==1; decoder toggles SAME GEffects24C after scene14,
even without a scene. Space decoder0042B90C uses **unsigned** DWORD→FILD64→FLOAT,
whereas authority-only direct mode uses signed FILD32; raw assembly resolves the
otherwise misleading decompilation. Actual scene+11C owns rain rendering.

Positive client splash00487240 is supplied, rather than disabling24C: persistent
BSS0089BAA4/008396E8 accumulation/cadence, original250 density200/254 radius8,
100ms periods, SAME CRT00475190 (not Engine RandFloat), static-only005E2CC0 rays,
material10000 choice44/45 and exact .02/up/orientation outputs through SAME
00445150/module17 owner. Every third period uses fresh camera/root/epoch, actual
reverse Players roster/head260, Engine scene60-byte shelter records, engine
inverse/vector transform and lower-inclusive/upper-exclusive bbox, then actual
0045D6D0 random collected marker and attached44 emission. Count-before-pointer,
random Z-before-X and normal-tail counter publication are retained. No replacement
Players list, collision tree, RNG or particle service is introduced.2399 native
ASan/UBSan/leak checks verify actual packet/normalization/cadence/splash/Player
policy with explicit engine/World leaves. Full actual O2/Werror application/probe
build PASS; new source is applied in-place to review preserving OWN differences.

After all three groups, **initial dispatch gaps=0**, but this is NOT a launcher
acceptance boundary. Full-program report still has22 missing types. In particular
TUT_Instructor1 resumes its2000ms Delay into CSC_RunCutscene162, whose full
camera/HUD/context/animation start/end ownership is the next required chain.
Continue that opening sequence and automatic handlers before staging/retesting;
do not hide known delayed failures behind the first-yield report. Full report:
WORK/startup-script-coverage-initial-batch.json. No desktop verification, matrix
promotion or new function-count credit. Native audits fingerprinted/removed at
the source checkpoint; extraction/compiler inspection tools remain while needed.

Fourth group: opening cutscene and continuation

Known2000ms continuation now has actual00423C70/00423EB0 source:824 publishes
first; save actual command visibility, hide through SAME UI, retain camera828,
reverse context cutscene notification and conditional nonplayer Human activity;
real IsIconic Steam site25 action, game6E input disable, scene24C/250 initial
rectangle, actual current-camera82C/830 projection save, native aspect/FOV and
SAME dubbing retirement. End clears824 before restoring/releasing saved camera,
notifies cutscene-done contexts, conditionally reactivates non-suspended Humans,
consumes saved FPV/secondary flags, sets actual500ms timer, restores full viewport
and native4:3 aspect, configures fade and retires ONLY cutscene87C/88C records in
marker→World unregister→snapshot destructor→Script Release order. Timer00428490
uses original signed/wrapping decrement before loaded/context filtering and then
reenables game6E. Finish invokes this after context/animation retirement, before
its existing final fade reset. No868 spline producer is enabled by these commands.
86 native ASan/UBSan/leak checks pass actual cutscene/animation queue policy with
explicit context/camera/HUD/platform leaves; actual production ownership separately
compiles. Current UI false/hidden branch performs original current-Human refresh;
positive command-modal transitions still honestly require camera/order producers.
No HUD flag/readiness default or successful missing GUI service was installed.

Human commands38 named special animation and3F working mode reuse actual DD0/C00
and state5 owners.005AF780→00591230 in offline7 calls actual005A9C20/005AC4A0/
0077CE10; nonoffline publication remains an explicit AFTER-effect boundary.
43 nearest player traverses SAME selected primary/auxiliary roster, health then
005A63F0/005A63B0, source-before-target world getters, strict extended squared
distance and FLOAT best with reverse ties.25 Stop preserves00591ED0 alarm request,
1C0 clear, optional SAME00572C20 aim stop, then a fresh nullable12C query2000 OR.
396 native ASan/UBSan/leak checks pass actual nearest/Stop policies, real query/
scalar owners and explicit aim/roster/engine endpoints; unchanged special-animation
workers were reused, not comprehensively re-audited.

6C/73 speech uses real module4 name/32/8/8 packet; decoder resolves actual scene
frame and invokes SAME SOUND50 with NULL actor/subtitles true.73 delays by actual
SOUND94 even on null frame, parks PC, and uses the actual MissionScriptContext
countdown; no implicit advance for zero duration. FA/FB subtitles use actual
renderer/list, preserving geometry queries only when required.84 camera slide
uses existing real look-target angle receiver. FF/100 retain exact ordered hint-ID
vector and duplicate entries;8F uses actual SOUND68 secondary-music selector.
95 SetActorState now supplies actual Door6 request3-(value!=0),true,false,false;
null/default native type gates are retained and other native positive families
still reject explicitly.38 native ASan/UBSan/leak checks pass actual VM/FIFO/
speech/subtitle/hint/music policies with explicit scene/SOUND/UI endpoints. Full
actual application/probe final O2/Werror build PASS. No user staging yet.

The check-ahead closure follows literal ScriptAssign targets recursively:
**131 programs** (114 initial plus17 additional selected resources), all compile;
no missing resources/includes/new predicate types. NPC1..4 are real reached
assignments. The earlier recursive report exposed seven remaining TYPES:
1C CreateItemInStore,64 TeleportNearCheckpoint,72 ITEM_Teleport,7F CreateParticle,
91 SetNPCTeamStatus,92 MakeExplosion,FE RemoveItemsFromStore (hex). All seven now
have actual dispatch and containing positive-owner bindings. Reinspection through
the actual compiler yields identical131 programs and **zero missing dispatch
TYPES** across their entire programs, including events/delays/cutscenes. Report:
WORK/startup-script-coverage-recursive-final.json. This is a static census, not
execution of every branch, proof of all runtime values or engine equivalence.

Fifth group: recursive NPC/items/particles and teleport closure

Human1C invokes005AF760 without a B0kind5 gate: signed definition/slot reach SAME
Items007D7810 with identity-1. The existing actual item factory/transfer/equipment
owners remain authoritative. FE repeatedly uses005A78F0 definition lookup, removes
actual identity10 through005A7B70(destroy=true), and publishes Human178 even for
no matches. Retirement failures preserve the already executed prefix.91 resolves
slot then BOOL then EC/type3/19C4-or5/positive health, calls actual primary Players
0045D100(true)→0045E5D0, then working mode0/suspendfalse/end Human Script state.
TRUE unconditionally ends THIS context and increments PC, even for null/wrong/dead
target, bypassing the ordinary advance gate; FALSE uses the ordinary guarded tail.

7F writes actual module4/priority0,8-bit definition then frame NAME, without a
successful null-frame skip. The decoder reads8/stringFF, requires actual scene,
findsFFFF, then emits through SAME process with the frame's full world MATRIX,
frame parent,true,false.92 obtains actual frame world pose BEFORE signed integer
power/radius; FILD×FLOAT.001 with one result FLOAT spill reaches SAME006DF170
wave(NULL,position,radius,power), no additional sound wrapper/input-positive skip.

72 uses both slots/type11, actual005B7920→005B76A0 and SAME registered loose Item.
Link38(parent,1), XYZ0, direction1C(+Z,0), update, unlink38(NULL,1), scene placement,
show/update and fresh world position/normalized direction precede005B7DA0's actual
body close/mass/category/configure/reset/actor restoration. Existing lifetime
counters survive. Offline7 returns; nonoffline command3 remains required AFTER
the local effect. Read-only005B76A0 export plus bounded raw resolves misleading
uninitialized decompiler direction; no original code is executed.

64 FRM_TeleportNearCheckpoint copies BOTH strings before nullable-frame handling.
Actual Car4 uses SECOND names/positions, other actors and plain frames FIRST.
Missing first point is the native skip. Car004E95B0(0) suspends SAME live graph
before pose stores: flags0 skips skid hiding but STILL transforms angular/linear
velocity and retires projector/particle/collision/contact/sound state. Actor30
position precedes model pose; Cars use inline0041D110 XYZ/static80 handling, other
actors/frame use actual08. Human0041D800 clears228, calls SAME CD0 position sync
TWICE with fresh base30, publishes CE8/DB4 and actual Scene144 BEFORE optional
direction. This exactly reuses existing publish_seat_ground_position. Optional
second point uses original X²+Z²+Y²/FLOAT/epsilon normalization; actor1C/base40 vs
plain frame0C remain distinct. Model24 precedes saved Car22BF,004E9DB0(0,true),
004F2530(saved,false). No whole-round reset, replaced actor or copied physics graph.
Native stack fourth-lane padding gets deterministic zero; XYZ behavior is preserved.

Verification:51 actual inventory lookup/retirement,77 actual lifecycle dispatch/
PC/termination/explosion,244 checkpoint teleport ordering/numerics and actual
particle packet,52 actual flags0/2 Car suspension checks PASS under native
ASan/UBSan/leaks. Explicit engine/World leaves are doubles, not engine execution.
Actual review application-launch/startup-probe full O2/Werror rebuild PASS. New
source matches review except its preserved OWN focus callback. OWN's six dirty
sources and older staged exe hashes remain unchanged. Temporary fixtures are
fingerprinted/removed; compiler/resource inspection utilities remain for closure.
Receipt:analysis/checkpoints/p22-frame-startup-recursive-20261117.json.

Scene-target follow-up: all27 literal SetActorState sites were matched against
actual selected Tutorial Scene2 actor records:13 distinct targets,12 Door6 plus
one **Pike28 Zavora1.Cylinder01**. Dispatch-type presence had hidden this concrete
positive receiver gap. Report:WORK/startup-script-actor-state-targets.json. It now
uses actual005B8EA0: guarded SAME context Use notification, then module13/Scene2
identity10/one-bit command005B9E70 queued BEFORE publishing184. Actual005B9EC0
consumes one bit, preserves start-sound-before-loop and the native startID=-1
outer gate, retains a single real voice and publishes state2/3 last.

The Pike's positive frame is also supplied, rather than enabling a command whose
next update still fails.005B9340/005B9470 preserve authority mode2/stationary skip,
wrapping timer, unsigned FILD64 elapsed, extended endpoint comparisons and FLOAT
angle stores. Presentation endpoint sound/voice retirement precedes tentative
rotation; obstruction restores only the prior angle and sets188=200, retaining
already published endpoint state/sound.005B8F00 uses SAME common captured quaternion,
real engine axis-angle Make/Normalize/update, own collider exclusion, actual
driverAC bounds,0.01 inset, reverse world transforms, exact12 edges0083DFE8 and
extra1→6/2→5 rays through SAME collision owner (2,0). Exclusion clears BEFORE
rejected quaternion restoration; failures also clear the borrowed exclusion.
Presentation local-cache/world/inverse/descriptor prefix precedes existing collider
refresh. No guessed geometry, no always-clear obstruction or replacement sound.

289 additional native ASan/UBSan/leak checks PASS actual Pike quaternion, packet,
Use/state ordering, sounds/voice, all14 ray stop points, exclusion/rollback and
frame policy with explicit engine/collision/SOUND leaves. **713 checks total**.
Full source branch application/probe O2/Werror build passed before PC restart;
the interrupted interactive review build was rerun successfully after restart:
WORK/startup-recursive-pike-resumed-review-build.log, fingerprinted in the final
receipt. Initial nonstationary Pike activation outside this Tutorial record is
still qualified; other SetActorState families are not promoted by literal targets.

A newer external desktop run appeared during work (not staged/executed by this
batch):13feab9e at17:43UTC again reaches all four first-frame stages then required28.
Exact executable/logs preserved in WORK/first-frame-script28-return-20261117T1743.
The new verified batch is staged with --stage-only for user desktop validation;
no Windows/DLL/Proton/original execution by the agent. Actual stable gameplay,
positive command-modal GUI transitions, other SetActorState native families,
Dog-specific predicates and nonoffline publications are not promoted by this
census. Do not reopen unchanged source audits or add delivery checkboxes. Next:
user Tutorial entry/continuation; investigate the exact new runtime failure if
one occurs. No FRAME/HUM/OWN desktop or whole-function/count promotion.

Cutscene desktop discrepancy follow-up

User reached and played the Tutorial cutscene on staged41bb65eb, with HUD visible
and wrong view, then reported a crash. Actual preserved host log records orderly
teardown result1 on `Vertex animation disagrees with actual mesh format/count`;
it is a checked adapter failure, not proof of an access violation. Exact exe,
logs and screenshot:WORK/cutscene-vertex-failure/manifest.json.

A concrete Scene2 parser defect was confirmed against original006CD940 tag22:
file words are WXYZ, copied to temporary XYZW before0041D180 writes engineC0 and
normalizes. Our generic four-real read preserved WXYZ, incorrectly making the
serialized scalar the engine X component. mission_scene_rotation now converts
ONLY this serialized boundary. Existing runtime quaternion setters stay XYZW.
Actual camera_1/camera_2 records and original source provenance are preserved in
WORK/cutscene-camera-scene-records.json / cutscene-rotation-provenance.json.
This affects all tag22 Scene2 orientations, not merely an invented camera offset.
22 focused native ASan/UBSan/leak checks pass actual decoder geometry (identity,
yaw and both captured cameras), truncation and HUD/text callback policy.

As a containing presentation fix requested by the user, UI suppression now also
queries SAME Script module824 cutscene state. It suppresses both HUD prepass and
draw while keeping caption/fade/effects callbacks. End clears824 and restores
normal HUD through the same live query; no extra saved flag/lifetime is added.
This is an explicit integration policy, not a claim that original0049B7C0's
retail RET4 secretly implements visibility or that native scene flags were set.
Actual engine visual confirmation remains pending.

The vertex guard stays enabled. The old message omitted the object and values
needed to identify the mismatch. It now names the frame, LOD, definition vertex
count, actual mesh vertex count and stride; active-camera changes also log names,
world position and forward axis. Original00473960 does32-byte mesh writes;
engine100311E0/10031380 and existing collision evidence establish54 count and
the fixed32-byte BASE buffer. The initial diagnostic mislabeled64 as stride; the
following correction establishes it as mutable render/stream flags.
No speculative clamping/skipping or out-of-bounds write replaces that guard.
One user diagnostic run is required for these actual engine values; agent execution
of the original/DLL/Proton/Windows application remains prohibited. The termination
is NOT marked fixed or source-resolved before those values arrive. Updated source
and diagnostics have production builds/receipt; final staging uses --stage-only.

Vertex flags guard correction

User diagnostic run on73e4cd80 identifies `la_GB_flag_1.vlajka`, LOD0, definition
224 vertices and actual224 vertices, but the diagnostic's mislabeled stride=0.
Archive:WORK/vertex-zero-flags-return/manifest.json. The error is in our metadata
interpretation: LOD64 is mutable render/stream FLAGS, not vertex width. Native
10031815 ORs8,10031CCA ORs20 and10031C19..22 clears20; zero is valid after render
preparation. Constructor10030DB0 setting64=20(hex) was misleading without these
consumers.10031900 also tests these flags to prepare a distinct render stream.
Actual base allocation100311E0 passes fixed32 to10066BF0;10031380 returns the
base lock through LOD34/38/3C. Client00473960 writes eight-DWORD/32-byte rows,
independent of flags64. New original static evidence under WORK/vertex-lod-state.asm,
vertex-buffer-allocation.asm,vertex-lod-vtable.json and the lookup receipts.

Removed all five mistaken flags==32 checks: animation, fragment source, Car
deformation and both Car strip paths. Keep exact base row width, actual vertex
counts/index/capacity checks, null-buffer checks and existing lock/unlock/invalidate
ordering. Vertex capacity failures still report frame/LOD/definition/actual counts;
there is no guessed stride, render-flag mutation, missing animation skip or
successful missing buffer. The same host's face writer already correctly uses
LOD64 as flags; those legitimate writes are unchanged.

Read-only asset preflight independently compares all four actual Tutorial flag
instances, their selected Vertanim definitions and decoded la_GB_flag.4ds base
geometry: every LOD0 count is224 and base row is32 bytes. Saved selected-byte
fingerprints and counts:WORK/vertex-flags-asset-preflight.json. Actual review
application-launch/startup-probe full O2/Werror sequential rebuild PASS; new
executable staged ONLY via --stage-only. No new mirror-only native test was added
for deleting an invalid metadata predicate; earlier interpolation/ownership audits
remain revision-qualified and unchanged. No Windows/DLL/Proton execution or
new desktop, function-count or matrix promotion. User continuation remains next.
Receipt:analysis/checkpoints/p22-frame-vertex-flags-20261117.json.


Gameplay return: movement, spawn events and cutscene projection

User confirms actual Tutorial gameplay entry and correctly hidden cutscene HUD
on ac6c4b78. This accepts those two observations only. Cutscene view, duplicate
selected/placeholder soldiers, missing helmet/backpack and movement remain reported
failures. Exact executable and logs are preserved in
WORK/gameplay-movement-crash-return/manifest.json. Movement is now an actual engine
access violation: LS3DF10018E5B indexes its frame-kind table with EBX=4A4F5250
(PROJ); client return0045F795 is the footprint creation path.

Original004546E0 creates PROJ through driver virtual50 (visual subtype), while
our host incorrectly passed that FourCC to virtual54 (frame kind). Corrected
ShotSurface, and the same concrete mistake in shell00454190 (OBJ_) and trace
0044B030 (BBRD). Shell raw bytes00454201..207 and indexed original exports confirm
all three virtual50 calls. Engine/model RAII ownership, clone/update calls and
pool lifetimes remain unchanged. No frame-kind success fallback or disabled
footprint replaces the failed call. Focused scan found no remaining literal
FourCC passed to driver_.create in the host.

Original00611C90's trailing0 to0041D960 was mistakenly translated as the source
method's authority gate. Native0041D960 actually reads process00982100 bit1;
the trailing0 is not that gate. PreparedHumanLoading now rereads actual mode bit1
at the existing late frameEC/context event boundary, preserving identity match,
publication and activation order. No alternative actor or extra script tick is
introduced. Tutorial Player_1..4 OnUse assigns NPC1..4 to the actual spawned Human;
without the event, Player_2..4 run their delayed placeholder branch instead.
The existing T_panak scripts suspend placeholders. The newly reached Player_2
OnUse also contains a Human-only1C instruction on its base SpawnPoint context.
Static native dispatch maps opcode28 through selector118 to0041861C, RETfalse
without advancement or side effects. Preserve that exact known decline rather
than aborting as an unknown receiver; Human-derived1C still executes its real
item receiver. The base context parks there after assigning NPC2, as in native.
NPC scripts grant actual
item80 into holder0 and item101 into holder5. Selected items.sav confirms80 is
kind16 B_EN_HELMPAR and101 is kind17 EN_bag01. Thus the repaired event also restores
the original equipment-request path; no hardcoded starting inventory is added.
Actual visible equipment/placeholder behavior still needs user confirmation.

A separate confirmed cutscene defect is the projection source: original006CC760
root3010 writes native gameBC; our Scene2 parser retains that same property in
MissionSceneFields, already borrowed by normal camera/FPV. Script startup instead
read an unwritten scalar at session.words[BC], giving zero field of view. It now
borrows the SAME loaded scene field through World, without a second projection
copy. Actual Tutorial root3010 is1.3962633609771729 radians; root3011 far clip120.
This explains the zoomed cutscene despite corrected orientation/look targeting.
Existing camera selection, target, projection-save/restore and HUD policy stay in
place. Source correction is not visual acceptance.

Verification:34 focused native ASan/UBSan/leak checks exercise the actual
PreparedHumanLoading notifier, captured Player_2/3/4 compiler/VM and actual frame
commands: authority-disabled calls leave PC unchanged, offline calls enter OnUse,
real-player NPC assignment occurs exactly once without placeholder assignment,
Player_2's base1C parks without a false item request while Human-derived NPC
scripts request both original equipment definitions, all retained frames are
released, context mismatch still fails and Script FOV dynamically follows its
borrowed owner. Engine/item/binding leaves are explicit doubles. No engine or
Windows execution occurs. The actual interactive REVIEW launcher and probe must
also rebuild sequentially, then stage only. Receipt:
analysis/checkpoints/p22-frame-gameplay-spawn-20261117.json. Keep camera, movement,
loadout and squad visual acceptance open until the user's next run; no new broad
FRAME/HUM/OWN matrix or whole-function/count promotion.


Gameplay controls: cutscene, soldier routes, prone, climb and chat

User accepts the preceding4efed3f5 run: camera projection, movement crash,
selected/placeholder overlap and missing equipment are all fixed. Keep that
acceptance revision-qualified. New reports are stationary cutscene soldiers and
barracks return, unavailable cutscene skip, suspected subtitle height, prone
teleporting, climb requesting prone, upward Ladder top jitter, chat Enter failing
with missing console producer, and pause status14. Downward Ladder exit works.
Exact archived executable/logs:WORK/gameplay-controls-return/manifest.json. That
archive also contains a distinct missing HUD selector producer; it does not explain
the user's clarified chat Enter error.

The original005AF210→0058E8A0 loads this Human B0's navigation atA0 before
0051E1A0. Its factory borrows FIRST. Our script adapter instead called the
vehicle SECOND-table lookup. HUMAN_Move now reads the SAME state.navigation()
and MissionNavigation named lookup; vehicles retain their SECOND dataset.
All46 route names in actual Tutorial T_CSCSol1..5 resolve to valid FIRST points
and are absent from SECOND. Actual check2.bin SHA
2164a9ce012c30841e9c4bcd05ee5a609c5026f56ff6b0da6f8e3a677464425f.
This restores requests to the existing navigation/path/AI/completion owners;
no invented route, extra tick, direct teleport or replacement animation.

Original00586200 action1F sends E0 kind4/action1E through005ABD90/0077F050.
The adapter called movement kind1/0077EC00 with30, which requests prone. It now
uses the real Look action_command(0x1e,true), preserving the actual48/dirty2 gate.
Prone has a separate coordinate error: original007AB589..007AB5E1 reads bone22
world coordinates then calls virtual08. The real joint implementation1001C9F0
converts world position through the parent inverse at1001CA4B..1001CAC6. Our
adapter instead stored those world coordinates directly as localB0. Bone22 now
uses that actual world setter; shared local position writers are unchanged.

For upward Ladder exit, original0074A690's local_2ac is baseY+radius. The source
started the upward sphere sweep at baseY, embedding the query radius in the
floor. The failed clearance worker then returnsfalse and00768230 queuesCE,
repeating on subsequent frames. Start at the original sphere center; preserve
Ladder backwardXZ offset, radii/masks, obstruction rejection, descriptor14,
occupant/equipment/control lifecycle and subsequent root motion. Downward path
is unchanged. This is a source correction with doubled collision geometry, not
an observed desktop climb completion.

Cutscene skip0069A560 and subtitle rectangles both read process0086922C, whose
source full_viewport has no live writer. Retail0049B7C0 is RET4, not a recovered
setter. Use the SAME actual Script824 cutscene state already selected for HUD
suppression, through the existing checked module link, for these containing
consumers and the text-stage viewport. This is explicit integration policy, not
native setter-body credit. Script link unbind retires the callback before World
owners. At800x600 original full-viewport subtitle bounds are bottom594/top528;
normal reduced layout is bottom510/top444. The previous path selected the latter
during the cutscene, explaining the suspected high position. Actual presentation
and Escape skip remain desktop verification.

Chat uses actual0072EA90 request3 →0072B1C0 FIFO →00727800 authority3 →0072A0B0
presentation4 →00728450 receiver. It is unrelated to spawn presentation3 and to
the scalar control FIFO. The default console chat endpoint now borrows the real
StartupSession producer; unrelated console commands still require their own
services. Enqueue owns bounded99-byte text/sender/team, without a Human or input
buffer borrower. Request3 reads the local peer at production, serializes32/32/1
and NUL text, releases its payload BEFORE0072BF90, and removes the node only after
finish. Authority updates real recordA8 under418 with multimedia time BEFORE
reading text, then transfers an owned message to broadcast4. That FIFO retains
1000ms preflush/envelope order and likewise releases the message before finish.
Failed finish does not resurrect a consumed payload or report successful send.

The original producer leaves payload108 unwritten. Both queues forward it and
the presentation receiver reads/discards it. Source initializes this opaque word
to0; it is not a fabricated target or routing ID. Received values remain opaque
through forwarding. Presentation preserves sender/team/dead-vs-alive filtering,
418 scope, <TEAM> prefix, ADMIN handling and actual byte-font UTF8 masking from
0064D6C0. Text reaches the existing colored HUD message owner.005A9B30 genuinely
returns in offline7; its nonoffline Human text/timer producer still fails honestly
when required. No network/peer UI or nonoffline completion is claimed.

Focused native ASan/UBSan/leak verification:159 checks for actual loaded Tutorial
route lookup, Look action field/dirty gate, 12-frame translated-parent prone pose,
live Script link/bind/unbind, actual skip branch and actual subtitle geometry;
78 additional checks execute the real Ladder step worker (clear exit selects14,
blocked exit rejects/restores query) and real chat FIFOs/header/string codecs/
authority/presentation helper, transfer/free-before-finish, team/alive/admin gates,
clipping, UTF8 byte policy and failed-finalization ownership. Geometry, engine
rotation, clocks and final HUD endpoints are explicit doubles. Production review
launcher/probe rebuild and stage-only receipt:
analysis/checkpoints/p22-frame-gameplay-controls-20261117.json.

Pause still reaches unimplemented application transition14. The retained in-game
menu/resume flow belongs to plan section5 and needs its actual owner graph; do
not swallow Escape or reset status as a substitute. The archived selector-producer
failure also remains separate required OWN/UI work. Neither optional issue blocks
staging these focused gameplay fixes. No whole FRAME/HUM/OWN, stable-play,
function-ledger or matrix promotion; new desktop acceptance remains pending.


Returned controls build: subtitle overlap and still-blocked climbing

User's c35be793 run (2429e19 source) exits cleanly but returns three failures:
subtitle captions visibly overlap in the lower black bar, upward Ladder exit
still jitters and now remains stuck after releasing input, and standalone climb
plays an unrelated reaction-like animation. No acceptance for these behaviors.
Exact exe/logs/screenshot archived at
WORK/gameplay-subtitles-climb-return/manifest.json. Preserve the preceding
4efed3f5 accepted camera/ordinary movement/spawn/equipment observations separately.

The earlier climb correction stopped one decoder too soon. Original00586200
common1F constructs command kind4/subtype1E and005A45F0 forwards through005ABD90.
Kind4 calls005ABE30, not005ABB50/0077F050.005ABE30 subtype1E gates offline/player/
authority then calls0077EC00(primary6A,changed1). The real0077DE30 treats6A as a
probe, selects descriptor10, invokes0074A690 clearance and keeps only the actual
selected8..14 step. Thus neither movement30 nor action30 is a valid translation.
Source now binds this actual decoder arm in request_human_step_motion through
SAME inline98 DD0/E0 and existing publish callback. No guessed animation resource,
forced collision success or primary descriptor bypass. Saved index before lookup
and static native decoder disassembly, with the already indexed C export reused.
This supersedes the earlier dirty2/action48 claim, whose native audit verified the
wrong endpoint. The previous237 checks did not establish this decoder mapping.

The earlier subtitle/full-text viewport integration policy also exceeded the
retail evidence. Source already replaces renderer record0 for each new subtitle;
focused checks confirm one record through successive captions and timer expiry.
There is no accumulating logical subtitle list. The new geometry put text in the
lower black bar outside the Scene viewport, consistent with old caption pixels
persisting between world redraws. Actual graphics/black-bar clearing has not been
agent-executed, so pixel persistence is an inference from the screenshot and
viewport path, not a new native renderer-body claim. Restore native0048EE50 and
006C9260 consumers to actual process.full_viewport, retaining Script824 policy
ONLY for cutscene HUD suppression and the separate0069A560 skip consumer.
At1600x1200 ordinary original bounds are bottom1020/top888; at800x600510/444.
The earlier claim that normal-layout subtitle height was itself wrong is retracted.
No speculative black-bar clear, fake font layer or extra caption copies added.

Ladder top remains UNRESOLVED. The earlier sphere-center correction matches
0074A690 but did not fix the user's actual exit. Focused source comparison of
00768230,0074A690,00777AD0/00779E60,00767D40,0075CD50 and shared collider ownership
has not established another concrete cause. Actual engine geometry/track progress
is needed to distinguish the rejecting call; agent execution is prohibited.
New observation-only diagnostics report changed reasons per actual Player owner:
`Human step probe` phase=clearance shows falling/attachment/already-stepping/
no-upper-surface/low-surface/upward-obstruction/forward-obstruction/headroom/
posture deferral/accepted; phase=ladder-motion distinguishes travel-obstruction,
advancing, stationary, top-exit-rejected/accepted. Each log includes actual active/
queued descriptor,988/B8/posture/9AC,HD marker presence, actor position, first
posture sphere and958 rise. Separate per-Control dedup fields prevent unchanged
per-frame logging. No new gameplay state, blocker swallowing, forced detach,
queue reset or occupant/equipment/clip ordering change. Diagnostics are not a fix
or ledger credit. Required failure paths continue to reject.

78 focused ASan/UBSan/leak checks pass actual corrected probe request/Look mask and
authority gates, actual Ladder worker descriptor14 selection and upward/forward/
headroom rejections with query restoration and returned diagnostic reason, and
actual subtitle record replacement/timer/original geometry. Collision, authority
and control leaves are explicit doubles; no actual engine or Windows run. Native
audit sources/binaries are temporary and removed after fingerprinting. Actual
REVIEW full O2/Werror launcher/probe build and stage-only evidence is in
analysis/checkpoints/p22-frame-subtitle-climb-return-20261117.json. Desktop retry
must verify subtitle/standalone climb behavior and collect the Ladder probe reason;
Ladder is not source-resolved or accepted. No new broad audit/checkbox/tally or
FRAME/HUM/OWN promotion. Pause14 and selector producer remain separate.
