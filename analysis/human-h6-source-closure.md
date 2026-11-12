# H6 Player items — source closure, 2026-11-12

H6's source boundary is complete on the existing owners, based on H2 `85593e8`
and completed HUM/H4 `b9d67aa`, in `feature/p22-hum-h6`. This closes the Player
item producers/receivers and their owned retirement; it does not close H8,
P22-HUM, the containing World frame or application/desktop delivery.
[Final receipt](checkpoints/p2.2-human-h6-source-20261112.json),
[dependency plan](p22-hum-h6-plan.md).

## Composition and earlier verification

- [Controller](human-player-item-controller-source.md): actual Player/Human,
  input Record, held actions, dry fire/reload, shot math/recoil and00542920's
  pending-message publication, with native100 lifetime checks.
- [Sound/actions](human-player-item-sound-actions-source.md): actual RuntimeItem,
  catalogue/cache/SOUND, melee/repair query and SAME first-queue commands, native100.
- [Bullet/light](human-item-bullet-production-source.md): actual ShotMgr record,
  pool/publication/retirement and item ammunition/clock order, native100.
- [Shell/capsule/grenade](human-item-projectile-production-source.md): native100
  shell and2505 capsule/grenade checks; actual H4 grenade notice is composed
  AFTER publication and BEFORE inventory retirement, preserving all merged owners.

Earlier documents/receipts describe their checkpoint state, including dependencies
that were open then. They are not additional current blockers. This closure is the
current boundary; unchanged kernels were not broadly re-audited.

Final actual-host i386 FULL LINK instantiates005433B0 with actual Player, inline,
World, RuntimeItem, inventory, input/queue, sound/FPV and ShotMgr types. The external
Runtime in this fixture is EMPTY: item attachment uses Actor98(seat) through the
existing H3/UI seat presentation owner, not a throwing Runtime declaration.
The World feedback overload supplies actual item24/28 through0058B6C0→B0/08;
completion's four-argument treatment dispatch now calls Player00546DC0 on this
SAME World, rather than an unbound Runtime endpoint. This verifies composition
and signatures; it is not execution of a loaded World or the Windows fixture.

## Immediate melee and hit zones

`007D4FD0` selects `00451230(undercut=false)`. Damage is primary payload native10
(serialized bytes0C). Direction XYZ is multiplied by exact FLOAT1.2000000476837158.
The real static/dynamic006F7C90 ray is called between inline005AC1C0(0) and
005AC1B0(0); restoration finishes before interpreting the borrowed hit descriptor.
Descriptor48 supplies the actor identity, independently of frameEC. Human3 and
StaticWeapon21's actual seat0 occupant use00455670. D4 receives kind3, damage,
IMPACT, SCALED DISPLACEMENT, source Human and hit-zone index. Human material D8
is the proven constant42. A miss calls005A5960 and returns before surface effects.
Other actors get Scene104 material picking, with no invented damage call.

`00455670` uses the SAME process ShotMgr humanzones model and borrowed10 frames,
with00455370's bone map `{1,6,13,14,9,10,20,21,16,17}`. A missing model or NULL
visual214 bone table genuinely returnsFF. Per zone, copy fresh bone world matrix,
GetScale, native dirty prefix, unit XYZ scale, lazy quaternion Make, final dirty
flags and frame24 update, then pick. Later zones remain untouched after an early
hit. At head index1, holder0's actual item with payload command0 first picks its
model using04040000; a hit returns11. Ordinary zone flags are08040000. SIMD W
scratch lanes remain unclaimed source guards.004558A0's diagnostic switch ends
in actual0069BFE0 RET; no additional logger service or function credit is claimed.

