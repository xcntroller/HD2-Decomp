# H8.4 Car controller policy — source checkpoint, 2026-11-16

Branch `feature/p22-hum-h8-4`, base85a9aa1. This continues the
[gear/clutch checkpoint](human-h8-car-gear-source.md). H8.4/D4/D5/H8 and P2.2
remain OPEN. This is source recovery with native boundary doubles and Windows
compile/link verification; occupied presentation and application loading remain
gated. No original game/DLL/Proton, Windows fixture or desktop execution.

## Original boundary and owners

Original image SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK=`analysis/work/mission-world-20261103`. Existing typed
`car-solver-typed-required/00509aa0.c/json` supplies the entire5841-byte controller
boundary. `car-runtime-typed-v4-required/004ea780.c/json` supplies initialization;
existing004E2900/004ED270/0050D2C0 bodies were located through the research index
and copied as `h8-4-controller-{constructor,parameters,force}-original.c`.
No new export, Ghidra mutation or fresh function credit is involved.
`h8-4-controller-all-constants.json` records bounded PE reads with types/bytes.
`h8-4-controller-steering-threshold.asm` resolves a concrete precision discrepancy:
0050AA39..0050AA65 subtracts950 from94C, takes FABS and compares directly against
DOUBLE00810980, without an intermediate FLOAT subtraction store.

Actor2321 is core2291 (the actor embeds the core at+90). The initializer now
requires a reference to actual `driver_enabled2321`; its controller borrows that
reference and clears the SAME byte at004EA780's2291 store. Existing H3
004D11C0/004D1200 seat assignment/removal writes that owner. There is no per-frame
copy or synthetic driver state. Actual production initialization is full-linked;
native audit driver flags are explicitly seeded boundary state and do not close
occupied presentation.

004EA780 initializes228A from nonempty native570/580 strip-connection lists,
not visual axle records or wheel-count guesses. The controller snapshots the
SAME applied geometry's first/second connection lists at this initialization.
Existing F68 solver angle and78C drivetrain torque accumulator are now the ONLY
owners: physical reset writes solver.speed_angle and drive.torque_amount rather
than separate reset-field copies.004E9DB0's reset order remains unchanged.
22C2 is retained separately;004EA780 clears it and00509AA0 saves22BF into it when
22C1 suppresses the engine transition.010 and020 vectors are initialized zero
by004EA780;0050DE60 clears their XYZ after incrementing its index and BEFORE
traction. The recovered cold solver now preserves those actual stores, including
partial publication if a later query fails. Powered010 production remains open.

## Recovered policy

`car_drive_controller.h` implements00509AA0's full decision tree over the SAME
initialized body, wheel, physics, drivetrain, input, health, sound and substep
owners. `car_automatic_gears.h` holds its local automatic-selection arithmetic;
these helpers are not separate frozen functions. The loading lifetime now calls
this controller rather than its restricted cold-only helper. The actual mandatory
step still calls `car_cold_solver_step`; unrecovered powered work throws, and the
running0050B180/occupied-frame requirements remain in place.

Zero milliseconds becomes1. Convert unsigned ticks to FLOAT before multiplying
by FLOAT0080F514. Rolling speed uses the selected ratio and forward/velocity dot
before any update. Emergency brake22C7 clears both requested inputs.22C1 saves
and clears22BF, then clears inputs. Hold554 publishes600 bit10000000; mass changes
wake the body and clear positive stable time. Kind2 calls real00507400.
Fuel consumption uses PREVIOUS85C/860, not the new6FC/700 input. Kind1 averages
absolute old throttles. The idle fraction uses864/221C and the actual784
coefficient. Native minimum0080F53C is .0001; exhaustion publishes zero fuel,
22A0 and zero requests before copying inputs to throttle.229C selects direct
clutch-target publication; otherwise the native FLOAT ramp/clamp is preserved.

The slip/wheel guards decide automatic shift eligibility using actual speed,
ratio, engine and wheel flags. The gate uses feature4000/health63C,22BF,229D,
SAME2291,600 bit1000000,22C6, current gear and2293/228A. It calls the recovered
004F2030 and004F3840 bodies. Fast contradictory forward motion publishes BOTH
current/requested reverse gear directly and sets saved local44=0. The distinct
free-rolling arm advances the positive-gear table; its pointer expression uses
ratio(index-1), not ratio(index). Reverse-to-forward motion has its own direct
publication. Slow reversal accumulates780 and uses native .002 seconds.

