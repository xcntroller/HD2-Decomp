# Car contact request and timed damage source —2026-11-12

Source checkpoint within the OPEN Human reactions/damage/ragdoll checkbox.
Wheel event2 and Human contact request9 now use SAME existing session streams,
Car damage receiver and actor/seat owners. Actual timed004E1990 reaches the
existing destruction body. Request10(fuel)/11(impulse), broader Car deformation/
occupied/full frame and Human entry/frame remain required. No mission loading,
original execution or desktop acceptance is claimed.

## Original evidence and native grammar

Client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK `analysis/work/mission-world-20261103` preserves index receipts
`human-car-{wheel-timed-next,wheel-human-writers,contact-request-grammar,
module-request-routing,module-authority,contact-countdown,authority-extra-branches}-index.txt`.
Existing004DB2B0/004DB2E0/00475570/004E1990 exports were reused. Bounded PE
`human-car-contact-request-vtables.json` pins Car00810778+1C→004DB360 and
+20→004DB760; event008108B0→004E0540/read and004E05B0/write; event0081088C→
004E0180/read and004E0270/write. Writers004E05B0/004E0270 and module004755D0
are absent from the frozen inventory and receive ZERO new credit.

New `human-car-contact-request-originals` has default aliases. Rollback-only
`ExportHumanCarContactRequestABI.java` exports corrected
`human-car-contact-request-abi`. Readers/writers and004DB760 are thiscall/RET4;
004D4EB0 is FASTCALL(self,index,velocity*,impulse*)/RET8. Authority indirect
occupant78 consumes just its index; a separately pre-pushed zero remains for
D4's final hit argument. The corrected D4 override includes receiver plus six
stack words, preserving kind5/amount/point/impulse/current driver/hit0.
Raw `human-car-{wheel-damage-packets,human-damage-packets,damage-request-receiver}.asm`
and indexed `scene2-packet-receivers.asm` resolve the remaining aliases. No
Ghidra project changes are saved, and Ghidra/compilation remain sequential.

004E05B0 writes kind4,wheel8,health10, then default-primary(-1)/mask7 velocity
only when signed health<1. The same payload helper serves existing broadcast
004DB2B0 and request004DB2E0; no separate event representation/queue is added.
Negative source health is not clamped into a different protocol: native bit
truncation and the signed writer predicate remain as original.

004E0270 writes kind4,target16,amount16 and SIX RAW FLOAT bit patterns: impulse
XYZ then point XYZ. These are not quantized vectors/directions.004E0180 sign
extends BOTH16-bit values;FFFF target denotes all seats. Amount sign/truncation
is retained, including negative values. Fourth vector lanes have no wire/native
producer and retain initialized source lanes. Presentation004DB360 still ignores
kind9 without reading payload; its grammar is not replaced by authority grammar.

## Shared publication and authority dispatch

004DB2E0 reads actual Car10 identity then00475570(id,false): module13/priority3,
request selection0072BED0 and target0/local=false before derived writer and
004755B0/0072BF90 finish. Mode&1 chooses existing request2BC, otherwise existing
third broadcast stream; priority3 clamps to2 in stream state. Target0 finish
clears active only; bits remain queued for the containing session. No transport
or immediate local damage is fabricated.004DB2B0 remains broadcast/third stream.

`GameCarCallbacks` borrows the SAME existing GameBlastWaves damage owner for
presentation, authority, wheel/Human publication and timed destruction. All
callbacks prepare before a single endpoint bundle publishes; failure leaves it
unbound. Matching unbind retires the entire borrower bundle. No model, actor,
physics registry, session stream or duplicate destruction owner is created.

Actual module13 authority004755D0 reads10-bit Scene2 identity. Reserved0..31
continues through existing005A5F80/Human consume/discard, including NULL. Larger
identities dispatch actor20. The shared typed helper now routes those actors to
actual Human authority or Car004DB760; previously every identity went through
the Human parser. Invalid required larger identities throw after identity read.
Other unimplemented actor20 receivers remain required failures. Presentation
actor1C routing remains separately bound to004DB360.

004DB760 source currently implements reached kinds2/8/9 and genuine defaults:

- Kind2 consumes the entire wheel payload BEFORE current-health admission. Positive
current230 publishes dirty2, puncture bit10000000 and integer→FLOAT health. At
health<=0 and field5E0==0, invoke SAME wheel698(velocity,NULL). There is no
presentation's mode&1 rejection in this authority receiver. The physical-owner
check stays at its actual5E0 read, after wheel mutations; Human9 has no global
geometry preflight.
- Kind8 writes actual2544=true BEFORE existing004DBF00(water=true). The source field
has no invented cold producer; publication survives a later failure.
- Kind9 all-seats iterates reverse/current occupants, skipping NULL and health<=0;
reads current driver0 anew for EACH actual D4(kind5,amount,point,impulse,source,0).
A specific target uses actual00475D50 command identity and performs no added
alive test. Existing generic D4 consumers apply their own actual rules.
- Other original default kinds consume only kind4. Required kinds10/11 explicitly
fail at the unimplemented reader/consumer boundary. Kind10 is FUEL004F2670, not
body health. Kind11 requires actual004D3D10 collision impulse/ownership behavior.

## Actual timed destruction

Car authority loading frame now calls actual ancillary004E1990 with the same
bound destruction borrower, after existing water handling and earlier required
frame gates. Triggered64 or countdown0 returns. Otherwise signed machine-wrapping
subtraction writes countdown BEFORE comparing<=0; expiry publishes countdown0
BEFORE actual004E1FE0→004DBF00(false). The existing64 flag publishes only AFTER
that receiver returns. A failed destroy leaves countdown0/64false, so the next
frame does not silently replay the expiry. Explicit004E1FE0 can still invoke it
again, following the original guard. Unreset constructor countdown-1 keeps its
native behavior; no positive/elapsed-zero gate is invented. The legacy cold-only
API still requires an endpoint for nonzero state and is not the live frame path.

## Verification and remaining boundary

Temporary `human-car-contact-request-native.cpp` passes ASan/UBSan/LSan100 focused
families through actual session buffers/quantizers/streams, shared Scene2 routing,
Car request consumers and ancillary metadata/countdown: modes1/2/3/7, wheel
broadcast/request round trips, raw FLOAT/NaN bits and signed16 values, authority
wheel mutation, reverse/current seat/source ordering, single dead-target call,
partial truncated packet/physical-owner prefix,2544-before-failure, genuine
defaults/required10/11 and
timer expiry/failure/no-replay. Actor, Human damage, detachment, glass, frame/math,
particle and final destruction endpoints are explicit doubles. This does not
establish original network/trajectory equivalence or complete Car frames. The
reused fixture environment is narrowed to these new families; prior unrelated
DM/presentation assertions are not replayed or claimed as a new audit.

Strict actual nested module13 authority/World/Car/Human/request/timer composition
and launcher/probe compile/link are recorded in the receipt. Windows outputs and
original binaries/DLLs/Proton are not executed. Application integration/desktop
acceptance remain separate.

Continue fuel10/impulse11 and their reached Car ownership/network consumers,
then deformed/global backup/occupied restore and the full Human entry/frame/
failure/re-entry/retirement. Preserve confirmed unproduced part5F9/wheel695,
EXPL record8 and absent-model smoke14; do not invent successful defaults.