`005A5960` preserves B0 flags40000000/actual-player suppression before virtualB8.
The final original vtables/bytes prove common00574CB0 for AI, active0052E780,
Script00590380, and genuine RET8 base005831F0/passive005A43A0. Common production
reads source world position before acquiring the actual shared AI pool and writes
kind40000010, lifetime10000, age0, handles(-1,source), full position and word12=0
into the SAME first event queue; other pooled words survive. Active queues speech64
only when the actual controlled roster's LIVE count exceeds1. Script queues first,
then applies its interrupt-bit1 gate, publishes flags/mask, rereads source position
and interrupts the actual context. Displacement is genuinely unused by B8.

Melee material effects use actual material3C and module17 with impact/normal,
NULL parent, orient=true, final=false. No camera visibility gate from00451C00 is
introduced. Item12/Mine material receivers reuse their actual owners/constants.
Other actor D8 material and absent Dog30 D4/factory remain explicit H8 requirements.
No whole00451230 claim: its undercut=true branch is not selected by007D4FD0.

## Player medical completion

`00546DC0` keeps its distinct offline protection stage19/random prefix: draw CRT
only on mismatch, compare against exact FLOAT327.66998291015625, find target,
negate005A80E0 direction, obtain005A3CC0 bone point and apply actual H2 D4(kind6,
amount-1, source Human). Lookup is repeated after that prefix. Full-health/missing
target, absent/wrong-kind medicine and initially ZERO integer amount return false.
The initial gate is nonzero, not signed-positive. Native signed __ftol arithmetic,
wrapping health, repeated amount read on clamping and FLOAT healed-amount spill
are retained. Subtract cost then add health; snapshot exhausted status BEFORE
notification. Offline medical skill precedes actual inherited00588870 notification.
Non7 produces kind14 on the SAME FIRST90 queue; it is not transport/admission.
Exhaustion retirement follows notification/queueing and uses saved identity. Late
non7, nonself score uses FLOAT healed amount times exact FLOAT.004000000189989805
and actual0072EDD0, preserving its locks/records/rank policy. Base00588AA0 is unchanged.

## Verification and remaining boundaries

New native C++17 warnings-as-errors ASan/UBSan/LSan PASS1600 over100 medicine
lifetimes with actual RuntimeItem/catalogue/storage, authored definition bytes,
zero gate, healing/cost/clamp, notification failure/item mutation, protection
relookup and kind14/score production. Player/World/Human/notification/damage/score
endpoints are explicit doubles in this policy fixture. New PASS2000 over100
actual ShotMgr hit-zone-state and AI pool/queue lifetimes checks missing resources,
ordered bone/helmet picks, early exit/failure, melee restore/damage/effect order,
active speech and Script event/interrupt/return. Engine/actor/state receivers are
explicit doubles there. Existing H2 damage audits are retained, not rerun as a
new claim. Strict actual-host FULL LINK and latest launcher/probe rebuild pass.
No original EXE/DLL, Proton, Windows fixture, application or desktop execution.
New temporary audit sources/binaries are fingerprinted in the receipt and removed.

H8 still supplies full Runtime/World scheduling and async ShotMgr combat bodies:
0044B910/0044B400 bullet,00454360 shell,00449290/00449030 capsule,
0044D260/0044D240 grenade and0044D570 rigid contact. Their required guards remain,
including attached bullet/capsule initial100 calls where selected. These are not
successful idle simulations. Non7 bullet producer00450D30 and full feedback/input
history/replay/admission remain conditional OWN/H8/Phase3 requirements. H6 only
supplies the named pending/packet producers. Existing Car D4/D5 and entry D6–D9,
confirmed D11 gaps, absent actors and broader H4 Runtime boundaries remain open.
H8 must complete/prove-out/report these before claiming a playable containing path.

Five new frozen partials only:00451230,00455670,00546DC0,00574CB0,005A5960.
No new credit for absent0052E780/00590380/005831F0/005A43A0, debug004558A0,
Human material D8 constants, new wrappers or delegated LS3DF functions. Typed
ownership/STL/RAII replaces original class/vtable/allocator/SEH ABI; no full-body
or trajectory-equivalence claim. Parent delivery checkboxes stay open.
