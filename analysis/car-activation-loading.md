# Car cold activation — source checkpoint, 2026-11-10

The actual offline, unoccupied Car activation and retirement sequence now has a
source composition and Scene2 type4 dispatch. **Application mission loading is
still disabled.** No loaded world, original-game execution or desktop verification
is claimed. Continue the full first P2.2 implementation checkbox.

This extends [cold physics/finalization](car-runtime-loading.md), reusing its SAME
geometry, solver, BODYCOL, query/tree/grid/buckets, navigation, callback, particle,
material, audio and process receiver owners. The containing world must provide
actual resources and session/Players/proximity consumers. No successful default
services were added. [Checkpoint receipt](checkpoints/p2.2-car-activation-20261110.json).

## Reconstructed sequence

`car_cold_initialization.h` composes004EA780: model update/pose/SceneEC,
colliders, reverse wheels, skids,004F3950 mass, sounds and receiver registration,
health,004FE910(1) part health,004FE850 retained glass textures, light registration
and axes.004EC5C0 retirement visits native linked wheel order separately from
axle order, restores bases/visibility/positions, retires each wheel's skid pair,
restores cold parts/lights and detaches actual receivers. Failed prefixes retract
acquired resources; no completed initializer is published on failure. Native
004F6070 is a genuine RET4, not a substituted service.

`car_loading_activation.h` composes004D16A0. Both calls reset actual DM metadata,
read current clock/settings264 and run common activation. The authority pass
registers proximity kind4 and returns before runtime initialization. Presentation
initializes once, selects special colliders, sets fuel, configures actual light
commands8808 and projector scale12/12/40, reads reverse seat metadata, hides VIEW
frames, constructs camera3RD owners, places the model in scene at Y+1, marks
navigation, attempts optional C<original resource> flags10 model, and runs actual
20ms004F67B0 settling through the recovered solver/finalizer.

**Actor activation overrides constructor layer defaults:** wheel534=98302 and
BODYCOL53C=D8002; flags538/540 remain0. The earlier F02/2 masks describe the
initializer before004D16A0 configuration. Callback6A4 is actually null cold.
The optional inline runtime keeps collision descriptor addresses stable across
reactivation, matching descriptors embedded in the native Car core; another
wheel's cached primitive identity must not point into a replaced allocation.

004D3110 clamps elapsed to83, computes steering threshold from prior934×0.9,
compares actual scene/model epochs, runs offline00509AA0 and0050B180, copies
base pose, selects004D3970 view features, updates real seat cameras/navigation
and invokes model+20.004D3970 uses extended squared camera distance Z²+X²+Y²,
actual visibility and epoch gates. Its800 versus20000 choice requires the newly
recovered00512390 alternating per-wheel cache, period/substep gates and live
primitive metadata. Front/rear plane cache mode remains excluded.

004D2790's offline cold authority path checks the actual disabled clock owner,
water, zero-timer DM and support. It does not run physics: mode7 also has bit2.
The occupied/control/exit/UI, periodic event, targeted damage and queued SOUND
producers remain explicit required receivers.004E08C0 initializes DM countdown-1,
but004E0EA0 resets it to0 **after** reparsing, so004E1990 genuinely skips timed
destruction on the first cold tick. No destruction/notification success is faked.

004D25C0 resets DM while initialized, retires optional helper, removes seats,
restores each camera again before releasing it, retires runtime under the original
mode/pass guard, runs common deactivation, clears navigation and removes authority
proximity. Partial initialization and repeated retirement preserve acquired-owner
cleanup. Reinitialization reuses actual seat metadata and stable descriptor storage.
004D9A40's unoccupied path resets seat mass, camera, seat0 engine/governor/brake
fields, empty-seat body flag and model. Its entry is absent from the frozen
inventory and earns no credit;004DA090 view handling is absent too.

## Shared fields and dependent bodies

- Native actor624/core594 is the SAME32-byte lighting list, including flags10,
  health14, type18 and part1A. Health/part detachment now mutate it directly;
  the independent contact-link copy was removed. Shared paired lights retain31.
- Callback state reads SAME runtime body/wheels/health/drive. Actor958/core8C8
  is speed; actor4A0/core410 is **velocity**; actor2D0/core240 is forward and
  actor2F0/core260 is up. Occupants are actual zeroed FA8+30 seats;24AC is the
  separate driver. Callback wheel mutations borrow actual CarWheelRuntime.
