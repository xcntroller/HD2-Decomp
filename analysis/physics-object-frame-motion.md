# Physics object frame motion — source checkpoint 2026-11-10

The actual module2 frame facade now drives the SAME World Physics manager,
FIRST/SECOND lists, stable handles, frame userdata, tree, sweep and buckets.
This is source-only. Application loading still declines; both P2.2 checkboxes
remain open. Types3/5 positive updates and type5 tipping/impulse remain required;
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
