# Animation and SupplyBox loading — source, 2026-11-09

Full P2.2 remains open. This checkpoint supplies the captured Tutorial's five
Animation and seven SupplyBox loading/first-update dependencies. The actual
GameWorldActors composition still is not constructed by GameApplicationState,
which declines loading. No original game/DLL or Windows fixture was executed;
no rendered world or desktop acceptance is claimed.

## Animation

Vtable008119B8:3C→005B3BE0,40→005B5000,14→005B5490(thunk to005B5190),
18→005B54A0,D8→005B5F20. Original005B3BE0 resets A0/A3/D4/B8/BC, sets
actual localized B0 or empty caption, calls common00610240, and skips derived
work on authority with mode2. The presentation pass finds `particle`, rebuilds
existing BODYCOLB descriptors in the SAME dynamic buckets, owns a real driver9
helper, and loads name0 with extension removed through category1/flags10.

005B4610 callbacks are installed on the helper and direct children backward.
They differ from Human markers: event1000 with `$filename near far` or `@entry`
emits relative to the callback frame, source null, flags1; @ uses group11.
GameAnimationActorEvents bridges actual frame+28 calls, groups registrations by
helper, retracts every binding before helper release and latches callback errors.
Position/quaternion/FULL local scale are copied; helper links original parent,
gets `<actual frame name>_aanim`, hides and updates. Actual subtree enumeration
uses005B45B0→00611090/005B5F20 and the SAME CarDefinitionTables material.word28:
case-sensitive first `matid`, atoi suffix, fallback11, >1500 setsD8=1/stop,
>505 setsD8=2/stop. No hit preserves D8. Exact lowercase resource `_mchescape`
with actual wing frames rejects the missing ancillary owner.

005B5000 calls common first; same authority/mode2 skip; clears A0/A1.
005B4790 retires collision registrations, event group and helper parent/model,
then shows/updates original and clearsD4. Engaged null factory results retire
before retry. Interaction98, starting/advancing tracks and escape wings remain
explicit failures. Captured cold005B5190 has inactive tracks; actual A2 refreshes
colliders. Exactly one of five captured records has A2.005B54A0 preserves the
mode2 authority skip; the presentation thunk receives no credit.

## SupplyBox and shared owners

Vtable00812D10:3C→005FAC80,40→005FB220,14→005FB380,18→005FBD20.
005FAC80's once guard precedes common activation. It refreshes actual005F98E0
geometry, resets scalar/animation state and preserves parent through common
activation. Real driver9 `cratedummy` loads flags10, driver6 dummy and driver9
`parachute` (verbatim category1/004290A0 key, flags0) are owned. Required cache3
clips are `_cratefly`, `_crateopen`, `_crateclosed`, `_cratedrop`; missing assets
cannot report successful loading. The two unused native clip slots stay empty.

005F9A60 finds shared exact `ammobox` definition and actual `morph` OBJ_ visual.
Actor-owned214 is registered in the SAME ordered MissionVertexInstances registry
as file-owned instances. There is no second update pool or definition table.
World constructor requires that registry; StartupSession exposes actual module9
definitions. The original persistent classification clock0089B398 remains an
explicit reference in the registry, never reset by SupplyBox or mission cleanup.
00473790 binds,004737C0 resets pose, then finished=true/ping-pong=true,
maximumLOD=definition LOD count−1, duration130/start0/repeat=false. Genuine absent
definition leaves an unbound default instance and no registration.004746B0(false)
retracts actor borrowers by first-match swap/pop; file-owned entries keep their
own allocation lifetime. Retire actors before registry/definition/frame owners.

Actual descriptor flags40/mask800 points to stable actor-owned normalized world
and inverse matrices, bounds scaled by LOCAL axes, sphere centre at frame WORLD
origin. Radius is horizontal X/Z half-extent norm times native DOUBLE
1.2000000476837158.005FD760 stores its x87 result to FLOAT before all14 authored
ammo visibility comparisons; final13 has the original >13/<14 special case.
Capacity0 rejects the native undefined input. Initial properties have full ammo.

