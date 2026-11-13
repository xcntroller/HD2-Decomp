# Reached Car queued destruction and physical reset —2026-11-12

The selected Human reactions/damage/ragdoll checkbox remains OPEN. This is a
source checkpoint through the Car event4 dependency, with required positive
fragment/Item12, timed-DM, entry and containing Human frame work still pending.
Application loading remains disabled; no desktop acceptance is claimed.

## Original evidence and ABI

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK is `analysis/work/mission-world-20261103`. Research-index receipts
`human-car-reset-dependencies-index.txt`, `human-car-reset-abi-index.txt`,
`human-car-mesh-inner-index.txt`, `human-car-reset-math-index.txt`,
`human-car-queued-inner-index.txt` and `human-car-glass-fragment-index.txt`
precede the corresponding new exports or reuse. Indexed candidates are not proof.

Rollback-only `ExportHumanCarResetABI.java` produced
`human-car-physical-reset-abi/004e9db0.{c,json}` from the read-only project.
Its first attempt failed a Java import before exporting; the fixed attempt
succeeded without saving. Default C remains unsafe:004066C0 multiplies a vector
by a FLOAT through a pointer, despite the decompiler rendering subtraction.
`script-animation-bounds-half.asm` proves that body.00454660 accumulates extended
Z/Y/X dot without a FLOAT return spill; the caller's raw instructions control
arithmetic. No new credit for either helper or delegated engine math.

Bounded raw `human-car-physical-reset.asm` covers004E9DB0..004EA759;
`human-car-queued-presentation.asm` covers004DC780,004DCD10 and004DCDE0.
`human-car-mesh-inner/{004dcde0,004dd040}.{c,json}` recovers the small workers.
`human-car-glass-transition.asm` and existing
`human-car-damage-presentation-workers/00500da0.c` establish glass order.
Existing004F2530,004FE910,00507A40 and004F5840 bodies were reused with raw caller
validation; the full cold solver was not audited again.

##004E9DB0 physical reset

RET8 receives speed FLOAT and restore byte on the SAME initialized Car core;
core6E4 is model, distinct from actor20 (the embedded core starts at actor90).
No new body, wheel, coefficients, buckets or sound pool is allocated.

- Update model24, then forward every wheel. Steering wheels with1, without400,
  and parent14C linked directly to model rebuild the local matrix, axis/up
  quaternion at zero angle, base quaternion product, normalized parent quaternion
  and local cache. Publish original local20 position unless the frame80 lock
  blocks it, then parent24 even when position was locked or steering skipped.
- Copy model world/inverse/rotation caches, clear only rotation translations,
  publish initial160 from a fresh model-world observation and transform actual
  centre1E0. Publish parent544 BEFORE SceneEC(model,centre).
- Clear2293 (gear change, **not engine-running22A2**), angular210 XYZ and velocity410
  XYZ. Copy direction170, body flags600=1, skid audio5F0=0, brake ramp710=0,
  mass_changed22C0=false, speed8C8=0 and engine864/868=0.
- Nonzero speed writes forward*speed, native normalization and abs(speed).
 004F2530(speed!=0,true) runs BEFORE gear/wheel/health reset. The actual lifetime
  sets F48/22A0/22BF/22A2 and existing81-slot stop path. Nonzero speed genuinely
  publishes the running engine even when004DC780 shortly forces it off.
- Reset native gears874/878, controls/brake/clutch/steering and additional mutable
  runtime words. Preserve external270/280, travel8E0, rolling speed8CC, solver
  substep history and other fields absent from this reset. Do not reconstruct the
  entire cold initializer over live state. Additional unconsumed driving/cache
  words have typed mutable storage, not guessed producers/coefficients.
- Reverse wheels: current frame148 world40, flags&FFFFFF07; restore only sets
  puncture2284=30000, clears C0000000 and shows the wheel. Reset actual skid218,
  contact/rotation/cache scalars and only A0/B0 XYZ; normal130=(0,1,0,0).
  Anchor50 uses actual diagonal1E4 and up260, with native Z product spill; previous60
  subtracts up. Typed SIMD lanes with no defined native scratch producer retain
  a known prior lane, without pretending to recover uninitialized bytes.
- Reset desired908/current90C clutch and transmission910 low5 bits; restore calls
  actual004FEEA0 and004FE910(0). Positive deformed-mesh backup still fails at its
  per-part restore boundary. Positive glass backup is recovered below.
- Clear actual air-request228E, reverse standalone light/projector relinks with
  flag0 (skip paired80000000), then reverse paired projectors. Invoke actual
 00507A40(1), refresh SAME collision pose and004F5840(1,1), publish22C0=true,
 22C1=false, brake/input/cache tail. Endpoint failures leave the original prefix.

`CarLoadingLifetime::physical_reset` is implemented by the existing activation;
actual frame cache, quaternion, SceneEC, sound, health, part, wheel and bucket
owners remain shared. Wider driving and positive mesh restore are still required.

##004DCD10,004DCDE0,004DD040 mesh/occupant transition

