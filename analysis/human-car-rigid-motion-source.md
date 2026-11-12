# Detached Car rigid-body motion —2026-11-12

Source checkpoint within the open Human reactions/damage/ragdoll checkbox.
Actual004C9000 is now bound through SAME Item12/RigidBody/World/tree/buckets;
there is no stationary/lifetime-only authority substitute. Fresh parts still
fail honestly at the confirmed unproduced5F9 read described below. Wheel695,
Car event2/timed-DM/deformed mesh, full Human entry/frame and application/desktop
acceptance remain open. No original executable, DLL or Windows fixture ran.

## Evidence and ABI

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK `analysis/work/mission-world-20261103` contains the lookup receipts
`human-car-{item12-next-motion,rigid-kernel-dependencies,rigid-contact-retirement,
rigid-math-add,rigid-math-multiply,rigid-motion-more,rigid-callback,
rigid-motion-final}-index.txt`. Existing candidates were reviewed before exports.
`human-car-rigid-motion-kernels`/`dependencies` are initial default exports;
004CCCC0 failed decompilation there. Rollback-only `ExportHumanRigidMotionABI.java`
exports `human-car-rigid-motion-final-abi` successfully. Its first `motion-abi`
variant omits elapsed from the indirect body20 signature; use final/raw instead.
All temporary Ghidra signatures roll back without saving the project.

Raw `human-car-rigid-motion.asm`, `human-car-item12-substeps.asm`,
`human-car-rigid-{sleep,impulse-filter-refresh,constructor,math-add,math-multiply}.asm`
and bounded PE `human-car-rigid-motion-constants.json` resolve aliases/spills.
004C94B0/004CB2B0 take FLOAT seconds/RET4;004CC520 takes FLOAT/displacement*/RET8;
004CCCC0 takes SEVEN stack words/RET28: linear*,angular*,normal*,point*,FLOAT,
point index,other actor. The last two mean index/actor, not material IDs. Actual
body ignores elapsed/other actor.004CD4F0 is componentwise XYZ multiplication;
004CD520 is XYZ addition. Neither is subtraction.004066C0 remains pointer-scalar
multiplication.004C9480 is the small wake helper;004C8EE0 is substeps. Both are
absent from the frozen inventory and receive ZERO credit.

007E4F02 imports actual `S_plane::IntersectionLine(other,outPoint,outDirection)`:
stdcall receiver plus three pointers, BOOL byte result. Bounded LS3DF.dll export
metadata identifies RVA306C0. The new frame binding calls the actual export,
never an original client address. Frame virtual10 has receiver/position/forward/
up (RET16); virtual08 has receiver/position (RET8).00454660 returns ST0 extended,
Z/Y/X dot. In004CC520, both quaternion dot sums retain this order; the second
W*W spills before subtracting the direct ST0 return. In004CCCC0, AngleTo's ST0
feeds FSIN without a FLOAT spill. The i386 endpoint preserves that return ABI;
engine math bodies remain delegated with no new engine credit.

## SAME body and containing frame

`RigidBody` retains its existing fields, actor borrower, borrowed frame, body list
and dynamic collision descriptor. No Physics object, new registry or retained
contact frame is introduced.004C9000 calls actual004C8EE0 BEFORE its own83ms/
zero handling. Substeps obey5A0/5A4 and564<=2000; clamp83 and publish83/original
at5A8, subdivide using actual568 and FTOL(delta/568)+1, then dispatch each mode1
wheel004C94B0 or part004CB2B0. State changes during substeps do not invent an
additional early stop.

The containing frame updates speed/angular sleep accumulation, B0 movement and
stable timers. Empty-cache sleep calls actual004CF440; elongated part axes can
produce004CE7A0 toppling impulse instead of sleeping. Stable borrowed actors are
observed in reverse; movement refreshes their recorded common actor30 position
and wakes the body. Otherwise signed wrapping cache lifetime decrements and
negative entries erase in order. Below-Y velocity-85 produces state2. Actual
model24, conditional58 and SceneEC(frame,frame+30) precede004CEAA0, descriptor
position/bucket movement and5B4/5E8 clear. This is distinct from auto-parent's
world-position argument. Substep tails refresh matrices only;004CEAA0 subsequently
refreshes centre/root/basis with its native repeated frame observations.