-004F3950 combines actual seat/ballast positions and zero initial masses,
  updates COM/inertia and mass-changed flag; it does not change918/gravity204.
-004FE850 retains per-glass texture0C across deactivation. Parameter names1E0/
  1E4 become native175C/176C. Secondary inline text is passed even when empty;
  graphics load flags4. Positive deformation/glass backups still reject.
-004FF8B0 reconstructs cold deformation threshold skips; positive required
  mesh/variant/glass/wheel damage rejects before mutation.00500EB0 applies actual
  per-frame damage thresholds/clamps, torque multiplier and destroyed latch.
-0051A950 implements inside/outside impact bands and800ms voice arbitration;
  0051B560 preserves double state read and volume+0.15 comparison.0051B8F0 stops
  all81 real voices via AC(0,1).004F2710 stops them on audible true→false.
  Interior impact coefficients are **C38..C44 /22AD..22B0**; C34 remains puncture
  volume.004ED270's prior source indices were corrected from original assembly.
-00466010 restores camera local position/quaternion, updates, refreshes cached
  parent/world position and sets snap. SEAT attributes accumulate digit bits;
  `name` uses atoi after the substring. None of four captured models has gun
  seat100, whose positive owner is still required.
-004D2C50 uses006FDA00's **second/max** output and -FLOATMAX sentinel, checks
  model Y below it, then queries upward100. A hit clears body health only and
  sends Human impact(10,1000,zero,zero,null,0) to occupants reverse. New006FCDA0/
  006FD340/006FD7E0 water queries use SAME tree, triangles/83 and exact masks,
  sidedness, parameter and edge gates. Existing006FDA00 is reused without new
  credit. Point/normal output modes remain excluded.
-0073EA90 uses actual actor2470/2474/2478 enabled timer and signed wrapping sum;
  expiry stores a second clock read. Cold enable remains0.004D5430 support flags
  1/2/4 and004F2670 upper-only fuel clamp have source bodies.

## Evidence and verification

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Ignored durable WORK=`analysis/work/mission-world-20261103`. Retained evidence:
`car-runtime-typed-v4-required/004ea780.c`, `actor-activation/004d16a0.c`,
`car-activation-required/`, `car-loading-runtime-required/`,
`car-initializer-mass-required/`, `car-impact-typed-required/`,
`car-actor-cold-required/`, `car-actor-cold-state-required/`,
`car-actor-cold-effects-required/`, `car-actor-dm-water-required/`,
`car-seat-retirement-required/00466010.c` and saved
`car-{mass-update,mass-vector,suspension-sound,health-damage,actor-activation,
water-queries,actor-features,parameter-application}.asm`.
The seat export job produced00466010 but later rejected undefined004DA090;
that whole Ghidra job is not recorded as a passing audit.

Focused native ASan/UBSan/LSan verification covers nine captured parameter sets,
part/glass/mass/impact behavior, alternating actual source ray/tree/tyre caches,
water planes/triangles/bounds/passenger damage, support and proximity failure
rollback. A composed actual source CarSceneActor/geometry/initializer/solver/
finalizer/seat/navigation lifetime passes both activation calls, cold authority/
presentation update, stable descriptor identities, repeat and53 distinct
activation/teardown endpoint failures. Its four-wheel authored model, engine
math/frame/resource/clock, callback and session boundaries are explicit doubles;
this is **not** activation of the four captured complete models or a live world.
Actual host Car/world dispatch is i386 compile-only; application compile/link
passes. Windows fixtures and original binaries remain unexecuted.

Twenty-four eligible client entries gain **partial** status, zero complete
promotions. Totals:34 complete /1,807 partial (1,802 client/five engine) /18,783
frozen. Inlined fragments, absent entries and delegated engine calls gain none.
Temporary fixtures are hashed/removed at this larger subsystem checkpoint;
exports, captured inputs and validation logs remain.

Continue containing Human/DD0/state, FPV/HUD/SOUND/VM/module/session owners,
ordered006CA780 loading/postactivation and first camera/update/render/failure/
repeat, then user desktop acceptance. WorldScene/WorldActors remain uncalled by
the application. Required positive running/occupied/deformation/skid/emitter,
non7 and front/rear plane-cache receivers are still explicit exclusions.
