# Physics object frame motion — source checkpoint 2026-11-10

The actual module2 frame facade now drives the SAME World Physics manager,
FIRST/SECOND lists, stable handles, frame userdata, tree, sweep and buckets.
This is source-only. Application loading still declines; both P2.2 checkboxes
remain open. The follow-up below supplies aggregate3 and contact-seeded tipping5 frame bodies; tree replacement/pickup notification remain required;
type6 still rejects its unresolved native registration. No successful simulation
substitute is installed. [Receipt](checkpoints/p2.2-physics-motion-20261110.json).

## Boundaries and native evidence

| Original | Supplied behavior |
| --- | --- |
|00410100 /004101A0|Authority/presentation FIRST-list frame selection, reverse captured count, signed DWORD milliseconds times FLOAT .001; virtual04 then common pose even after false|
|00410940|Event5 before FIRST→SECOND migration; captured158 or exact callback return1 deletes after migration; swap-last handle repair|
|00410230|Manual release leaves NULL hole; retract EC handle and invalidate caller/index before destruction; retain EC block through callback3|
|0040E160|Point0 angular before point ray; unobstructed motion then gravity; high bounce .4 without position advance; low hit retires with velocity intact|
|0040E3B0|Sphere2 backwards .001 ray start, actual swept sphere, bounce hit-travel/displacement ratio, slow hit zeros linear XYZ|
|0040B910|Fading4 normal point or protected cached4m floor query; floor clamp+FLOAT .0001, no-hit/slow stop sets158, bounce denominator zero→1|
|0040EDD0|Extended-ray1 adds170 to ray length then subtracts it from hit/delta; no unobstructed gravity/angular, always true with live frame; high-speed bounce publishes unchanged start|
|0040ADE0 /0040AD40|Unclamped angular damping and gravity; actual LS3DF Z/Y/X rotation then(X*Y)*Z and quaternion RotateByMatrix|
|0040B360|Normalize/double cross/restore speed, actual callback1 before family damping|
|0040AB80|Direct frame08 position, normalized quaternion, actual SceneEC parent/update24; when160&&!161 refresh actual matrices/sphere centre and buckets|
|00410420|Wake SAME dormant owner before virtual08 impulse, then callback2|
|0040B030 /0040EB10|Common magnitude sqrt(speed²*scale/130), exact-zero divisor→1; sphere magnitude15 ignores caller scale; inverse-point/angular contribution and phase164=0|

00410100/004101A0/00410230 are absent from frozen inventory and earn NO credit.
All credited additions remain partial. Engine calls/allocator/ABI adaptations
are not newly credited. Motion storage safely defines unknown fourth lanes;
only native XYZ/known quaternion claims are made. Source finite-vector guards
reject undefined inputs rather than invent successful trajectories.

Frame close restores type5 saved rotation BEFORE common collision removal,
then callback3, EC retraction/reclamation and frame release. Constructor installs
feedback after derived construction and publishes actual list/handle/EC before
callback0. Feedback arguments are event in ECX, SAME handle in EDX and captured
context on stack, confirmed from original instructions, not decompiler guesses.
Callback failures preserve cleanup and the first error. Source guards reject
callbacks removing the current active slot during motion/pose publication.

Fading4 retains the same GEffects fade record independently of Physics deletion.
Native170 is radius3C<.5;174 height is unwritten until a successful descending
ray. Source represents that height as optional and rejects an early read. Missing
floor zeros ONLY linear XYZ and advances position before deletion; angular and
fourth lanes are not zeroed. Synthetic floor uses native this-vtable low BYTE
B0 (bit40 clear) solely for the dynamic-filter branch, not a fabricated collider.

Extended-ray1's high-hit scratch residual is discarded by0040F510..0040F5A2;
do not add a second ray or advance to the hit. Its internal08/24 and bucket move
precede common0040AB80 and are supplied separately. Local-matrix translation is
used for that first descriptor update, then common pose uses visual sphere centre.

The facade uses actual007183A0/00718530 source ray/sweep owners with native0/0
masks and actual identity flags/normal. No invented self-exclusion is applied.

## Verification and remaining work

Native ASan/UBSan/leak checks passed for four motion families, inverse impulse,
mode/reverse/signed-time selection, pose-before-retirement, callback0/1/2/3/5,
dormant wake/migration/handle repair/reuse/NULL holes, callback-failure cleanup,
EC lifetime through callback3, type5 restore-before-unregister and actual type4
fade retaining a retired Physics frame. Geometry/engine leaves use explicit
doubles; unsupported3/5 simulation in the selector fixture is an explicit double.
Strict actual-host i386 instantiated World, Physics facade, both18-module passes,
actual create/impulse/tick/release with unrelated required receivers declared.
Application link and27 tracker tests passed. No Windows code was executed.
Temporary fixtures are hashed/removed in the receipt; logs/evidence retained.

Original PE vtable mapping is preserved in WORK/frame-physics-object-tick-map.json.
Motion exports/assembly are under WORK/frame-physics-{object-update-boundary,
other-update-boundary,motion-dependencies,type1-update.asm,type4-update.asm,
common-impulse.asm,sphere-impulse.asm,pose-sync.asm,handle-release.asm,
dormant-transition.asm,destruction-feedback.asm}. Read-only research for the next
work is in frame-physics-type3-workers and frame-physics-impulse-workers. See
[loading/constructor contract](item-transfer-physics-loading.md).

