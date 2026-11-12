# Car blast damage reached by Human/Mine reactions

P22-HUM reactions/damage/ragdoll remains open. This is source progress in its
reached Car dependency, with actual application loading/desktop acceptance open.
Original client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Evidence below lives in ignored `analysis/work/mission-world-20261103` (WORK).

## Identity, original evidence and ABI

`human-containing-car-blast-vtable.json` pins constructor004D0B60's00810778
vtable: D4=004DBA10, D8=004DBEE0. Blast006DDD40 deliberately skips the material
D8 for Car4; this work does not export/credit D8. Reused
`car-callback-world-damage-required/004dba10.{c,json}` has incorrect indirect
engine signatures. `human-car-blast-damage.asm` overrides those aliases:
004DBA10 is thiscall with SIX stack arguments and RET24: kind, FLOAT amount,
point, direction, source Human and an unused final word. In particular,
004DBEA4 passes **embedded Car+24E0** to004E1A20, not the Car physics health
receiver00500EB0.

Index receipts precede exports: `human-containing-car-blast-index.txt`,
`human-car-blast-dependencies-index.txt`, `human-car-dm-damage-index.txt`,
`human-car-damage-writers-policy-index.txt`, `human-car-scene-packet-index.txt`.
Reused policy/predicate/getter bodies retain their existing credit.
New rollback/read-only exports: `human-car-blast-damage-workers`,
`human-car-dm-hit-workers`, `human-car-damage-event-writers` (their export logs
contain no errors). Raw helpers are `human-car-dm-damage.asm`,
`human-car-damage-source-policy.asm`, `human-car-damage-wheel-hit.asm`.
`human-car-damage-events-pe.json` pins five event vtables and constants; event
kind count is DOUBLE00810860=12, denominator0080F748=2 and additive0080FB00=1.
These writer entries are absent from the frozen client inventory: **zero credit**.

## Shared actor, physical and DM owners

`GameBlastWaves` now calls004DBA10 through the actual Car actor, existing
loading lifetime/body/wheels/dirty flags, deformable parts and SAME ancillary
model. Its policy reads actual Sight friendship, Human health/B0/property,
presentation suppression and session side. The source identity24B0 is published
before Scene104 picking; early policy/destruction returns precede even borrowing
the initialized physical state. Frame/geometry endpoints never execute originals.

004DDE20 is called only for nonnull source faction19C=2. Offline7 scans seats
backwards and queries hostility with **source as observer**, then health>0.
Other modes require live occupant→005A63B0→!005A63F0→occupant/source005A8820(false)
sides unequal. Reads retain native order. A254A destroyed Car ignores kind2.
004D16A0 resets254A, and authority resets24B0=-1. The existing004D3970 feature
path now borrows that same actor flag. Native2350 is core22C0, the existing
solver mass_changed flag, published only after DM returns successfully.

004DBA10 uses actual Scene104 picking (same engine RVA110F0 used by menu
picking), root Car frame20, input point, direction*10, zero masks/remaining args
and a64-byte output record. It reverse-searches intact matching deformable frames,
subtracts `(amount - resistance28*.01*amount)*.2` from SAME health24, sets dirty
61C bit2 and **WORD** part38 bit2, then clamps negative health to0. Applied
resistance is SAME part28, distinct from current24 and reset source1C.
Type3 glass emits kind0 and full direction. Other parts emit kind1 and truncated
health; types2/6 use world position minus cached model-world position, X/Z*.2,
Y=1.5. Others use direction with Y+.1. Amount scaling retains native FLOAT spills
and the kind1 extra*.01. Event velocity is SAME body linear velocity minus the
native angular/offset cross components (X/Y cross stored before subtraction,
Z remains extended).004DB2B0 **only broadcasts**: it never applies locally.

Wheel iteration is reverse, skipping40000000, matching native frame148 or its
14C parent relation to the hit parent.004D3B70 returns2 immediately for5E0.
Direct hits use distance from live wheel40. Parent overlaps inverse-transform the
point through wheel148 and discard **X**, testing radial Y/Z. Tiny FLOAT bits,
near-one and sqrt gates precede distance<=radius1D8*.7. Resultbit1 and nonnegative
flags1FC emit kind3. No puncture/detachment is silently substituted.

