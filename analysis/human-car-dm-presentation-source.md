# Car packet and DM effect ownership

Source-only continuation of [Car blast damage](human-car-blast-damage-source.md).
The reactions checkbox and P22-HUM remain OPEN. No application or desktop claim.
Original SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Evidence below is under ignored `analysis/work/mission-world-20261103` (WORK).

## Original boundary

Native00810778 actor1C=004DB360 is pinned by
`human-car-damage-receive-vtable.json`. Research index receipts precede all new
exports: `human-car-damage-receivers-index.txt`,
`human-car-damage-dispatch-index.txt`, `human-car-damage-presentation-index.txt`,
`human-car-dm-lifetime-packets-index.txt`, `human-car-particle-lookup-index.txt`.
New read-only exports: `human-car-damage-event-readers`,
`human-car-damage-presentation-workers`, `human-car-dm-particle-stop`; logs pin
successful exports. Their default C signatures retain indirect-call/hidden-ABI
aliases. `human-car-particle-retirement.asm`, `human-car-dm-metadata.asm`,
`human-car-dm-cold-state.asm` and allocation evidence establish the ownership fields.

Car module13 now reaches the actual actor1C through SAME GameWorldActors,
CarCallbackServices/GameCarCallbacks and GameBlastWaves containing owner.
The packet borrower binds only to matching actual Frames. Constructor failures
unbind it, and normal destruction unbinds before the explosion/world borrowers.
Unbound packet delivery fails. No alternate Scene2 identity or transport is made.

## Packet consumers

004DB360 always consumes its four-bit kind; there is no early mode gate.
004E0340 reads part10/health10/direction(-1).004E0410 reads part10/health10 and
both default-primary(-1)/mask7 vectors only when health<1.004E0540 reads wheel8/
health10 and one such vector only when health<1.004E0630 reads wheel8,
004E06A0 explosion4;004E0710 is genuinely empty RET4.

Kinds0/1 decode fully before dirty61C bit2→part38 bit2→FLOAT health24 publication.
Kind0 preserves00500DA0's material threshold/flag8 gate and zero-health flag1/
existing fragment-hide callback. Positive material exchange still requires its
actual receiver; **00500DA0 gets no new credit**. Kind1 zero health invokes SAME
existing694 part detachment with both decoded vectors, ignoring its result.

Kind2 still consumes its payload on authority. Only !modebit1 and current wheel
health230>0 publish dirty bit2, flags1FC bit10000000 and FLOAT health. Zero health
with5E0==0 invokes actual698 wheel detachment/decoded velocity/null impulse.
Its Item12 producer and event2 broadcast/request serialization remain required.
Kind3 sets SAME wheel80000000 before0051A880. That actual sound path reads
Car audibility, inside/outside slots9EC/CBC, enables then sets volume1 (read-only
0083BEC4), and outside updates sound position from SAME wheel30. No frame24 or
mass_changed2350 is added.

Kind5 invokes actual DM004E1F70; kind4 invokes DM004E2000 BEFORE the still-required
004DC780 visual/physical presentation. Other reached kind6/7/10/11 receivers
remain explicit failures. Native defaults8/9 and unassigned four-bit kinds are
genuine empty returns after consuming only kind; no physical state is borrowed.
00611080(0) is a genuine empty call throughout.

## Particle identity and retirement

00445740's signed binary lookup operates on SAME process active1014 records,
using its native middle choice and an output index unchanged on misses.
00445800 closes/deletes the actual matching instance **before** stable erasure.
It releases parent, points and visual immediately and does **not** return the
instance to a reuse pool. Missing IDs, including after module close when records
are empty, return without needing a live World facade. A C++ cleanup failure
keeps the record/index while preserving the actual cleanup prefix; a retry can
remove the now-closed instance. Typed cleanup/RAII is not an original SEH claim.

DM metadata borrows this same process through the actual existing Car callback
services/StartupSession.004E1F70 ignores partID=-1, otherwise aligns DM, reads the
explosion marker world matrix and emits004451B0(partID,matrix,CarFrame,true,false).
It rereads the explosion slot and publishes record8 AFTER emission. Repeated
emission overwrites that handle without stopping the earlier instance, as native;
the process retains the earlier instance/parent until its own retirement.