Continue actual type3 phases0040BF20/0040CE00/0040D6A0 and their pivot helpers,
type5 contact00408780/impulse004094D0/update004070C0 and collision interpolation
00406D10, then required Human/Dog/movement/Sight/Shot and containing application
frame integration. Type5 byte202 is genuinely unwritten in its constructor:
never assume false before a real contact seeds it. Shared blast force0043C6A0
belongs to a separate grouped Physics owner and cannot be redirected here.

## Aggregate/pivot and contact-seeded tipping follow-up

[Follow-up receipt](checkpoints/p2.2-physics-pivot-tipping-20261110.json).
Types0..5 now have concrete frame bodies in the actual Physics facade; its
external `advance_other_physics_object` requirement is removed. Required positive
pickup notification and tree model-replacement branches still throw when reached.
Type6 is still rejected by construction. Application loading remains declined.

Type3 selector0040BEE0 dispatches phase0→0040BF20,1→0040CE00,2→0040D6A0;
unknown phase genuinely returns AL1.0040BEA0/0040BEC0 delegate gravity/angular
ONLY in phase0; they are not unconditional RETs. These three wrappers are absent
from frozen inventory and receive no credit.

Phase0 tentatively publishes translated frame08/24, transforms its actual
principal endpoints, then queries static rays in forward/reverse/translation
order until hit. Reflection/event1 precedes .4 damping; speed>=.5 keeps phase0,
slower hit switches to phase1 and records actual support point/zeros angularXYZ.
At speeds in[.1,.5), upward normalY>.7 also zeros linearXYZ. No-hit advances owner
position, saves actual frame quaternion, runs angular and queries rotated primary
axis both ways. Angular collision rolls OWNER quaternion back, reflects/damps.5
and supports when speed<.1. The frame remains tentative until common pose.
Gravity runs only on the final no-hit branch.

Phase1 pivot helper0040C670 normalizes gravity-step and actual support lever,
normalizes their gravity×lever cross, then rotates by actual lever length times
gravity-step length. Original .3-offset intermediate scratch has no published
output and is not turned into a constant rotation. Real LS3DF quaternion/matrix
and vector rotation are bound. Frame quaternion/update24 precedes rotated lever
position08/update24; owner quaternion/position commit only after clear queries.
Phase1 checks BOTH old→new endpoint trajectories, FARTHER endpoint first; ties
prefer endpoint1. A hit restores quaternion then position before ONE update24,
sets phase2, normalized old endpoint1-endpoint0 axis, owner-position support and
negates axis only if transformed secondary0Y>secondary1Y. It returns FALSE even
after publishing phase2, so manager retirement happens immediately. A later
contact can wake the SAME phase2 object. Do not invent immediate phase2 ticking.

Phase2 helper0040D510 rotates around the saved axis by gravity-step length times
native130 times .25.0040D6A0 checks only the farther secondary endpoint trajectory
(tie1); clear commits frame pose, hit restores old pose and returns false.
Actual shared World exclusions006EFA70 set descriptor mask8 and reset after each
query window, including source exceptions. Native masksFFFFFFFF/0 disable dynamic
candidates; no private exclusion owner or guessed collision identity is created.

Type5 impulse004094D0 is genuine RET12, absent frozen/no credit. Its manager still
wakes and emits event2.004D0440 debug drawing is genuine RET16, no new credit or
rendering receiver is introduced.

Actual00408780 non-tree contact seeds200/202/20C/210/21C/220/224/22C and quaternion
targets1C0/1D0 from SAME actual frame1E0. Locked axis uses world-matrix row0 and
original cross sign; free axis is down×incoming (not incoming×down). Kind0/non-tree
sets oscillation angle .0172787606716156, speed12, damping.6 and phase1. Other
kinds use requested angle/speed1/damping.9, disable161 and, at strict angle>
1.2566370964050293, set160=false BEFORE actual collider removal. Common pose skips
that disabled descriptor. Active200 rejects non3; kind3/nonpickup genuinely
returns true without seeding202. Pickup204 clears BEFORE the required00407BD0
notification; source throws at that exact missing receiver. Tree kind0 leaves202
unchanged, including genuinely unwritten. Tree nonzero parent-kind9 requires the
unrecovered model/cache/proximity replacement branch. No fake tree or pickup.

004070C0 preserves signed/negative time, strict overshoot, one phase per frame,
exact progress1 direct target even with amplitude<1, strict amplitude<.1 stop
and200 clearing. Rising phases1/3 Slerp bool=true; returning2/4 bool=false. Normal
tipping uses00406D10, both static old→new endpoint rays ALWAYS run even first hit,
FFFFFFFF/200000 masks and no normal request. Blocked interpolation restores OWNER
quaternion at (progress-FLOAT .01)*amplitude, without changing progress or
immediately restoring the tentative frame. Engine Slerp bool=true here.

Three native ASan/UBSan/leak fixtures pass for three-phase query/pose/thresholds,
axis sign/signed time, original retirement result, exclusion rollback, tipping
clock/targets/flags/decay and actual PhysicsObject contact→motion/disable/lifetime.
Engine/geometry leaves are explicit doubles. Actual-host strict i386 compiles the
World/Physics facade and both18-module passes; app links and27 tracker tests pass.
Eight temporary source/binary/object files are hashed and removed. Seven new
partial entries cover the five aggregate/pivot and two tipping bodies; existing
00408780 partial expanded. Inline setters/helper arithmetic gain no standalone
credit. No complete promotion or mission/desktop acceptance.

Next: containing first-frame services still need actual Human/Dog/movement,
Sight/Shot, volumetric visibility/procedural, meaningful integrity publication
and actual GameApplicationState loading/activation. The remaining tree/pickup
branches and type6 retain explicit boundaries; do not claim general Physics or
combat delivery from this motion checkpoint.
