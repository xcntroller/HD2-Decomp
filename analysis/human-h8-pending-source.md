# H8 Active pending orders and move52 — source checkpoint

PACK was already finished, as confirmed by the user on2026-11-16. This work
resumes **H8.2** after51082a2. Full H8.2/H8, containing00526550 authority and
application/desktop acceptance remain OPEN. This is a focused source checkpoint, not a new PACK gate.

## Actual pending owner and calls

`human_active_ai_pending.h` supplies005282E0 processing and00529F60 setup on the
SAME Active state, process order pool,250 sequence,124 direction,130 path,12C
query and current-action leases. No additional pending queue or World is created.
`HumanActiveAIStateLoading::process_pending_orders` exposes the concrete receiver;
it does not activate the incomplete containing authority frame.

An empty list clears only178/180 and returnsfalse. A changed first record clears
its49 before18E. Setup borrows the actual record across callbacks. Processing
then **rereads the first native pointer slot**, including the retained process
record after kind4 regroup/reset; it must not cache the old lease or assume the
list stays nonempty. Action activation precedes record49=1. On48 completion, a
single non4 order appends its actual hold BEFORE virtual12C retirement. Kind1
uses stored direction20; other kinds use a fresh model forward vector, normalized
with native extended unit comparison and FLOAT length before sqrt. A second
fresh model lookup supplies the hold position. Normalized XYZ and model W are
retained independently. Multiple orders retire first without appending a hold.

Setup1/2 writes202 before target model lookup and actual00534340 task52 setup.
The initializer clears202; then204/201 become1,174 bit1 clears and first mode
becomes10/11. Target40 supplies actual model position; NULL target uses stored30.
Kinds6/7 publish202 and mode13/14. Kind4 uses actual positive regroup and returns
before shared flags. Kind3 requires actual Players00458E50 dispatch/filtering;
its mandatory World receiver throws explicitly rather than accepting the order.
Other native kinds retain the common default-flag path.

Common flags mutate the real record: default posture4, movement4000, mind200.
Active178/180 publish their masked values; B050 uses native maskFFFE08FF with
**only mind700**, not the full saved order flags.130 movement publishes before
second mode11/10/9, then first-mode posture16/17/18 and actual005767F0 when
needed. Started2/6/7 refresh direction target, current, then raw160 only with both
198/19C zero. Other started kinds drive task decisions and set48 only after a
false result or206 completion; required exceptions do not complete the record.

## Positive order-generated on-foot move task

00528950 now supplies its delayed/inactive gates and **action52 on foot with
NULL224**, as produced by setup1/2.18D bypasses214 untouched.214 stores wrapped
subtraction, returns while signed positive and clamps a negative result. Inactive
202 handles17C posture only when it differs from B050, clearing17C after the
actual request; an already matching17C remains.174 bit20 controls direction
refresh, with native19C/198 gates.

The positive on-foot path sets205 BEFORE0058C400. That receiver uses the actual
E0 readiness clear and B0 feedback. It then requests real12C destination1E0,
clears174 bit1, computes160 from ActorBase cached world position minus1E0, and
publishes first mode4/5 from188. XYZ differences spill individually; squared
length spills to FLOAT before normalization's unit test. Native temporary W is
unwritten on this path: the adaptation retains160's existing W, with no native
value/equivalence claim for that lane.

On later frames,205/nonarrival200 reads the actual model position and compares
FLOAT squared distance against FLOAT radius21C². Equality and unordered do NOT
arrive. Arrival cancels query via2000 and writes200, then returns: completion is
a **separate later frame**. With201 set,206 short-circuits repeat completion;
otherwise218 posture and174 bits8/200 retire in native order.203 controls the
early202 store,206 follows, and actual00535C00's pending prefix stores202 before
its nonempty-list return. Pending processing can then append hold and return the
old lease. No navigation movement is synthesized merely by requesting a path.

Other actions, task targets, occupied move/vehicle policy,201-clear turn phase,
and standalone00535C00 completion still have precise required failures. Full
00528950 and00536120 follow/decision work remains necessary. The inlined pending
535C00 prefix receives **no additional body credit** at this checkpoint.

## Borrower retirement

The existing late ActorBase→composition→B0 retirement callback now also reaches
Active order/issuer/task borrowers. Order records retain separate base addresses
so the callback does not dereference an already destroyed Actor. Live/retained
pending slots clear matching issuer/target pointers; an expired required target
is flagged and fails before model access or action acquisition, rather than
silently becoming an authored NULL target. Pool acquisition resets this flag
and both base-address borrowers. The existing Base target invalidation still
runs first. These are defined native ownership protections, with no new original
function credit or claim that wider target/AI event producers are complete.

## Evidence and verification

Original SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Index was consulted before new exports. Existing originals:
WORK `human-h8-active-frame-workers/005282e0.c`,00528950.c,
`human-h8-active-task-workers/00529f60.c`,
`human-h8-pending-players/00458e50.c`. Saved raw
`human-h8-active-pending-flow.asm`, `human-h8-order-start.asm`,
`human-h8-active-move-task-prefix.asm`, `human-h8-active-move-completion.asm`
resolve ECX, native order publication, FLOAT spills and phase boundaries.
00535C00 was exported once, read-only/noanalysis, to
`human-h8-active-pending-complete`; no ERROR or saved Ghidra/type mutation.

Focused ASan/UBSan/leak checks use the actual process order pool, pending list,
AI resources/direction/path, action pools/activation, task initializer and
regroup/follow cancellation. Human/model/World/Players/posture/readiness endpoints
are explicit doubles. Includes default/mixed flags, direction gates, target
position, action pools, hold-before-retire, retained-slot regroup, live pooled
reuse after a callback, null/positive selected regroup, callback failures,
late expired-target access, timer equality/overshoot, query/readiness prefix,
move/arrival/later completion, radius equality/NaN and all selected required
failures. Native counts and concrete host/launcher results are in
[receipt](checkpoints/p2.2-human-h8-pending-20261116.json).

The strict actual-host fixture explicitly instantiates Active pending and task
receivers plus the complete existing packet path, actual nested World/B0/DD0/
E0/posture/formation/Players/HUD/equipment owners. Windows fixtures are linked,
never executed. Temporary audits are fingerprinted and removed. Ledger claims
are bounded partials at005282E0/00529F60/00528950, not full ABI equivalence or
source closure of all actions. No application loading or desktop test occurred.

Next: complete actual00458E50 filtering/command producers for kind3; remaining
00528950 task families/turn/occupied/standalone completion and00536120, then the
reached00526550 containing chain. Human190/route74 and actor lifetime event
producers remain required. PACK integration is already finished; do not return
to its deleted worktree or reopen its completed requirements.
