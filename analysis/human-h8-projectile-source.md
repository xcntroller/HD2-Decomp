Current: H8.3 ordinary offline source is complete; [closure and external H8.4/acceptance limits](human-h8-projectile-closure.md). Earlier open-state statements below describe preceding checkpoints.

# H8 projectile frames — source checkpoint, 2026-11-17

H8.3 remains open pending the reached derived-actor/containing dependencies.
The SAME World now binds all-hit collision, bullet/capsule combat and visuals,
shell/grenade frames, impact effects, Car picking and picked-part Versions damage.
No application loading or desktop acceptance is claimed.
[Receipt](checkpoints/p2.2-human-h8-projectile-20261117.json).

## Collision identities and ordering

006FF140 now borrows the loaded tree and actual dynamic buckets. It traverses
dynamic Z/X buckets first, then the existing directional static grid traversal;
extracting that traversal leaves the closest-ray consumer unchanged. Both paths
retain their own masks: layers OR8; static flags OR20; dynamic does not inspect
the static flags mask. Hit records borrow tree kind/index or descriptor identity,
carry the native parameter and normalize their own normal after insertion.
Insertion scans through equal parameters and checks identity before the first
greater parameter. A repeated closest query is not used to approximate this list.

00700BC0 retains triangle flags, barycentric cases1–7, endpoint1, original FLOAT
dot spill and no sorted-X early termination.00701350 retains box/rotated-box,
sphere82, bounded rectangle83 and radial84 branches;85–87 have no positive native
branch. Dynamic43/default likewise have no native positive branch. Dynamic
primitive normals and static normals deliberately have different signs.

Raw bytes settle misleading decompiler constants: DOUBLE0080FAC8 is
9.99999993922529e-09, not .0001. Rectangle83 uses a strict greater-than gate;
triangle two-sided and radial84 gates admit equality. Shape83 admits maximum X/Z
and endpoint1;84 has no Y extent check. The static dominant-axis all-hit traversal
has no vertical alternate: exactly zero X/Z displacement gives an empty secondary
span after invalid FTOL, while dynamic results survive. This is an explicit raw
control-flow inference, not a successful synthetic vertical ray. Nonfinite source
coordinates fail rather than invoke undefined integer conversions. Native unused
vector lanes/record padding remain unclaimed.

## Shell and grenade frames

00454360 uses007183A0 layers1/flags0, with source Human query exclusion/restoration
around the SAME World ray. It is not a sphere sweep. Native displacement spills,
strict signed age>2000, state3 retirement, coincident-point stop and .1 speed gate
are retained.00454660 sums dot Z/Y/X; reflection coefficient0080FAD0 is1.2.
00454680 NEGATES velocity after a negative normal dot; it does not normalize it.
The local SOUND request uses the old position before contact publication, group5,
flags1, command30 and voice group2. Actual visual positioning/update precedes the
velocity reload and3.8 gravity step. Native local5C is explicitly0, resolving the
C export's apparently unproduced ordinary return. Padding W is not credited.

0044D260 returns state1 early; otherwise it calls0044D290 only without mode2 and
returns the current state. Absent-frozen leaf0044D240 calls the common worker only
while state!=1. Offline7 therefore advances the grenade Body from presentation.
The existing actual rigid Body updates BEFORE reading state/timers, with source
query exclusion/restoration. Body close precedes blast/effect generation.
Contact-state2 offsets authority blast by normal*.5, with presentation at the
model position/material−1. Timed explosion uses strict positive fuse<elapsed,
then material690; age>=15001 retires. Blast uses the existing bound explosion
owner and actual payload native60/5C/58, not a second wave simulator.

0044D570 preserves callback kinds/return values0/1/2/3, offline nonplayer kind1
source grace through signed age200, self contact, static materials77/76, native
UNSIGNED material2C threshold/FFFFFFFF sentinel and discarded actor D8 call.
Versions-backed descriptors use actual detailed scene geometry picking and
004511E0's case-sensitive matid parser. Only authority damages the picked Versions
part; material62 uses10000, otherwise native truncation of power. Fuse0 publishes
state2 before returning3. Other authority Human contacts call D4 kind5/power1/hit0
before presentation sound.0044D4C0 requires signed698<50 and positive694, emits a
fresh local group5/flags1 SOUND request at the current model position, then sets
698=2000. No invented countdown is added.

0043C660 is bound through the existing Versions contact link: validate the SAME
Master against frame EC+14, then use its actual Part EC+18. It damages one part,
not every part in the Master. Missing/null/mismatched metadata returnsfalse;
stale nonzero Part metadata fails honestly. The frame output remains a local.

## Effects and ownership

00451A80 still publishes the actual reused ShotMgr D0 SOUND request before any
effect, preserving untouched fields.00451DB0 now also supplies its positive
material branch: reverse-normal0044FBC0 projection, base particle, then native
material4C particle, using existing module17. The earlier scoped Mine API remains
unchanged.

0044FBC0 queries static geometry with layersFFFFFFFF/flagsB90000, direction*.5
from point−direction*.3. Quality/query/missing texture gates consume no random
draws. Native80 now owns actual projection records, replacing its opaque pointer
placeholder. Exact maximum equality recycles the oldest record BEFORE the two
engine RandFloat draws. New records set mode3/enabled/projection once. Reuse
retains that actual frame; scale(.5*(draw+1),same,.1), reverse hit normal, random
roll, point+hitNormal*.05, fresh texture lookup, actual scene light and explicit
scene E0(position,1) sector selection precede update/publication. Light has no
invented minimum. These records have no per-frame tick.0044E110 retires them in
reverse before textures;0044E4C0's selected cold-null field4 branch releases the
frame. Its nonnull COW branch is not claimed. Portable RAII retracts failed new
or reused records while required receivers remain failures.

