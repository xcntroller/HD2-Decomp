# H8.4 powered Car step — source checkpoint, 2026-11-16

Branch `feature/p22-hum-h8-4`, base96f43ec. This continues the
[controller checkpoint](human-h8-car-controller-source.md). H8.4/D4/D5/H8 and
P2.2 remain OPEN. Loading now calls0050DE60's powered source step; occupied
presentation and running0050B180 still require their actual receivers. No
original EXE/DLL/Proton, Windows fixture, application or desktop execution.

## Boundary and original evidence

Research index lookup covered0050DE60/0050B180 and their existing C/ABI variants
before implementation; no fresh export or Ghidra project mutation was needed.
WORK=`analysis/work/mission-world-20261103`. Original image SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
`h8-4-powered-original.c` copies the typed-v2 full17706-byte0050DE60 body;
`h8-4-powered-metadata.json` preserves fingerprint, ABI and callees without its
large instruction-reference listing. Its24-byte argument area is elapsed, three
drag values and two drive forces. Inputs6FC/700 are read from the SAME core.
Existing body integration, BODYCOL, wheel queries/notifications, steering and
0050D760 source remain the mandatory downstream dependencies.

Bounded original ASM preserves concrete decisions:

-0050EAD3..0050EC71: velocity square/normalization and directional projection.
 X/Y stay extended, normalized Z spills, then the projection spills. The
 reciprocal must not be rounded before multiplication.
-0050F081..0050F0FC: brake demand stays extended through torque conversion;
 remaining grip spills before comparison, and final brake torque spills once.
-0050F2FA..0050F349:674 is compared as a SIGNED INTEGER word against current
 gear, even though004ED270 copies its serialized word through FLOAT storage.
 Its remaining-capacity square root is not rounded before the comparison.
-0050F50C..0050F661: longitudinal resistance stays extended through the wheel-
 lock comparison. X/Y subtraction products stay extended; Z product spills.
 Rounding before this comparison changes20/10 flags and chooses a different
 friction branch. These were actionable discrepancies found by the native audit.
-00510E0F..00510E95: core160 is transformed, advanced and published to model
 translation. The full C also proves collision retry rewrites/additions to160.

`h8-4-powered-constants.json` retains original bytes/read interpretations.
00810990 is FLOAT2.799999952316284 (governor brake window),0080F748 is DOUBLE2
(wheel-spin discrepancy threshold),0080F708 is FLOAT1000 (2287 steering pass),
00810948 is FLOAT.017453279346227646 and0080FD00 is FLOAT.3333333432674408.
No successful constant/default is supplied where a producer is absent.

## Corrected live owners

The earlier claim that160 was a frozen initial position was wrong. New evidence
in0050DE60 contradicts that interpretation:160 is the LIVE simulation position;
240 is the forward vector. The source now names it `physical_position`, updates
it BEFORE model publication and on collision rollback/retry, and integrates the
next candidate from it. The separate frame-derived pose cache remains available
to existing collision/snapshot code.004E9DB0 refreshes the physical position from
the actual model. Inactivity030 and Human-to-Car hit58 force origin now borrow
that SAME live160. Earlier receipts describe historical checks, not proof of
an immutable160 invariant; this checkpoint supersedes that claim.

2293 is the existing drivetrain driven-contact byte, not an independent solver
Boolean. The wheel pass clears it and sets it for actual driven contact, making
it a real producer for00509AA0's automatic gate.229F is ONE controller/visual
byte: the solver clears it on nonspinning driven wheels, and finalization sets
it true. Wheel and strip visual updates and physical reset now borrow that same
controller field. The independent solver wheel-spin/contact copies are removed.
The restricted cold helper accepts its actual229F reference and permits proven
engine-off driven contact; it does not manufacture a separate flag owner.

228F is initialized from applied parameters, temporarily enabled in the actual
governor window and restored with8E4 after the wheel pass.228B is initialized
false by004EA780 and remains the actual handbrake-mode state. F7C starts from
004ED270's zero and0050DE60 unconditionally publishes1 after traction/aero.
There is no source producer of its old0/2 dispatch arms during the second pass;
no alternate differential behavior is invented.004EA780 initializes target40
and2287; the optional solver steering pass consumes that SAME target/flag with
actual004F2420/00507400 bodies, without manufacturing a target producer.

