# Car loading runtime — source checkpoint, 2026-11-10

The cold Car physics and finalization subsystem is reconstructed and verified in
source. **Car actor activation is still unbound; the application still declines
mission loading.** This is neither a loaded-world nor a desktop result. The full
first P2.2 implementation checkbox remains open. See [geometry](car-geometry-loading.md)
for the earlier model/parameter/callback owners and the
[checkpoint receipt](checkpoints/p2.2-car-cold-runtime-20261110.json) for validation.

## Reconstructed boundary

| Original entries | Source owner / implemented behavior |
| --- | --- |
|004EA780,004F5620,004F5840,004EC5C0|car_collision_loading.h, car_wheel_runtime.h, car_skid_models.h: actual pose/descriptor/SPECCOLL registration, wheel initialization,50-per-wheel retained skid pairs and cleanup subsets. Full initializer/retirement composition remains. |
|0051BD80,00517370,00517CF0,005181C0,0051C180|car_sound_loading.h:81 actual optional sound slots, filenames/flags/ranges/loop/head, shared receiver order, detach/release/null order. |
|004DB060,004DAB60,004DAC80,004DAFE0|car_navigation_loading.h: SAME navigation records, duplicate origin, loading/moving marks and cleanup; other counter-mark producer excluded. |
|004E0EA0,004E0DC0|car_ancillary_model.h: reset/reparse retained DM metadata; required missing model rejects. Live DM sound/effect handles remain separate. |
|00465F00,00466110|car_seat_camera.h: constructor and kind4 smoothing/sweep, actual0089B0F8 process scalar, separate from MissionCamera258. |
|0050D2C0,0050D5C0,0050D760|car_drivetrain.h: curve, force and clutch/engine speed; required audible shift receiver remains explicit. |
|0050DE60,00512390|car_body_integration.h, car_wheel_traction.h, car_cold_wheel_forces.h, car_body_forces.h, car_cold_wheel_contacts.h, car_suspension_impact.h, car_cold_solver.h: actual engine-off forces, prediction, F6C&800 queries, tyres/material/contact filters, hard stops, stabilization and histories. Powered forces and alternate timed wheel cache excluded. |
|00514D70|car_body_contacts.h and car_body_collision_controller.h: BODYCOL roots/points, ordering, referenced frames, shared exclusions, original callbacks/materials, impulses/friction/penetration and pose retry. |
|006F06F0,006F0050,006F02C0,006F04F0|collision_box_query.h: original dynamic/static broad candidate filters on the SAME tree/grid/buckets; point sphere sweeps still determine contacts. |
|0050DB90,00509AA0,004F67B0|car_substeps.h, car_settling_controller.h, car_settling.h: original substeps, cold active/sleep/held controller and timeGetTime settling with mandatory real finalizer. Running/occupied/shifted inputs remain outside this entry. |
|0050B180|car_cold_finalizer.h: scene placement, cold audio, periodic light sample, doors/windows, skid retirement, bucket refresh, road displacements, visual steering/spin/lights/axes/strips, cold particle retirement and final flags/result. |
|00507400,00507A40,00501CB0,00506700|car_steering.h, car_wheel_rotation.h, car_loading_lights.h, car_strip_update.h: original visual state; active lighting commands excluded. |
|00501750,00501AB0|car_door_update.h: reverse seat/door order, native endpoint inequalities, quaternion normalization, window offset/visibility and detached-glass cancellation. |
|0051A6C0,0051A7A0,00519A60,00519DC0,0051A130,0051B660,0051A580|car_sound_updates.h: same sound slots, signalling, original rate/fade calculations, puncture/skid/door/air-brake gates. |
|0069E9D0|scene_light_sample.h: actual cached scene lights, point/spot attenuation, ambient/directional contributions, shadow rays,4C/4D surface exceptions and saturation. |
|004FEEA0,004F6080,004F61E0|car_health_reset.h, car_light_receivers.h: wheel/body/link health, paired projector texture/power, actual shared receiver registration/detach ordering. |
|004FE610,004ECFC0,00506F00,00508350,00508EE0,00509540|car_skid_models.h, car_particles.h, car_cold_particles.h: exact current-pair retirement and pending emitter byte; cold damage/wheel/exhaust/impact gates and slot retirement. Positive emitter production explicitly rejects. |

No helper receives separate credit for inlined fragments, CRT/engine calls or
forwarding wrappers. The54 newly credited entries are all in the frozen client
inventory and remain **partial**. Total:34 complete /1,783 partial
(1,778 client/five engine) /18,783 frozen; zero complete promotions.

## Behavioral evidence retained

WORK is the ignored durable `analysis/work/mission-world-20261103`. All original
exports retain the client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Relevant exports/assembly:

- Initializer: `car-runtime-typed-v4-required/004ea780.c`,
  `car-runtime-initialize.asm`. SceneEC is THREE args at004EAA07/0050B214;
  004290A0 is THISCALL/six stack arguments/RET24. Earlier exports distorted the
  skid pool loop. Factory absence skips an attempt; both model loads run even
  when the first fails; load failure stops all remaining allocation.
