# Car frame events, inventory and pending damage —2026-11-12

D4a is the bounded source receiver checkpoint, not completion of D4/H2 or a
running Human/Car frame. D4b explicitly retains the actual AI command admission
and reached occupied/non7 consumers; D5/D10/D12 and application/desktop remain.
The [Human dependency plan](p22-hum-plan.md) defines their exit boundaries.

## Evidence and original ABI

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK=`analysis/work/mission-world-20261103`. Index receipts `human-car-frame-events-index.txt`,
`human-car-authority-frame-abi-index.txt`, `human-car-frame-consumers-index.txt`,
`human-car-periodic-inventory-index.txt`, `human-car-inventory-factory-index.txt`,
`human-car-inventory-transfer-index.txt`, `human-car-periodic-reset-deps-index.txt`,
`human-car-reset-target-producers-index.txt`, `human-car-target-item-retirement-index.txt`,
`human-car-target-action-index.txt` and `human-car-target-model-index.txt` locate
existing evidence before new exports. Indexed candidates are not equivalence proof.

Rollback-only `ExportHumanCarFrameEventsABI.java` exports read-only Ghidra results
under `human-car-frame-events-abi`, `human-car-frame-reset-target-abi` and
`human-car-target-action-original`; temporary signatures are never saved.
004DEAD0's actual return is AL Boolean (raw RET16), despite the initial temporary
void signature.004DEB50's old export loses its Human argument and wrongfully
renders unaff_EDI; raw stack accesses control interpretation.

Bounded raw: `human-car-periodic-event-reset.asm`, `human-car-event7-envelope.asm`,
`human-car-targeted-damage.asm`, `human-car-pending-producer.asm`,
`human-car-target-presentation.asm`, `human-car-reset-suspension.asm`,
`human-car-navigation-frame-tail.asm` and `human-car-event6-workers.asm`.
Bounded PE reads identify Car virtual44 as004DD0D0; event6 table00810868 has
004E0760/004E0770. The latter two are absent from the frozen inventory: no credit.
Earlier guessed004E0020/0030 addresses are not these workers.

## Authority event6 and actual reset

004D2790 first invokes the existing0073EA90 clock owner. Expiry refreshes2470
using its second clock read before any event publication.004DB2B0 publishes
kind6; only AFTER publication reread mode2 and conditionally004DB2A0. Clear2478
only after both succeed. A publication/reset exception retains2478 and the
already advanced clock. Kind6 serializes only its four-bit kind.004DB360 receives
it without payload or physical/DM lookup, then004DB2A0 calls virtual44(true).

004DD0D0's selected offline7 reset borrows the existing activation/body/wheels/
collision/mesh/Door/particle/skid/navigation graph.004DCE40 runs first: reset DM,
clear254A, optionally reload the original model basename, reverse-detach occupied
frames, enumerate actual visual meshes using004DCFE0, update model, reverse-link
current occupants to actual seats, clear2548 then refresh reverse part mesh8.
Missing factory or rejected load returns from004DCE40 before its physical reset;
the containing004DD0D0 still continues. Otherwise004E9DB0(0,true) restores the
same geometry/resource backups and Door visual.004D25C0 now shares this prefix.

For true/authority/offline7, proximity registration overwrites24DC only AFTER
success. Native does not remove an older row here; the SAME process remains its
owner. Ascending seat removal precedes each nonNULL mounted004C6620 consumer,
still explicitly required under D5.004E95B0(2) then:

- Ascending wheels/reverse skid pairs: clear active8, hide first visual only.
- Transform angular210 then linear410 by cached inverse rotation350; these are
  real motion mutations, not unused scratch expressions.
- Reverse standalone projectors/light links, then paired projectors, unlink0.
- Ascending wheel effects, reverse exhaust effects, damage effect; disable actual
  collision groups, free contact798 records, reverse impact effects, stop81 sounds.

Republish saved position/direction, native position-lock-aware XYZ and model24;
read camera setting264, select special collision, body flags1, physical reset
(0,false), actual5ms settling/finalization, fuel1000 (upper-clamped by capacity).
Then source24B0=-1, driverNULL, water latch2549=false, navigation refresh, release
model254C and reset2554/2550. Non7 behavior remains required at the existing
mode guard; the bit1 permanent skid-detachment caller of004E95B0 is excluded.

## Event7 and SAME Items ownership

004DE1F0 RET8 consumes unsigned identity16. FFFF ends immediately. Each remaining
entry consumes definition8, optionally calls007D7810(definition,NULL actor,slot0,
identity), THEN rereads actual Car74/first storage and invokes its2C insertion;
then reads the next identity16. No preliminary inventory clear and no transaction
over the whole packet. Truncation/factory failure preserves prior entries/cursor.
Apply=false consumes all entries without factory or storage access.004DB360
captures apply=!(mode&1) before calling this receiver.