004E1DC0 aligns the SAME optional DM model with two actual Car world reads,
Make quaternion→model18(position,quaternion)→link actual scene root. Reverse
HOR/VERT markers copy the matching Car short-name local quaternion: publish
source cachebit8 before Make, normalize target/cache flags, marker24, then model24.
No clone or independent Car model is created.

004E1A20 requires !DM64, actual DM model and kind4/1/7/2. It hides reverse
**EXPL_D records at28** (HOR/VERT are38), then enables reverse seat markers from
current occupants. Kind2 reverse-scans D_ZONE48 records for nearest squared
distance<=1e6 (equal distances replace the previous candidate). Empty zones return.
Other kinds temporarily show the model, pick from `(point-direction)-direction`
with mask40000, then hide it. A matching seat marker delivers actual Human D4
kind1/full amount/final0 through SAME guarded containing Human lifetime.

Other hits search reverse D_ZONE records. Unknown marker calls004E18C0, which
shows model/each occupied marker, ray-picks each marker backwards and delivers
Human kind1 using `point-direction` as the point; then hides model. A zone
subtracts unsigned material word28 from amount, stores the reduced FLOAT, then
truncates integer zone health minus that amount. Exactly0 health does **not**
trigger; a negative result clamps0, damages occupants FIRST, then destruction.
Kind1 first calls004E1820 reverse explosion-marker picking. A successful match
leaves the model visible (native early return). Already armed returns. A nonzero
delay updates minimum countdown, publishes record10 armed BEFORE kind5 wire;
failed wire keeps both. Zero delay/other admitted kinds call004E1FE0, whose
triggered64 publication is AFTER the actual Car destruction callback.

Seat retirement's engine stop is distinct from254A:004D9A40 passes
004F2530(false,!Car234F), where234F/core22BF is SAME finalizer.engine_transition.
The recovered source now uses that field, including the transition's stop path
when drive.running is false. The actual00518710 implementation is reused.

## Packet grammar and required remaining boundaries

004DB2B0 uses00475530(id,false): SAME module13 third stream, priority3 clamped
header2, physical Scene2 identity10. Four-bit event kind follows. Kind0 writes
part10/health10/direction(-1) even for positive health. Kind1 writes part10/health10
and, only health<1, both vectors with default primary(-1)/mask7. Kind3 wheel8;
kind4 no payload; kind5 explosion index4.004755B0 finishes the same stream.
No local presentation is applied by these producers.

Terminal004DBF00 and Car004DB360 presentation remain explicit required failures,
as do positive timed-DM frame consumption, mesh/variants/glass/puncture/detachment,
Car surface effects and other derived blast families. The actual blast path can
now reach and mutate Car/DM state and publish packets; **that does not establish
complete Car damage or a complete reaction frame**. The selected Human checkbox,
non7 entry, containing frame/failure/re-entry/retirement and P2.2 remain open.

## Focused verification

`human-car-damage-native.cpp` uses actual DM parsing/damage owner, actual
CarDeformableParts storage, CarBodyIntegration and CarWheelRuntime, plus actual
SessionBroadcastPackets/bit streams/quantizers. Repeated100 lifetimes cover
nearest-zone ties, zero-versus-negative HP, policy/seat/side order, direct and
secondary seat rays, positive part/glass/wheel events and cross-force fields,
countdown/armed publication, required timed/destruction/event failures and state
prefixes. Actual header/body compilation is checked separately against the full
World/Car/Human/Blast composition. Engine, geometry/physical coefficients,
containing actor/policy, Human damage and destruction endpoints in the native
fixture are explicit doubles. No original trajectory/equivalence claim.

ASan/UBSan/LSan, strict actual-host i386 and launcher/probe results are pinned by
the checkpoint receipt. Windows outputs are compiled/linked only. No original
EXE/DLL/Proton, application-world execution or desktop acceptance is claimed.