## Source composition

`car_solver.h` replaces the cold-only call in the loading lifetime. It advances
index, clears010/020, computes the pre-traction engine road-speed temporary,
runs actual traction and accumulates springs into020 as each wheel is processed.
`car_powered_body_forces.h` adds two aerodynamic downforce samples and directional
drag BEFORE the second wheel pass, preserving diagnostic publication and torque
arithmetic. `car_powered_wheel_forces.h` then handles the full reached running
and1000000 engine-off wheel paths over SAME runtime/parameter/tyre owners.

The reverse wheel order and alternating side survive skipped/broken wheels.
The pass computes directional projection/slip, lateral grip, emergency brake,
ordinary brake, optional yaw correction, brake-limiter ellipse and shared drive
allocation. The native distribution denominator is the two driven-limit totals
PLUS1. Gear/type select drive direction.010 accumulates requested drive before
ellipse limiting;020 includes first-pass springs, aerodynamic and final wheel
forces, then subtracts010. Brake reaction and used drive magnitude remain
separate scalars for drivetrain integration and low-speed remaining grip.

Friction ellipse, blocked-wheel20/10, slip200, spin80/10, rolling resistance,
2293/229E/229F and86C engine load retain their original ordering. Builtin and
registered tyre coefficients use actual owners; missing surfaces fail explicitly.
Undefined nonfinite ellipses retain an explicit guard, matching the preexisting
cold-source policy rather than guessing a zero force. Low-speed stabilization
subtracts the requested drive magnitude from longitudinal grip.020 receives
external force and gravity but excludes low-speed correction and010 drive.
Gravity diagnostic publication occurs after velocity update and before rotation
endpoints. The body is published before mandatory BODYCOL acceptance and cached
pose refresh. The optional steering target pass precedes wheel queries; speed,
rolling speed, rest policy, actual0050D760 reaction integration and finalF68
angle follow. Kind1 uses the native doubled first force, with sign selected by
first throttle; this is not replaced by an invented symmetric drivetrain.

## Verification and remaining boundary

ASan/UBSan/leak PASS846317 checks /18000 original wheel-pass comparisons across
nine captured parameter blocks, including focused full-step failure fixtures.
No undefined ellipse guard was selected. Strict actual-host i386 full link,
launcher/probe builds, tracker exact check and27 existing tests PASS. The receipt
records fingerprints and removal of temporary native/Windows audit fixtures.
The temporary native reference transcribes the original second wheel pass,
with explicit32-bit scratch pointers, FLOAT bit tests and initialized unused
W/padding temporaries. It executes reconstructed native source only. Concrete
spills above are resolved against ASM rather than trusted decompiler temporary
types. Flags/gear/control publications are compared exactly; float quantities
use4e-5 relative tolerance and a .002 absolute allowance for cancellation in
summed torque. No whole-function numeric/ABI equivalence is claimed.

Nine captured ordinary parameter blocks use actual decode/conversion/application.
Wheel/contact histories, geometry/frame/math, query/collision/audio endpoints,
handbrake/ABS/yaw and kind0/1/2 injections are explicit boundary doubles; those
injections do not prove authored driving, registered tyre-table provenance,
occupied input production or desktop acceptance. Focused native composition
covers cold repeated movement, positive powered010/contact/load, body publication
before collision failure, query failure before drivetrain/angle, clutch audio
failure before selected gear/angle, aerodynamic force/torque and remaining grip.
This is targeted verification of changed paths, not a repeat of old H1–H7 audits.

0050DE60 stays partial. Existing plane-cache query guards, conditional absent
coefficients, actual occupied presentation and running0050B180 engine/audio/skid/
particle production remain required. D11's confirmed fresh part5F9, wheel695,
EXPL8 and smoke14 omissions remain guarded and were not re-investigated or
initialized speculatively. Non7/entry dependencies and full OWN/FRAME/application
composition remain open. No frozen function, complete promotion or delegated
engine credit is added. Continue H8.4 at0050B180; full checkbox closure remains
blocked by the already confirmed D11 limitations and containing-path work.