007D7810's no-source/no-destination slot0 transfer is the actual007D8B40 branch:
new constructors publish1C=false, no holder/weight/178 change or placed-world
transfer; publish identity and field14=game28+920, then register in SAME Items.
The real Car's ActorStorageOwner/ActorSlotStorage receives the entry. NULL factory
result is passed to insertion; a full container rejection does not release or
unregister an item. Storage retirement and module teardown retain their existing
ownership. No fake Human, parallel inventory, dynamic Item actor or successful
transport is introduced. Model/catalogue/engine and containing admission remain
the actual existing module dependencies.

## Pending damage and presentation

00586200 subtype14 is the original caller of004DEAD0. It first performs AI/item/
target admission, actual transform generation004DEC60 and004DDCF0 gating. These
containing command consumers remain D4b/H4 work, not completion from a direct call.
The exposed GameBlastWaves receiver uses actual Human and Items borrowers.

004DEAD0 RET16 returns false for NULL item/Human or an already armed countdown.
When mode2 is set,004DEB50 first stores both2560/2570 vectors, borrows SAME
007DF750 definition template, creates the matching visual with driver50, publishes
254C BEFORE copy4C (status ignored), consumes Human inventory by identity through
existing005A7B70, links to Car frame and sets14 pose. NULL factory is published
before the required failure. This owned reference is the same one retired by
destruction/reset. After presentation succeeds, publish15000/source Human ID;
REREAD mode2 before the nonpresentation inventory consumption. Thus presentation
failure leaves the timer unarmed; a later consumption failure retains it armed.

004DEC10 performs signed wrapping elapsed subtraction. At <=0 publish2550=-1,
source24B0=2554 and flag2544=true BEFORE existing004DBF00(false). A destruction
failure cannot replay the timer.004D2790 places this before water and timed DM.
Cold2550/2554 now live on the actual Car actor and are produced by activation/
reset; the old private misnamed loading scalars are removed.

## Navigation correction and verification limits

004D3336..335D fetches game54 and invokes0051D5A0 on reverse259C identities,
discarding every return. This is navigation, not SOUND. CarNavigationLoading
retains each actual identity with its existing marked record and performs the
observations before final model20 tick. No unknown voice update is fabricated.

Focused temporary native audits cover100 event/presentation families (actual
packet/storage source, explicit item/clock/reset/damage/visual endpoints),100
actual event6/7 dispatch families plus existing100 fuel/impulse regression, and
100 restore/suspension families over actual body/skid/light/particle/contact
source with explicit DM/model/frame/collision endpoints. Private access only
seeds known coefficients/prior skid storage in the reset audit; no producer claim.
Native checks do not run original code or establish whole-game trajectory parity.
Strict nested actual World/Car/Human/module13/Items/target receiver i386 and
launcher/probe compile/link are separate checks; no Windows output execution.

Reached EXPL8/smoke14 and detached part5F9/wheel695 remain honest D11 failures.
No unproduced state, positive occupied/mounted reset, AI subtype14 dispatch or
non7 service is made successful by this checkpoint. D4b/D5/D10–D12 and actual
application/render/user desktop acceptance remain open.

## D4b caller research after the receiver checkpoint

No additional implementation or function credit. The new index receipt is
WORK `human-car-target-command-deps-index.txt`. Separate rollback script
`ExportHumanCarTargetTransformABI.java`→`human-car-target-transform-final` retains
004DEC60/default00586200; the earlier `target-transform-original` is superseded.
The frozen D4a script and every receipt fingerprint remain unchanged.

Raw `human-car-target-command-subtype14.asm` verifies0058622E..0058631D:
refresh actual held item first, resolve target identity, reject NULL target/item
or definition virtual10!=0B, borrow Human260 head world position and B0 target
direction, then004DEC60.004DDCF0 is simply actor6A0==0 (the current Car body
health), not an unknown general admission service. With mode1, zero health
returns false; otherwise call004DEAD0. Without mode1 call004DEB50 directly.

Raw `human-car-target-transform.asm` verifies004DEC60 RET16: multiply direction
XYZ by FLOAT10 from bounded0080FA14, call actual scene104(root=target frame20,
flags08040000). On a hit, native details0 supplies distance and details10/14/18
normal; position is distance*UNSCALED direction+head, with a separate Z product
spill, and second output is negative normal. Fourth output lanes copy unproduced
stack padding; no geometric claim is made for those lanes.

On a MISS the helper leaves BOTH output XYZ vectors unwritten. Caller005862AD
does not test the returned Boolean:005862B2 immediately tests global mode and
continues to the receiver. D4b must preserve that missing-output boundary rather
than choose a guessed attachment point. Actual AI command routing also remains
open: the supplied packet receiver selects Player/Remote B0 protocols and cannot
be relabeled as the missing AI virtualDC dispatcher. This research does not
close D4b or bypass the separate H4/state ownership boundary.
