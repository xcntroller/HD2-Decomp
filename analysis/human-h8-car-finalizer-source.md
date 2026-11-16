# H8.4 running Car finalizer — source checkpoint, 2026-11-16

**Subsequent source checkpoint:** [active lights](human-h8-car-lights-source.md)
now removes the00501CB0 rendering guard described below. Occupied/D11 and
containing delivery remain open.

Branch `feature/p22-hum-h8-4`, base0ed148b. The actual loading lifetime now
calls0050B180's running finalizer on its existing owners. This extends the
[powered step](human-h8-car-powered-source.md); H8.4/D4/D5/H8 and P2.2 remain
OPEN. Occupied/non7 presentation and confirmed D11 missing producers remain
required. No original EXE/DLL/Proton, Windows artifact, application or desktop
execution; no post-fork Agent1 source imported and no push.

## Evidence and calling conventions

Research-index lookups preceded exports for0050B180,00518AB0,0051B7C0,
004FCBD0,004ECE20, the four particle workers and their vector dependencies.
Existing variants are recorded under WORK=`analysis/work/mission-world-20261103`
in `h8-4-finalizer-index.txt`, `h8-4-finalizer-particle-index.txt`,
`h8-4-finalizer-abi-index.txt` and `h8-4-finalizer-physics-index.txt`.
Original PE SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.

`h8-4-running-finalizer-original.c` retains the existing full finalizer export.
Read-only/noanalysis Ghidra exports recovered the missing engine mixer/pause,
skid and pool-allocation bodies. `ExportCarRunningFinalizerABI.java` then
applied temporary signatures/call-site overrides inside a rolled-back
transaction; the research project was not saved. Final typed evidence is
`h8-4-finalizer-typed-v3/*.c/json`, with initial audio exports in
`h8-4-finalizer-missing`. Raw decompilations have invalid stack/value inference;
their differing ABI versions are not interchangeable correctness evidence.

Specific assembly/constant checks resolve consequential expressions:

-00518AC4..00518B08: F48 seconds ramp, cap0.3 and shared gain multiplication by
 FLOAT3.3333332538604736 at008109A8. 0083BEC4 starts at1 but **is mutable**;
 the earlier read-only claim in the gear checkpoint is superseded.
-00518CB2..00518D76: exterior C54 fallback pitch uses CEC/CF0 and914/221C;
 volume uses CF8 with low-speed CF4 fade. The decompiler's unassigned register
 and RPM-as-volume expressions are not used.
-00518F36..0051916A and corresponding later banks: pitch is spilled before
 engine calls, volume is the stack volume temporary multiplied by gain.
 The old global is restored at ordinary returns, not replaced with RPM.
-00507146..005071EE/0050722B..00507264: point offsets stay extended, cross
 X/Y spill before velocity subtraction, cross Z remains extended; spread Z
 product spills before its base-vector addition.
-00506FB2: damage638 is compared directly with requested definition ID, unlike
 wheel/impact workers which compare the pool entry's definition. Preserve this
 original oddity, including the possible second allocation after first success.
-004ECE44/004ECF80: callbacks are fastcall with ECX=prior slot/entry and
 EDX=definition. 004ECFC0 instead has Car ECX and a stack slot argument.
-0050D23A..0050D288: low-speed automatic requests are gear1 then gear2, with
 the current gear ratio and native threshold order.
-008104E4 is FLOAT0.6499999761581421, ordinary dust's projected-velocity scale;
 spread is0.001 there,0.01 for slipping wheels/impacts,0.1 for smoke/exhaust.

Bounded ASM is retained in `h8-4-engine-audio-original.asm`,
`h8-4-damage-particle.asm`, `h8-4-particle-allocation.asm` and
`h8-4-finalizer-tail.asm`; PE bytes are in `h8-4-finalizer-constants.json`.
The [receipt](checkpoints/p2.2-human-h8-car-finalizer-20261116.json) fingerprints
important evidence and temporary verification artifacts.

## Source behavior and owners

