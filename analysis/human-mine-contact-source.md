# Armed Mine contact and explosion — source checkpoint, 2026-11-12

[Receipt](checkpoints/p2.2-human-mine-contact-20261112.json). Extends the
[actual Mine owner/producers](human-runtime-mine-source.md),
[containing Human blast binding](checkpoints/p2.2-human-containing-blast-20261112.json)
and [Versions contacts](version-damage-transitions.md). FULL selected Human
reactions/damage/ragdoll checkbox remains OPEN; application mission loading is
still disabled and no new desktop acceptance is claimed.

## Original contact and data

005FEFB0 is thiscall/RET8: contacted Actor pointer and a genuinely UNUSED second
stack word. Raw5FEFBD..5FF10E overrides incomplete C parameter labels. Bound but
inactive98 returns. Own Human9C pointer comparison precedes touching-Actor reads.
Human3 does not trigger definition71; Car4 does not trigger70. NULL touching with
nonnull source fails at the reached native dereference, while bothNULL still
satisfy the native own-source early return.

Clear98 BEFORE unregistering a registered99 descriptor. Capture Human9C, clear99,
then source selection: existing9C wins; otherwise touchingHuman3; otherwise actual
Car24AC through existing004D11F0, distinct from seat0 occupant. Unknown nonHuman/
nonCar fallback rejects its native invalid Car cast; no substitute source. Actual
runtime contact checks owner membership before dereferencing descriptor borrowers.
The original catalogue pointer/native48 lookup follows source selection BEFORE
wire; payload fields are read AFTER packet completion. Missing required definition/
payload keeps the already disabled/unregistered/wire prefix as reached.

Actual readonly `analysis/work/single-pipeline-data-20261031/items.sav` SHA256
`9e82e40543e3c5b65d6004f2d45fac656d1e4f5a8df5259a3a0db2520c5983b6`
confirms70/71 are first-payload **type2,40 bytes**, not type8. Both sound native14
are2; radii native28 are5/8 and powers native10 are2500/25000. Native payload
includes its vtable at0; serialized bytes start at native+04: radius24, power0C,
sound10. Existing Mine visual effect sound10/particle18 remains correct and works
for these actual40-byte records. Earlier type8/28-byte synthetic checks established
only that shorter effect-field boundary; they were not the original Mine table.

00478780 borrows SAME module12 process broadcast stream priority0/3 fortrue/false,
writes selector0 in TWO bits then runtime token10. Contact writes command0 in two
bits and finishes through00478880/0072BF90. No new streams, flush or transport.
Use retained base30 position (already modelY+.2 from arming), add FLOAT0080F540=.2
for the EXPLOSION centre, then00451990(source,centre,radius,power,sound). Presentation
005FEF50 still uses the original retained base30/40 for its sound/particle effect;
these two centres deliberately differ by.2 in Y.

After explosion, reread mode: !bit2 removes actual runtime owner immediately;
bit2 retains its disabled/unregistered owner until actual presentation packet0
plays existing005FEF50 and removes it. No eager presentation or atomic rollback.
An explosion's initial damage can retire the source Human and its Mine9C links
while005FEFB0 is still on stack. Keep containing runtime reference locally for
post-callback mode/removal; no reference is read through the freed Mine and no
extra Mine/Human retention changes native destruction order. Focused sanitizer
checks use actual00479700-style source scan for this reentrant retirement.

005FEEE0 clears98, emits the same envelope with command1, then removes only when
!modebit2. It does not read binding fields or unregister99 before its packet.
Actual removal/deactivation, including presentation005FEB50 command1, unregisters
collision. No successful general action receiver is invented to reach this body.

## Actual containing receivers and explosion producer

0059FFD0's moving/live/state5C mask16000 branch now uses actual shared World
services: actual model world70 centre,006FCB60 radius.5/maskFFFBFFFF, only FIRST
returned descriptor, then mask40000/actor test and005FEFB0(Human,-1). It does NOT
try later nearby Mines if the first ignores this Human family. SAME dynamic bucket
query order/descriptors and runtime owner membership; no copied actor registry.
Loaded World and real grid are required. Other Human frame/action/formation
receivers remain required; this does not assert complete containing frame delivery.

Car6AC uses the same actual runtime contact (wheel index is the unused second
word); its reached00451990 request now borrows the same explosion bridge. Runtime
links bind with existing Car/physics construction and unbind before owner teardown.
Actual GameBlastWaves binds checked World/Car borrowers only after construction;
its destructor unbinds while those owners still live, including failed lifecycle
construction. No parallel ShotMgr, wave process, scene, damage or sound owners.

00451990 takes FIVE stack arguments. If power>0 THEN radius>0, create a request
with exact centre/radius/power/source and same-sector=false. SAME006DF170 performs
its actual initial100ms advance BEFORE the late `(mode&7)==7` SOUND branch. Ignore
wave result. Reuse actual ShotMgr D0 request: group6, full retained position,
relativeNULL, entry argument, SAME borrowed source, flags22. Leave filename,
ranges, voice group and other reused fields unchanged. Emit real SOUND, ignore
result. Nonpositive/unordered dimensions skip wave generation but can still emit
offline sound. Existing loaded-World/finite-wave required failures remain honest.

Blast-to-Human continues through existing actual lifetime→005A1C90→Player B0
commands with source kinds2/4 and final hit0. Positive other derived damage,
especially CarD4, remains REQUIRED and is not bypassed. Actual Car vtable00810778
maps D4=004DBA10/D8=004DBEE0; the existing blast kernel skips material D8 forCar4
and callsD4 directly. Indexed reused evidence is WORK
`human-containing-car-blast-index.txt` /`car-callback-world-damage-required/004dba10.c`.
Car surface effects, other derived damage, save/replay and full entry/frame remain
next dependencies; no completed Human or Car frame claim.

## Evidence and verification

WORK `human-mine-{contact,contact-envelope,explosion-producer}.asm`, indexed existing
005FEFB0/005FEEE0/004D11F0, new00451990 C/JSON and original table report. Research
index preceded new export. Existing Car driver getter and unchanged blast/solver/
packet kernels receive no new body credit. Heavy compilation/Ghidra sequential.
No original EXE/DLL/Proton/Windows fixture execution.

ASan/UBSan/LSan800 new armed-contact lifetimes across1/2/3/7 x100 xoriginal70/71
plus existing Mine owner/producer checks PASS. Actual Mine/runtime/buckets/ShotMgr/
streams/envelope/leaf, source/family/disabled-state gates, unregister-before-wire/
blast, partial failure, late mode, native disarm and source-linked self-retirement.
Actual dynamic radius query/first-result Human consumer tests its first71/second70
quirk. Frame/resource/localization/Car-driver/explosion endpoints are explicit
doubles in this fixture; original items.sav is readonly input.

Separate ASan/UBSan/LSan800 actual00451990 producer→BlastWaveProcess100ms→SAME
borrowed source/NULL lifetime→005A1C90→actual Player B0 queue→late reused SOUND,
plus preceding800 direct wave checks PASS. Geometry/ray/material/Versions/SOUND
and the reusable sound-value owner are explicit doubles. These two fixtures are
not a native full World composition. Strict actual complete World/Car/runtime/
Human contact/Blast/Versions/VM/SOUND/module i386 and final launcher/probe compile/
link PASS; Windows outputs not run. Temporary audits remain ignored until full
selected checkbox completes. Adds4 client partials/extends2, zero promotions:
34 complete/2701 partial(2696client+5engine) of18783 frozen.