Predictive up/down selection reads previous solver010 driving force and020
published net force. Preserve FLOAT X/Y/Z intermediate stores and the extended
Z contribution to the dot; refusal leaves initialized local force values1.
The first-gear negative-dot arm clears22C6/clutch, writes current874 only,
updates910 and calls actual0051A480 even when audible2296 is clear. Low-speed
clutch downselection retains original local44 and uses lower/upper threshold
indices80C+candidate*4 and7C8+candidate*4 with ratio(candidate-1).

714/718 capture throttle BEFORE clutch suppression and acceleration filters.
The acceleration gate uses previous420 velocity and previous7C4 interval;
reverse negates it. The first filter's bit4 bypass does not apply to the second
filter.910 bit10 bypasses both. Ordinary force is shared between sides; kind1
preserves signed independent forces and its asymmetric FLOAT spills. Capacity
limiting, stable/inactive modular counters, steering wake, held/sleep decisions,
engine/clutch policy and reverse wheel pass preserve original ordering. Histories
publish before the mandatory0050DB90/0050DE60 call.22A0 invokes actual00518710(false)
only AFTER that step returns. Contact-bit8 hysteresis follows final wheel flags.
Inactivity compares retained initial160 against checkpoint030, not moving pose240;
the old restricted cold helper was corrected to use this SAME retained position.

Reached0050D2C0 evidence also proves its engine-force gate is22BF, not22A2.
22A2 can already be true during startup while22BF is false. Force now requires
that actual transition explicitly; production controller passes its SAME22BF,
and the restricted cold helper passes its proven false state.78C physical reset
now clears the actual force accumulator. No startup force is guessed from22A2.

## Verification and limits

Native ASan/UBSan with leak detection passed4,150,496 checks across90,000 mixed
held/active scenarios and900 focused fixtures over nine actual captured ordinary
parameter blocks.57 deliberately seeded missing-threshold guards matched.
The separate all-held pass passed3,870,011 checks/90,000 scenarios with69 guards.
Strict O0 actual-host i386 FULL LINK, final launcher/probe builds, tracker exact
check and27 existing tracker tests PASS. The checkpoint receipt records these
results, fingerprints and removal of temporary native/Windows audit fixtures.
The temporary audit independently transcribes exported00509AA0 C into a native
raw-offset reference. It executes rebuilt source only; no original binary
instructions or engine calls are executed.32-bit scratch wheel references are
adapted to native pointers. Undefined native ADD spellings use explicit unsigned
wrap; native FLOAT bit tests use bit reinterpretation. The concrete steering
FABS precision above is reproduced. Reads of unproduced threshold slots fail in
both models rather than accepting zero-initialized audit scratch.

Actual parameter decode/conversion/application and source body/drivetrain/
controller/gear code are used. Geometry/frame methods, random driver/input state,
SOUND endpoints and required solver entry are explicit boundary doubles. Held
cases finish policy; active cases compare through the throwing mandatory step,
including force/input arguments and partial histories. Missing threshold guards
in deliberately inconsistent seeded shifts are counted separately; this neither
proves those states reachable nor invents native initialization at7C8/80C/810.
Focused repeated fixtures cover driver-reference identity, zero ticks, fuel and
input order,22C1/22C2, clutch modes, signed counter wrap, initial-position sleep,
startup22A2 versus22BF, direct reverse publication, two signed kind1 forces and
failure before engine stop. Kind1 is an explicit scalar injection, separate from
captured parameter provenance.

00509AA0 stays partial: conditional absent coefficients, powered0050DE60 and
running0050B180 remain required; no whole-function ABI or loaded-World equivalence
is claimed. No denominator/complete-count/delegated engine credit changes.
Continue with powered wheel/body forces and running engine/audio/skid/particle
finalization, then actual H3 occupied presentation. D11's already confirmed
fresh part5F9/wheel695/EXPL8/smoke14 omissions, selected non7/entry dependencies,
other H8 agents' containing work and desktop/application acceptance are unchanged.