`CarSoundUpdates` implements00518AB0 on the SAME81-slot sound owner. It retires
old-side banks in reverse order before updating2298, chooses the external
fallback or forward/load/reverse five-row banks, applies authored neutral flags,
RPM ranges, load interpolation and fade windows, and preserves the distinct
idle<=0.01 versus playing<0.01 thresholds. Its final single loop has its own
range/volume gate.0051B7C0 uses AC(0,1), in ascending bank order, on only the
native engine loops. Other existing Car sound consumers now read the SAME
process gain rather than a per-Car copy or hardcoded1. Sound loading itself
still sets literal1, as0051BD80 does.

`CarSkidModels` implements004FCBD0 on its real shlaif01/02 retained pairs and
SAME wheel218 cursor. Bytes8/9/A are separate active, continuation and second
visibility flags. Rejection retires without frame calls. Active production
uses contact+normal*0.05, reversal retirement/restart, 0.05/5 length gates,
previous second-model corner joins and repeated0.3 segment splitting/wrapping.
Existing direct position, SetDir3 and scale bridges perform the native frame
writes; delegated engine geometry/math bodies receive no new credit.

`CarFinalizerParticles` borrows actual callback6BC/6C0, shared pool and retained
particle table from the actor.004ECE20 tries prior reuse first, reverse-scans
matching/inactive entries, copies an existing actual visual and base vectors
or calls real6BC, then sets active before engine Activate(true). It does not
silently clear pending, manufacture emitters or turn allocation failure into
success. Body/wheel/exhaust/impact workers publish lighting, properties D..12,
actual positions and frame24 in native order; retirement uses existing pending
flags. Point velocities retain the original angular sign convention. Exhaust
uses completed22BF, independently of start-request22A2.

0050B180 restores22BF from22C2 before zero-time/low-Y handling when22C1 is set.
00501CB0's active-light rendering guard is retained: steering, brake and reverse
requests can select it before particle publication. Thus this is an engine/audio/
skid/particle source checkpoint, not closure of the full running finalizer.
It reads the SAME740 brake amount, F48 timeout and2294/2295 signals, and uses
motion170 for slip distance rather than controller420 history. Engine work
precedes remaining audio. The finalizer now calls active skid/particle workers,
clears only force/torque XYZ, preserves engine-aware sleeping and requests final
low gears through the existing004F2030 source. No independent pending-engine or
air-pressure duplicate remains. Existing collision/road/door/window/light/
wheel/axis/strip ownership is retained.

## Verification and limits

Focused native ASan/UBSan/leak audit passes267374 checks, including12000 engine
mixes against independently stated range/load/fade/pitch expectations, old-side
and pause ordering, exception gain restoration, skid start/short/split/wrap/
reversal/rejection, pool callback/matching/copy/full cases, positive dust/slip/
body/impact and running exhaust publication. Frames, sounds, scene/math/geometry,
material/physics coefficients and particle callbacks are explicit doubles;
positive exhaust754 is injected by a test geometry, not claimed as authored
World delivery. Native worker tests are not a full0050B180 execution or original
binary comparison. Actual nested World/Car activation and both frame passes
fully instantiate/link under strict i386 O0; launcher/probe compile/link too.
Windows outputs were not executed. Existing tracker validation/tests pass.
Temporary audits are hashed and removed, with no permanent tests added.

All entries remain partial/source-only. Native object/container/allocator/SEH
ABI, malformed inputs, undefined scratch/vector W padding and whole-function
or engine equivalence are not claimed. RGB helpers write XYZ only; the adapter
retains serialized fourth-lane padding rather than claiming a native alpha
producer. Engine gain additionally restores on a C++ endpoint exception, an
explicit failure-containment adaptation to engine calls that cannot unwind in
the original ABI. Native debug-selected drawing remains an explicit guard.
00501CB0 active-light rendering and
D11's confirmed fresh part5F9/wheel695/EXPL8/smoke14 omissions remain unchanged;
no fallback coefficients or successful services were added. Occupied/non7
presentation and containing Runtime/World/OWN/FRAME/application/desktop gates
remain open. This checkpoint closes neither H8.4 nor the selected full checkbox.
