# Runtime Mine and Human-reference retirement — 2026-11-12

Source-only P22-HUM reaction/death/peer-retirement dependency. The selected Human
checkbox, application loading/rendering and desktop acceptance remain OPEN.
[Checkpoint](checkpoints/p2.2-human-runtime-mine-20261112.json). Reuse SAME runtime
actor registry, item catalogue, Scene/frame data, collision buckets, ShotMgr,
SOUND and module17; no parallel owner or successful missing service.

## Original owner and fields

Type2 registration005FDE10 selects factory005FDE30, ctor005FDEC0 and final
vtable00812F08. Factory allocates **0x120 aligned16**, not the older persistence
note's0x458 estimate. Constructor invokes00610070(2), zeros A0..FC descriptor and
108/10C/110 table pointers. Native104 comes from an unwritten stack byte; no
source default/reading credit for it. Native90/94/98/99/9C/100 are not constructor
defaults. Source backing values are inaccessible guards until actual producers.
The source owns the cold108/10C/110 allocation as raw bytes, frees/resets it
BEFORE common/base destruction, and exposes no unknown element interpretation.
Populated108 table/save/restore producers are not exposed or credited.
005FDF40's reached delete is source unique ownership;005FDF60 cold table-free/base
retirement is adapted. Final source cleanup retracts an acquired descriptor even
if native ordered deactivation failed; this safety fallback is not original ABI.

005FDF90 writes model name `Actor_mine` BEFORE common00610200, ignores name-call
HRESULT. Runtime004770B0 still initializes, binds SAME frameEC, assigns identity,
publishes owner and activates before the caller supplies005FE010 properties.
005FDFB0 clears99 before common activation.005FDFD0 unregisters descriptor when99,
clears99 AFTER receiver, then common deactivation. Shared registry unbinds frame
userdata before false/true retirement and releases the model after owner removal.
Source destructor also handles failed registration prefixes before grid retirement.

005FE010 writes100 material,94=0,98=false,90 definition and9C borrowed Human in
that order, before model/definition/name receivers. Definitions70/71 normalize
model forward and subtract stored FLOAT forward*.07/.08 from refreshed WORLD
position. Raw bind assembly sums (Y²+Z²)+X² extended, matching the existing shared
actor normalizer (the first two additions commute); do not use decompiler's
apparent reordered expression. Preserve FLOAT squared-length reload/sqrt and
extended inverse. Then visible=true, model update, actual scene placement with
fresh world position, update, required catalogue caption/localization LAST.
Failed caption retains property/pose/visibility/placement prefixes. 9C is typed
SceneActor borrower supplied by actual Human caller, never a DWORD or copied owner.
005FEEB0 returns bound100 as signed material and ignores its hit-frame argument.

## Frame, collision and removal

Presentation005FEE90 advances timer only with bit2; authority005FEE70 only without
bit2. SAME005FEE50 skips negative94.005FEE10 wraps DWORD addition before signed
FILD/2000 comparison; if reached, set94=-1 BEFORE005FEBB0. Negative wrap disables
future increments.005FEBB0 has **NO stack arguments** (older forced ABI was wrong).
Read model world position, add FLOAT .2 to Y and publish base30; reread world
forward and normalize into40. Native fourth position stack lane is unwritten;
source guard0 has no ABI value claim. Clear actual descriptor, set type42, frame,
mask40000, actor pointer, sphere centre and radius.4 for definition71/.15 otherwise.
Insert through SAME006EF3E0 buckets, then94=-1,98=true,99=true. No independent
physics body/scene/query geometry is constructed. Missing loaded grid fails at
actual registration. Source rejects overwriting a still registered descriptor
instead of reproducing native dangling bucket ownership on invalid rearm.

00479700 now uses SAME actual type2 Mine9C. Compare pointer identity, not id10,
then existing runtime owner removal. Preserve forward index increment after a
swap-last removal: a matching swapped-in owner can remain until another scan.
No source counter, callback repair or reverse map fixes this native quirk.
Actual GameRuntimeActors factory/frame/deactivation/peer removal now borrow this
owner; native focused registry audit uses an explicit environment adapter for its
predicate, while actual host binding is compile-verified.