`GameWorldActors` supplies the existing World CollisionRay, tree/resource
identity and shared exclusions.006EFA70 excludes SAME descriptor before each
query and clears before the contact callback; RAII also clears on source errors.
Part water emission and its second query remain inside ONE exclusion window.
Static flags/frame come from actual tree records; dynamic fields borrow the
actual descriptor. Actor keys are checked against SAME runtime ownership or
Scene2 identity before type/common30 dereference, including stale/null failures.
No borrowed actor becomes retained or delays native retirement.

## Integration, collision and ownership

004CC520 converts angular motion through the current inverse without translation,
constructs/multiplies actual axis quaternions, rotates forward/up and, for parts,
rotates root position around centre. It rebuilds/normalizes right/up/forward,
adds displacement then calls frame10.004CE7A0 preserves state/timer publication,
linear impulse/mass and inverse-inertia angular response order.004CCCC0 handles
incoming normal response and bounded tangential friction, records maximum5B4/
point index5E8/point400 and preserves partial writes before later engine errors.
Unproduced SIMD padding retains an initialized/prior source lane.

Wheel004C94B0 consumes actual prior598 support, rolling/contact friction, torque,
gravity, mass and inertia; saves pose before integration. Its downward support
ray and forward ray use native8002/210000 masks. Native wheel plane intersection,
width/radius contact selection, support calculation and penetration rollback/
impulse/reintegration are present. There is no guessed sphere/bounce replacement.
Actual5FC stop gate and contact callback filtering remain in their native order.

Part004CB2B0 transforms the same14 geometry points in reverse, alternates query
selection, reuses produced contact planes when eligible, and preserves native
88002/200000 then210000 water masks. Water writes580=9, damps sufficiently fast
motion, emits actual00445150 definition45/up/NULL/false/false through SAME process
and module17, then queries again. It consumes point penetration impulses in
reverse, borrows contacted actors, selects the largest allowed positional
correction, applies native water speed damping, publishes position/24 and caches.

004CE990 forwards a full typed callback event (primitive/position/normal), then
preserves bypass55/43, reject1F/20/46,5FD and dynamic actor4/25 classification.
The existing loose-item005B7630 adapter still sees the same flags/layers and
context; it now receives the complete event rather than losing its position/
normal payload. State0 callbacks keep their previous ordering.

Contact cache records own only their container: actor common30 snapshot, borrowed
actor,1000ms lifetime and point index. Duplicate actors refresh only lifetime;
append uses actual capacity growth4.004C8EA0 removes descriptor BEFORE freeing
cache/counts. Reset clears actual point actor/surface/plane-valid state, without
inventing contact/frame retention. The body list/common frame lifetime remains
with the existing Item12 and actor teardown.

## Confirmed unproduced byte and verification limits

004CB2CD..004CB2D8 reads/toggles5F9 before any motion store. Actual004C84A0 writes
5F8=true,5FB=false,5FC=false,5FD=false; it DOES NOT write5F9.004C8980 resets5EA..5F7
and writes5FA=true, also leaving5F9 unchanged. Both actual launchers/configurators
leave it unchanged. Fresh part authority therefore requires a produced prior
5F9; source fails at that first read, BEFORE gravity/movement. No random/zero/
false default is fabricated, even though slower motion writes1 later. This is
separate from confirmed unproduced wheel695. Future actual producers may preserve
prior state; none is invented. Do not repeatedly audit these omissions without
new evidence. Warm-state native checks seed an explicitly known prior byte.

Temporary `human-car-rigid-motion-native.cpp` passes ASan/UBSan/LSan100 repeated
families: free/ground wheel,83ms substeps, complete callback payload/rejection,
fresh-part failure and seeded part integration, dynamic contact cache/movement/
expiry/stale borrower, positive water/requery and failed emission/query cleanup,
normal impulse prefix before angle failure, angular impulse, interrupted pose,
shape toppling and native surface filters. It uses actual RigidBody/geometry/
dynamic buckets/exclusions; frame/engine math, ray/actor lookup and particle
endpoints are explicit doubles. It establishes source invariants, NOT original
trajectories/convergence, exact native layout/allocator/SEH or whole-body parity.
Item12 lifetime/registry regression, strict actual nested World/Human/Car/Item12/
Physics i386 and launcher/probe checks are recorded in the linked receipt.

Continue actual Car event2/wheel request serialization, deformed/global backup/
occupied restore/timed-DM, then full Human entry/frame/failure/re-entry/retirement.
The selected checkbox and P2.2 application/desktop acceptance remain OPEN.
