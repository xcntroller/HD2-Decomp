# H8 projectile frames — source checkpoint, 2026-11-17

H8.3 remains open. This checkpoint binds the all-hit collision query, shell
motion, grenade authority/presentation/contact, material projection effects and
picked-part Versions damage on the SAME World. Bullet/capsule combat and visual
workers remain required; no application loading or desktop acceptance is claimed.
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
allocator, class, SEH or register ABI. H8.2/H8.4 dependencies, bullet/capsule
workers, containing verification and actual application/desktop gates stay open.