## Verification and limits

Native strict ASan/UBSan/LSan:33,408 all-hit query checks/32 loaded tree/bucket
lifetimes;9,901 shell/grenade/contact/projection checks/100 lifetimes;800 selected
Versions checks/100 actual two-part Master lifetimes. Geometry/Frames/Body/
Human/blast/SOUND/module17 endpoints are explicit doubles where named by the
receipt. The Versions fixture runs only the new selected-part API, not another
broad audit. Analytic collision cases use independent intersections. An early
fixture wrongly asserted an unused matrix W lane; the fixture was corrected to
XYZ, leaving source padding unclaimed. The final fixtures are fingerprinted and
removed. Actual combined World/factory/Active/rigid/contact/effects source and
launcher/probe are i386 compile/link only. No original or Windows fixture ran.

The first requested Ghidra batch stopped on absent0044D240 after exporting two
preceding workers; that log is retained as a failed export. Separate completed
read-only/noanalysis exports and raw wrapper bytes support the actual source.
No absent wrapper/delegated engine implementation receives credit. All new frozen
function claims are partial; source STL/typed borrowers/RAII are not original
allocator, class, SEH or register ABI. H8.2/H8.4 dependencies, reached Dog damage/Animation destruction,
containing verification and actual application/desktop gates stay open.


## Bullet/capsule follow-up — 2026-11-17

0044B910 and00449290 now query the SAME World all-hit owner, restoring source
collision exclusion before callbacks.0044A910/0044AE00 preserve the native
median/partition/final-insertion sort: equal hits above16 CAN reorder. There is
no stable-sort substitution or synthetic repeated closest query.0044B3B0 applies
range attenuation before detailed picking refines a contact.00448380 retains
speed, both normalizations, FLOAT reflection spill and the native ricochet factor.
Bullet material2C is unsigned; capsule uses signed2C, including native negative
non-sentinel values. Threshold equality, distinct pass77/kill76, source/previous
actor gates, distance600 late retirement and position publication order survive.
Capsule publishes20/40 before blast/effects and sets78 afterwards; presentation
retirement stops particle70, then hides sound8C/model and decrements timer84.
Static capsule misses retain material processing where bullet misses continue.

Human hits use actual hit zones, friendly/hostile policy, damage and miss owners.
StaticWeapon006057B0 forwards to actual first-seat Human, normalizing incoming
direction, and invokes Script hit AFTER damage. Its miss uses the weapon's base40
direction. Car damage uses existing004DBA10 via the SAME checked blast-owner link;
004DBED0→004E1530 picks the actual DM after reverse hiding EXPL_D children and
publishing real occupied-seat visibility. Missing DM is genuine−1, and negative
matid values remain distinct. Body impulses use actual frame EC word4, not text.

Picked Versions damage retains the actual replacement Frame output from
0043B000. A Part suppresses the Script/Physics alternative; ordinary non-Part
picks use actual Physics and Script context owners. Animation absent-frozen
005B5EB0 adds damage with strict greater-than threshold and shares A3 with its
frame owner. Its triggered track receiver remains required, not silently idle.
Loose-item005B74D0 preserves signed age300/item20/21 gates;005B79C0 applies the
actual Body impulse BEFORE its late non7 packet requirement. The selected offline
path is implemented; dynamic-item containing-frame age advancement remains a
separate runtime requirement. Actual table/raw proof permits RET24 only for the
confirmed common families and distinct Pike005B8980, Supply005FBD80,
Switch00613390 and ExplosivePlace0060DA70 leaves. Dog30 still needs its live owner.

0044B400/00449030 orient from interpolated motion, retaining signed100 clamping
and capsule original elapsed80. Flare particles use actual module17;004458B0
TOGGLES emission, retaining existing points and B4. It is not00445800 deletion.
The actual module/instance toggle has focused lifetime verification. Ordinary
00451670 supplies special90=false/incoming=false. The positive90 command9 branch
still fails at its actual MissionCamera requirement; no successful camera stub is
introduced.004486B0/00451E40 use the actual selected Player/head and shared SOUND,
with source/listener distinction and late6E publication. Existing attached bullet
production already calls authority100 BEFORE active-vector publication, ignoring
its return as native.

0044EBD0 uses the SAME native80 projection pool as0044FBC0. It preserves frame
kind/flags/ancestor gates, exact-maximum recycle before draws, parent link/text
capture order, explicit sectorE0(point,1), .4 light floor and white/NaN branches.
Native4C..60 are blood textures following explosion48; the prior checkpoint's
use of the decal array for explosion marks was incorrect and is corrected here.
Bullet decals use24. Parent COW text survives explosion-mark reuse; frame release
precedes text retirement.00451BC0 emits projection before material particle policy.

Verification:211,000 visual/sort,7,500 combat/flyby,900 actual emission-toggle,
901 actual Car pick,1,100 actual Versions replacement,1,400 actual shared ShotMgr
impact-pool and1,800 Animation/shared damage-link checks pass under native
ASan/UBSan/LSan. World/ray/actor/damage/engine callbacks in the policy fixtures
are explicit doubles; these are not whole-World execution. Strict actual i386
World/factory/Human/frame/bullet/capsule/derived-owner link and launcher/probe
builds are recorded in the receipt. No Windows artifact was executed. Frozen
entries receive partial source credit only; absent005B5EB0/00455650 and delegated
engine calls receive no new credit. Remaining H8 and actual application/desktop
requirements stay open.