- Core: `car-runtime-typed-v2-required/0050de60.c`,
  `car-wheel-query-typed-required/00512390.c`, `car-solver-core.asm`,
  `car-wheel-contact-core.asm`. Default F6C and F70 are8000DFF9. Wheel queries
  use534/538 =F02/0; BODYCOL uses53C/540 =2/0. They are separate masks.
  Projected radius8DC is initially an applied physics value, not a ray result.
  Contact rejection preserves previous identity/flags in the native order.
- Steering: `car-steering-body-query-required/00507400.c`, `car-steering.asm`.
  Original sign branches survive Ghidra's incorrect simplification. Side
  threshold is DOUBLE+/-.34. Low-speed stabilization threshold is3. Native
  force torque uses force CROSS offset. Unproduced contact normals stay absent.
- BODYCOL: `car-body-contact-typed-required/00514d70.c`,
  `car-body-contact-core.asm`.00718530 is STDCALL/RET28;006F06F0 is
  THISCALL/RET32 and its007185E0 wrapper has distinct ownership. Actor callbacks
  are FASTCALL;6A8 has four total args,68C three,688 ten,680 six,690 nine,684 five.
  Sorted ascending processing preserves encounter order on equal keys.
  Contact offset uses the START point minus radius*normal. Cached COM/inertia
  precede the post-query pose refresh. Retry uses post-contact velocities and
  the saved pose; correction is applied only after a previous contact.
- Box filters: `car-box-query-filters-required/`,
  `car-box-query-shapes-required/`, `car-box-query.asm`. Original query+68 is a
  SQUARED corner distance added directly to dynamic/static sphere radii. Dynamic
  broad filters inspect layers only. Static shapes use strict contact bounds;
  dynamic41 accepts touching. Cell-Y uses only cell+08 with a strict comparison.
- Finalizer: `car-finalizer-cold-audio-typed-required/0050b180.c`,
  `car-finalizer.asm`, `car-strip-finalization-typed-required/`.
  State22C0 clears on real finalization. Steering-wheel frame1C at0050CF40 has
  a FLOAT roll: current/max steering angle times applied504. Strip XYZ edits
  preserve other vertex components except the native U scroll.
- Sound: corrected virtual78/30/8C/98 signatures recover rate/fade math lost in
  the old decompilation.0083BEC4 is FLOAT1 and all recorded xrefs are reads.
  Light receiver0089C0A4 and sound receiver0089C0B4 are distinct process vectors;
  the light vector has static setup/cleanup and consumers but no runtime producer
  in the recorded xrefs. Original unregister detaches duplicate light bindings.
- `car-finalizer-environment-required/0069e9d0.c`,
  `car-environment-query.asm`: live cached light fields1A0/224/230, not serialized
  or Human colour snapshots; shadow rays use the actual masks and world tree.
  DOUBLE0080FB00 is1, despite its low FLOAT word being0.
- Road displacement consumes three process rand draws per resampled contacted
  wheel, even with zero amplitude.007E6368 sets control word173F/round-down and
  executes FRNDINT through007EDB99; the phase uses floor of a DOUBLE input.
  Zero randomized amplitude with a positive minimum selects the negative half.
- `car-skid-retirement.asm`:004FE610 clears only the current active byte and
  advances/wraps218. It issues no frame hide/release.218 is now a typed integer
  cursor.004ECFC0 only sets pending; active/visual ownership stays intact.

## Verification and limits

Native ASan/UBSan/LSan checks pass. The combined fixture uses nine captured
coefficient blocks,64 wheels, SAME source tyre/material/query/tree/grid/bucket
owners and the actual cold update, settling loop, BODYCOL controller and cold
finalizer. It also covers ordinary sleep/held updates, health/road random order,
tracked/steering-wheel publication,41 finalizer failure prefixes with exclusion
cleanup/retry, and the existing76 wheel-query endpoint failures. Geometry,
frame/math/clock and actor notification boundaries are doubled. The composed
fixture uses a default material registry and doubled geometry; it is not a
full captured Car/model/world activation or rendering test.

Separate focused fixtures pass actual sphere-sweep/body response, penetration,
material friction, actor impulse/filter/script cases,21 frame/math failure
prefixes, door/window endpoints, all light families/shadows/glass, nine sound
parameter sets and rate/voice gates, and skid retirement. Earlier collision,
audio, navigation, DM reset, camera, wheel and skid-pool checks remain valid;
unchanged broad audits were not repeated. Strict i386 actual-host composition
and application compile/link pass. Windows binaries/DLLs/original game/Proton
were not executed. Temporary audit artifacts are hashed in the receipt and
removed; research exports, captures and logs remain.

Next: finish actual004EA780 initialization/004EC5C0 retirement composition,
004FE910(1) part reset and004FE850 glass texture ownership, actual Car actor
004D16A0/004D25C0/004D2790/004D3110 dispatch and DM/session callbacks. The real
mode7 presentation update DOES call00509AA0, so BODYCOL cannot be bypassed.
Retain explicit active-particle/skid/engine/debug boundaries. Then continue
containing Human/session/VM/FPV/HUD/SOUND/module owners, ordered006CA780 loading,
first camera/update/render, failure/repeat and desktop acceptance under the
same full checkbox. No new broad audit is needed before implementation.