Mine5C is genuine00611070 (AL0/RET28), shared with known nonHuman families; Car
contact now recognizes this actual new runtime family. Mine D8 is005FEEB0 and
D4 is005B88A0, **RET24 only**, return register unclaimed/ignored. Blast bridge now
uses its material and real empty D4. These are verified vtable leaves, not unknown
actor success fallbacks. All three absent frozen entries receive zero new credit.

## Packet effects and shared resources

005FEB50 reads two bits:0 clears98, invokes005FEF50 effect and removes SAME runtime
owner AFTER effect;1 clears98 and removes through005FEEC0 without reading binding;
2/3 do nothing. Authority005FEBA0 is genuine RET4. This is the concrete Mine leaf;
full module12 envelope/transport remains required and guarded before admission.

005FEF50 reads actual definition48 payload sound14/particle1C. The serialized
payload starts at native+04, excluding vtable: read byte offsets10/18, minimum1C,
with no serialized vtable. Shared view uses existing LE parser and signed
sound interpretation. Native type8's1C-byte record and negative sound checks are
covered. A missing/short definition fails after98 clear, before SOUND/removal.

00451A80 then00451DB0's **material=-1** branch is bound specifically for Mine:
reuse SAME ShotMgr D0 request; group6, copy full position, flags1, sound entry,
relative/sourceNULL. Leave filename/ranges/voice_group and other reused fields
untouched. Invoke real SOUND emit, ignore result, then existing00445150 with
particle/position/direction,parentNULL,orientation=true,flag=false. Actual World
consumer borrows Startup ShotMgr/particle process and SAME sound/frames. No new
sound/particle definition owners. Positive-material00451DB0 branches, including
0044FBC0 and additional material particles, remain unrecovered/unadmitted here.
No new credit for already verified00445150/module17/SOUND or delegated engine.
The C export of00451A80 has a misleading post-indirect-call stack alias; raw
451AE2..451AF4 establishes argument order. Use raw, not that decompiler tail.

## Verification and remaining dependencies

ASan/UBSan/LSan PASS actual Mine/common actor/runtime registry/dynamic buckets,
mode1/2/3/7 ×100 ×definitions70/71 =800 lifetimes. Check signed timer/wrap, exact
registration fields, late-caption and missing-grid prefixes, source9C identity,
forward removal skip, frame/node/descriptor order, native packet0/1/2, effect
failure retaining owner and shared ShotMgr sound-before-particle/preserved fields.
Frames/localization/sound/particle and grid geometry are explicit doubles; this
is not actual full World or transport. Strict full host factory/frame/bind/Mine
leaf, Human removal, effects and Car/blast composition i386 plus final launcher/
probe compile/link PASS. Windows outputs not executed. Temporary audits ignored.

[Receipt](checkpoints/p2.2-human-runtime-mine-20261112.json) adds13 client partials
and extends6:34 complete/2680 partial (2675 client+5 engine), no promotions.
Absent005FDF20/005FDFB0/005FEB50/005FEE10/50/70/90/B0/005B88A0 earn zero credit.
Runtime source APIs now exist, but normal00478B90 resource/model/pose publication,
its00478CC0 wire producer, actual placed-item ground query and module12 envelopes
remain required. Current placement/packet guards were not removed to bypass them.
Save/restore005FE440/005FE800/006DF690 and Versions transitions are not delivered.
Continue these reached dependencies, full entry and actual containing Human frame/
failure/re-entry/ordered teardown. No full Human or application acceptance claim.

## Normal producers and module12 envelopes — 2026-11-12

This checkpoint supersedes the producer/envelope gaps above. Source-only receipt:
[normal Mine producers](checkpoints/p2.2-human-mine-producers-20261112.json).

00478B90 uses Driver factory9 BEFORE catalogue lookup; native definition34 is
`resources[2]`, passed verbatim to004290A0 flags0, with no model suffix. A nonzero
loader status releases the factory reference and returns with no actor/inventory/
wire. Direct modelB0/E0 position-lock-aware writes precede model1C(direction,0),
model24, SAME runtime create2/-1 and activate, then005FE010. Release the factory
reference BEFORE Human inventory lookup005A78F0(definition,NULL). A missing entry
returns with the created Mine retained and suppresses wire publication. Otherwise
005A7B70 performs its own identity-based lookup/removal and Items destruction;
keep that second search and native duplicate-identity/holder priority. Reread
process mode after inventory, then optional00478CC0 unless(mode&7)==7. A NULL
Human with non7/publishtrue fails at the reached identity dereference. Created
owners remain registry-owned across bind/inventory/wire errors; no atomic rollback.
Wire uses the ORIGINAL supplied pose, preceding005FE010's70/71 model offsets.