004E2000 reverse-stops recorded explosion identities before DM alignment.
It emits fire then smoke using the optional particle marker's world matrix or
Car frame world matrix, always parent Car, orientationtrue/flagfalse. Native1C/
20 publish after each call. Positive SID emits a fresh actual SOUND request:
flags1, relative optional marker/Car, group6, sourceNULL, entrySID and the native
remaining defaults. Actual shared SOUND and module17 perform the work.

004E0DC0 clears planes38, zones48, seats58, then reverse explosion stop/free and
marker08 NULL; the DM model releases afterwards. It deliberately leaves fire/
smoke instances process-owned.004E0EA0 clears/reparses, publishes countdown0 and
triggeredfalse, then stops old1C/20 in that order WITHOUT resetting their IDs.
The optional missing-DM model is a genuine clear/reset path, not a load failure.
That corrects the earlier cold adapter's overstrict missing-model guard.

## Confirmed unwritten native fields

004E09D0's raw allocation at004E0B77/79 allocates0x14 bytes through007E4F62;
004E0BAB/BB6/BBC/BD6 write offsets4/0/10/C, leaving **record8 unwritten**.
The container append copies the pointer, not another initialized record.
`human-car-dm-allocation.asm` shows the alignment wrapper calling CRT _malloc
007E81C7, with no record zeroing. Lookup/metadata/allocator index receipts and
`human-car-dm-allocation-base.asm` preserve this finding. The reconstruction uses
an optional identity rather than fabricating-1/0. If the actual active list is
empty, every possible native ID is a miss, so cleanup can prove no ownership.
With active records and no producer, cleanup fails explicitly.004E1F70 supplies
a real identity and permits normal positive retirement. Do not re-investigate
this confirmed omission without contradictory evidence.

004E08C0 similarly writes native10/18/1C/20=-1 but leaves **smoke definition14**
unwritten.004E09D0 produces14 when an actual DM model exists (PSMOKE/default95).
For a missing model, damage's !model gate and reset are genuine success paths;
positive destruction publishes the invalid-fire result, then rejects the missing
smoke producer at its actual emission boundary. No arbitrary effect ID is chosen.
These guards preserve required failures for undefined original input state.

## Verification and remaining dependency

Temporary `human-car-dm-presentation-native.cpp` extends the earlier damage fixture:
100 repetitions of packet part/glass/wheel mutation/authority cursor gates,
reserved kinds, positive DM explosion/fire/smoke/SID, repeated-handle ownership,
clear/reset and optional-model cases, failed stop and unwritten-field guards.
It uses actual CarAncillaryModel, part/body/wheel storage and stream/quantizers;
engine/geometry/coefficients/actor-policy/Human damage/terminal destruction/
particle/SOUND endpoints are explicit doubles. The terminal visual receiver is a
counter double, and the actual host retains its required failure.

Separately, `human-car-particle-retirement-native.cpp` exercises actual process,
module, instance and point owners: binary hits/misses/output indices, front/
middle/back stable deletion, immediate release without pooling and interrupted
close/retry across100 lifetimes. `human-car-dm-sound-native.cpp` uses actual sound
slot loading and puncture source with explicit engine endpoints. ASan/UBSan/LSan
results, strict actual full World i386 and final launcher/probe compile/link are
pinned by the checkpoint. Compilation/Ghidra remain sequential; no Windows run.

Continue004DBF00 authority destruction/nearby Human004DC580, then004DC780 and
reached004E9DB0 physical reset/004DCD10 mesh transition, Item12 detach, positive
glass/other required effects. Their indexed/new original evidence is in WORK
`human-car-destruction-dependencies-index.txt`,
`human-car-destruction-inner-index.txt`, `human-car-damage-presentation-workers`,
`human-car-destruction-mesh-workers`, `human-car-destruction.asm` and
`human-car-destruction-human-area.asm`. Full timed-DM frame, non7 entry, containing
Human frame/failure/re-entry/ordered cleanup and application acceptance remain.