Guard actor2548 first. Reverse occupants detach their model with Link(NULL,1).
Enumerate actual Car model44 with kind1 and helper/camera model24A4 context.
The real engine callback004DCDE0 immediately reads nameE8+signedE6 (empty for NULL),
looks up helper48(name,FFFF), assigns matching mesh1E0 using visual9C(mesh,1),
then visual24. It does not collect all frames before doing swaps. The actual
callback captures C++ endpoint failures rather than unwinding through engine ABI.
A missing required helper fails when the first visual callback needs it.

Update Car model24; reverse CURRENT occupants relink to actual seat2C marker,
flag1, then re-read current occupant before model24. Refresh each SAME part's
mesh8 from frame1E0 in reverse (004DD040), then publish2548=true. Failure does
not publish that final flag or replay prior links. No replacement Car/seat tree.

##004DC780 queued event4

Actual shared004DB360 event4 still calls004E2000 DM presentation first, then the
now-bound004DC780 through GameBlastWaves and the actual loading lifetime.
00611080(0) is already confirmed empty and receives no credit.

Save detached40000000 for forward max6 wheels; call004E9DB0(previous speed958,true).
Restore saved detached bits/hide/update for those six only. Execute existing
callback-sensitive light clear, forced engine off, fuel0/body health0.
Actual004DE800 policy precedes direct004F2790: impulse(0,mass918*3,0), scale(1,1,1),
flags0, point actor350/core2C0 **model position**, not centre1B0. State6A4 remains
an optional actual callback, coldNULL. Invoke mesh transition before parts.

Reverse intact parts use current frame world minus cached model position,
native extended square/normalization, one ENGINE RandFloat gain(r-.5)*10+20,
absolute Y and minimum5. Glass health becomes0 before00500DA0(direction*.1).
Non-glass additionally consumes CRT bit; a selected part's health becomes0 before
actual694(index,zero velocity,force). A false detachment result is ignored.
After all parts, release actual model254C, then forward CURRENT seated Humans:
when actual DD0 exists, targetDA8=.2/appliedDAC=.3 and00789CD0(model,500,0).
The actual Human loading composition/control/opacity owns this state and model
opacity notification; no substitute flash/damage owner.

##00500DA0 glass and004FE910 restore

For health24<threshold20 and flag8 clear, borrow SAME part mesh8 LOD0/material0.
If material8 has04000000, invoke44 before reading texture0C. When both current
material texture and part retained texture0C exist, increment current texture14,
SetTexture1C(part texture), release old part texture, publish retained prior
texture, then flag8. Actual engine retained-reference ownership is used; no
texture clone, new registry or successful missing resource.

Zero health publishes detached bit1 BEFORE selecting inside2298 name1A40 or
outside1EDC (applied parameter names299/3C0), then actual69C. Its installed
004D4A10 genuinely ignores direction/name (confirmed source/raw); it still emits
actual GEffects+B4 for visible mesh parts before hiding. Positive mesh-fragment
00485BA0→00484F60 remains an explicit required failure. Those absent/unfinished
bodies receive no new function credit. Packet0 and queued destruction now share
this glass path, including positive material-exchange prefixes before that failure.

004FE910(0)'s reverse glass restore uses the SAME exchange, updates part frame24
only after a successful swap, and clears8 even when either texture is NULL.
Alternatives/base visibility and native health/flags/6B4 ordering follow. A concrete
old all-parts flag preflight was removed: a later positive mesh backup now fails
at its actual per-part point after earlier reverse parts have been restored.
Positive deformed-mesh/global backup and occupied attachment restoration remain
required; no complete004FE910 promotion.

## Verification

Temporary ignored `human-car-physical-reset-native.cpp` passes100 repeated
ASan/UBSan/LSan families using actual body/wheel/health/visual rotation, generic
reset, mesh transition, glass state/restore and HumanOpacityControl. It checks
negative/zero speed, required engine call point, preserved fields/lane state,
paired/standalone links, restore gates and interrupted publication. It seeds
known actual coefficients/lighting with `-fno-access-control`; it does not load
real Car geometry. Scene/engine/math/collision/mesh/material endpoints are explicit
doubles. Texture reference ABI is raw-reviewed and actual-host compile-verified,
not native engine executed.

Temporary `human-car-queued-presentation-native.cpp` passes100 repeated event4
families: real stream/DM/part/wheel/body source, six-wheel retention, correct
mass*3/model-position impulse, ENGINE/CRT order, glass/non-glass split, model
retirement, opacity dispatch and required failed tail. Physical reset, control,
mesh/fragment and opacity endpoints are doubles in this fixture; they are checked
separately above and composed with actual owners by the strict i386 fixture.

Strict actual nested World/Car/control/Human/DM/frame i386 instantiation and
launcher/probe compile/link pass in `human-car-physical-reset-*`. No original
EXE/DLL/Proton or Windows output executes. No whole-body engine equivalence,
application loading, loaded frame, live transport or desktop acceptance claim.