00477660's flag1D/kind10 branch now calls SAME World006F7C90 with startY+.5
(FLOAT0080F518=0x3F000000), displacement(0,-2,0), masks0/0 and normal requested.
A hit supplies its actual position/normal; a genuine miss uses original placement
and(0,1,0). Missing tree/grid/resources remain required failures. It calls the
normal creator(NULL,definition,pose,normal,false), then actual007DC8A0 item removal.
A failed asset load still removes that entry before the containing mandatory-load
error: no successful missing-mine loading claim. The shared placed array retires
only after its loop, retaining native prefix state if a required step fails.

004787E0 begins SAME0072BEB0 module12 broadcast, priority0 fortrue/3 forfalse,
then writes selector1 and subkind with TWO bits each. Priority3 uses the third
stream with stored/clamped priority2.00478CC0 uses subkind0/true, writes runtime/
Human/definition16 each, primary quantizer(-1) with AXIS MASK7, then secondary
direction(-1), then00478880→0072BF90 finish. No new stream/transport owner.

004788A0 reads10-bit runtime token, backward identity lookup, returnsfalse if
missing without discarding payload; an existing Mine authority20 is genuine
005FEBA0 RET4.004788E0 reads2-bit selector:0 reads10-bit token and dispatches an
existing actor1C, missing stilltrue;1 reads2-bit subkind, only0 reads three SIGNED
16-bit IDs then default quantized position/direction and invokes00478D30. Unknown
selectors/subkinds consume no further data and returntrue. Other existing runtime
actor packet leaves remain REQUIRED errors, not invented empty handlers.

00478D30 in modebit1 only returns backward runtime identity lookup, before any
model/definition/Scene2 access. Other modes use the same factory/load/direct pose;
nonnegative Human identity resolves actual Scene2 before runtime create2 with the
supplied identity.005FE010 and factory-reference release precede optional Human
inventory consumption; a missing inventory match retains/returns the actor. No
wire is sent here. The actual inventory adapter validates its containing live
Human owner before dereference, at the reached native inventory stage.

Evidence WORK `human-runtime-mine-{producers-abi,create-publish.asm,envelopes-abi,
envelopes.asm,recreate.asm}`. Index used before exports. The first envelope export
stopped at absent Ghidra004788A0; rollback ABI export explicitly creates this
function from its raw entry. Absent FROZEN004788A0 earns ZERO credit. C exports of
00478B90/00478D30 still contain indirect-driver-call stack aliases: raw RET20 and
bounded instructions establish the arguments/order, not misleading C parameter
names. No original binaries or Windows outputs executed.

Native ASan/UBSan/LSan PASS prior800 Mine lifetimes plus400 normal producers with
actual Mine/common Actor/runtime registry and actual session streams/quantizers/
envelope/Mine-removal leaves. Checks model release before inventory, retained
owner on inventory/wire errors, missing-load prefix, post-inventory mode reread,
NULL source publication failure, default profiles/axis mask/signed16/cursor,
unknown/missing tokens, authority lookup bypass, and ground hit/miss/failure args.
Frames, inventory/Scene2 and ray endpoint are EXPLICIT doubles; no native full
World/geometry/transport equivalence claim. Strict final actual World factory,
Human inventory/model retirement, ray, module facade and Mine/packet composition
plus final launcher/probe compile/link PASS. Temporary audits remain ignored.
Adds5 client partials, extends00477660;34 complete/2685 partial(2680 client+5
engine), no promotions. Full Human entry/frame/failure/re-entry/ordered teardown,
Versions transitions, save/replay and actual application/desktop acceptance remain.

The subsequent [Versions transition checkpoint](version-damage-transitions.md)
now supplies positive Car/blast frameEC5 contacts. Continue actual armed Mine
contact005FEFB0 and explosion00451990 producer; full Human entry/frame remains open.

Armed contact005FEFB0, disarm005FEEE0, actual moving-Human first-candidate query
and shared00451990 explosion now have a [source checkpoint](human-mine-contact-source.md).
Positive Car/other derived blast damage and full entry/frame remain required.
