# H8.4 Car gear and clutch dependencies — source checkpoint, 2026-11-16

Branch `feature/p22-hum-h8-4`, base1a6c958. This continues the
[offline authority checkpoint](human-h8-car-authority-source.md) with required
00509AA0/0050D760 dependencies. H8.4/D4/D5 stay OPEN. The automatic controller,
powered force solver, running finalizer and occupied presentation are not enabled.

## Original boundary

Client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
The research index found existing typed00509AA0/0050D760 C and004EA780
initialization evidence, and no exported004F2030/004F3840/0051A480 bodies.
Those three workers were exported read-only/noanalysis with ExportFunctions.java;
the headless logs contain no ERROR. No project/type/signature edits or original
execution occurred. WORK=`analysis/work/mission-world-20261103`.
New `h8-4-car-drive-dependencies` C/JSON and `h8-4-gear-request-original.asm`,
`h8-4-gear-force-original.asm`, `h8-4-clutch-audio-original.asm` and
`h8-4-shift-completion-original.asm` retain original behavior evidence.

004F2030 checks requested gear in[-1,count] before the unchanged-request return.
Kind1 rejects a NEW negative request; an unchanged request returns true first.
When feature4000 is set and actual drivetrain63C is not positive, return true
without mutation. Otherwise, except requested-1, compare221C against EXTENDED
`(requested ratio/current ratio)*engine864`. Refusal leaves780/878/910/22C6
unchanged. Accepted changes clear780, conditionally publish22C6 and910 low bits
when910&20, then write878. Current874 is left for0050D760. The actual Car loading
lifetime now exposes this source body over SAME physics, drive, solver feature,
health and reset-field owners. The containing automatic00509AA0 producer remains
to be recovered; an exposed source API does not establish that producer.

004F3840 checks the same gear bounds, then RPM against retained parameter words
5/4 at coreFF0/FEC with native FLOAT.001 tolerance. These are RAW RPM bounds,
distinct from converted RPS914/221C. Comparisons use native ordered refusal
conditions; output stays unchanged on refusal. Convert RPM to a FLOAT using
00810924=.01666666753590107 before calling the existing0050D5C0 torque body.
The optional second torque call uses throttle0 and adds a FLOAT spill of the
first result; without it, retain the extended first result. Multiply by the
selected ratio only for nonzero gear, then store the output FLOAT. The source
API uses SAME retained CarPhysics.applied_parameters, never duplicate coefficients.

0051A480 uses SAME CarSoundLoading voices and per-Car650 variant initialized0
by004EA780. Feature20 gates everything. Query the CURRENT interior9D8 and exterior
CA8 variant using real78(sound,0), even if the first query reports playing;
a playing second voice returns immediately. Only both idle permit one SAME CRT
draw. Native stores rand%100 then retains its >79 Boolean as650. Read actual
2298 AFTER that draw, enable the selected0/1 interior/exterior voice, reread its
slot, then set volume to shared0083BEC4 (initially1). The [running mixer](human-h8-car-finalizer-source.md) proves that gain is temporarily mutable; the earlier read-only interpretation is superseded. Missing selected resources still
consume the draw and retain the selection. If a synchronous endpoint retires
the required voice before volume publication, source fails explicitly. The
real CarColdReceivers clutch callback now calls this body over SAME sound,
feature, last-inside and process-random owners, replacing its old required guard.

## Corrected retained shift state

The newly reached dependencies exposed a concrete mismatch in0050D760's existing
source: it used the flag named changing_gear for both native2293 and22C6, and
published completion values into85C/860 instead of508/50C. Raw0050D760 proves
22C6 is the shift-in-progress byte. It is initialized false by004EA780, produced
by004F2030 and retired by actual shift completion.2293 stays a separate neutral
`flag2293`, including its actual physical-reset clear; it is not retired as22C6.

Retained508/50C filters now live once on SAME CarDrivetrainState, initialized1
by004EA780's actual stores.00509AA0's existing cold worker borrows them there;
the old independent controller fields are removed.0050D760 writes those filters
to FLOAT.8 on clutch completion, or1 on its low-rolling shortcut, without changing
current85C/860 throttle outputs. Clutch-down publication precedes0051A480;
requested878 is read only AFTER sound returns, so sound failure preserves the
old current874 but leaves the native already-updated clutch/transmission prefix.
A NaN clutch does not incorrectly complete the clutch-down phase. No unrelated
engine/torque/solver audit or whole-body promotion is claimed.

## Verification and remaining work

Temporary C++17 warnings-as-errors ASan/UBSan/leak audit PASS178100 checks over
100 lifetimes and nine captured parameter blocks. Those blocks pass actual
read_car_parameters→CarPhysics.apply on explicit four-wheel geometry doubles.
Checks cover gear bounds/refusal/no-op/publication, real retained RPM bounds,
torque/ratio composition, request→drivetrain→clutch-audio order, failing sound
before current-gear publication, fresh requested-gear reads,2293/22C6 separation,
retained filter values, unchanged throttle outputs and NaN clutch preservation.
All100 CRT remainders are exercised on both interior/exterior selections;
current-variant query order, busy suppression, missing voices, query/random
failure and voice retirement prefixes use real sound acquisition/ownership.

Actual source gear/drivetrain/sound owners and parameter decode/application are
used. Geometry, engine/frame methods and query/random failure injection are
explicit endpoint doubles. This does not execute a complete loaded Car, the
automatic00509AA0 decision tree, powered0050DE60 or running0050B180.
Strict O0 actual-host i386 FULL LINK instantiates both Car frame passes, actual
loading-lifetime gear APIs and same-owner clutch SOUND binding. Launcher/probe,
tracker checks and temporary-audit cleanup are recorded in the
[receipt](checkpoints/p2.2-human-h8-car-gear-20261116.json).
No Windows artifact/original EXE/DLL/Proton, application or desktop execution.

Three frozen partials are added:004F2030,004F3840,0051A480.0050D760 and00509AA0
gain corrected source/ownership evidence; no complete promotion, denominator
change, original allocator/vtable/SEH ABI or delegated engine credit. Current
gear APIs are source dependencies, not a successful replacement controller.

Next:00509AA0 running/input/automatic gear policy and its live state producers,
0050DE60 powered wheel/body forces,0050B180 running engine/audio/skid/particle
finalization, then actual occupied-frame verification before removing D5 guards.
Actor2321 and controller2291 describe the same native control byte; their current
separate source fields must be reconciled with the containing driver producer
when binding that controller, rather than copied or guessed per frame. Reuse
the indexed worker evidence; do not repeat completed H1–H7 audits. Remaining
non7/entry/absent-actor requirements and confirmed D11 failures are unchanged.