005FBD90 initial flags=property120|18. All seven captured records have120=1,
so initial state is active1, real descriptor insertion/move and immediate
recursive fade-in. Shared EffectsFrameFades implements00489060/00488EE0's
immediate branch and004890F0's fade-out recursion, children backward and native
visual-code filter. Immediate fade-in sets actual frame+A8 alpha1 and optionally
shows; it allocates/cancels no opposite record. Positive fade-in/210 queue is
explicitly unsupported. Existing direct fade-out200 records remain shared.
Correct GEffects vtable base is008101F0:64→00488D60,68→00488EE0,
6C→00489060,70→004890F0.00486E90 at60 is a separate camera-amount wrapper.

Native005FCD80(6)→005FC860(1) starts `_crateopen` on the actual original model,
slot0/weight1/flags1, marks vertex finished, sets max(signed clip duration,1000)
timer and emits SOUND group1/entry10/flags1 at actual frame world position.
005FB380/005FCA60 cold states6→0 subtract the wrapping timer and tick actual
model+20 via005FC9A0.005FBD20 skips authority/mode2. Falling/respawn/pickup,
positive fade-in and crossfade are distinct required later receivers and reject
when reached. Serialized item lists are definition IDs; loading creates no
invented runtime item owners.

005FB220 common-first cleanup marks/reset vertex geometry, hides crate, unlinks
parachute/original preserving world, runs stop state5 (closed clip/tick0/open
clip/tick0/remove), updates1, unregisters actor vertex before freeing, releases
helpers/clips, restores initial flags19 then removes/frees descriptor. Source
failure guards omit only genuinely unacquired resources and collect cleanup
errors, including partial activation and destruction without explicit retirement.

## Concrete vertex-layout correction

New004737C0 exposed an existing00473960 destination error. Original assembly
reads definition normal words4/5/6 (padded position), but writes packed mesh
words3/4/5, preserving mesh words6/7. Both reset and existing interpolation now
map these separately; they do not copy definition padding into mesh normals.
004738C3..00473922 and00473B9F..00473C0F are the decisive writes. X sums remain
extended; Y/Z products retain FLOAT stores. This is a targeted correction from
new contradictory evidence, not a repeat audit of unrelated verified code.

## Verification and evidence

[Receipt](checkpoints/p2.2-animation-supply-20261109.json): Animation native
ASan/UBSan/LSan passes five captured records, actual descriptor/helper/event/
material/retirement/repeat and41 frame failure prefixes. SupplyBox native
ASan/UBSan/LSan passes seven captured records with actual captured Vertanim.def,
shared source vertex registry/buckets/fades, first update, repeat,150 frame
failure prefixes, sound/null-model failure, genuine absent definition, packed
reset/interpolation writes and untouched mesh words, borrowed swap/pop and
fade recursion/opposite record retention. Engine geometry/model/math/localized/
audio services are boundary doubles. Strict actual-host i386 composition and
app compile/link pass; Windows binaries were not run.27 tally tests/check pass.
Temporary fixture source/binaries/objects/generator are hashed and removed.

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored WORK=analysis/work/mission-world-20261103. Original exports:
animation-loading-required/,animation-first-update-required/,
actor-activation/005b3be0,actor-cold-subowners/005b4790,
supply-box-loading-required/,supply-box-state-required/,
supply-box-initial-commands/,supply-shared-owners-required/004746b0,
physics-effects-fade-lifetime/,vertex-instance-geometry-update/,
vertex-instance-interpolation.asm,supply-box-collider-setup.asm.
Actual property summaries:animation-tutorial-properties.json and
supply-box-tutorial-properties.json. Captured actors.bin21064bytes SHA256
b98bd7f6e73047fa2dc25ef3725cb8ba13212d6e41f2f0a3aa3e822057d6453d;
base Vertanim.def SHA25615f2a3ba1a3c19452c88a47f7ddbd9d795102570e7f8e044a86475be5c405bfe.
Twenty implemented frozen bodies receive partial credit only; absent inventory
bodies, thunk and engine delegates receive none. No complete promotions.
Continue StaticWeapon/Car, containing Human/session/VM/FPV/HUD/SOUND owners,
ordered loader, first update/render/failure/repeat and user acceptance. The full
P2.2 checkbox is unchanged.
