# H8.2 ordinary on-foot task execution — source checkpoint

Continues529b9a9. Pending kind3 target tasks can now use selected ordinary
00528950 startup/arrival/turn/effect/wait/completion phases on SAME Active B0,
124 direction,128 target,12C query,130 path and World common-command owners.
H8.2 and the containing00526550 remain OPEN. H8.4 belongs to another agent;
this checkpoint does not implement its occupied/Car/seat/absent-actor policies.
After H8.2, this branch proceeds to H8.3, leaving H8.4 separate.

## Selected task progression and effects

`human_active_ai_pending.h` accepts on-foot0/1/2/3/4/5/14/15/19/27/28/33/34/38/52
through the shared move prefix. This does not make all their producers/effects
complete.22/25/26/32 and other specialized families retain required failures;
any Human160 attachment retains the H8.4 occupied dependency. Startup preserves
205 BEFORE actual readiness clear, query request,174 clear and160 publication.
No synthetic movement is applied when a navigation request is submitted.

Arrival first fetches the actual Human model and FLOAT squared distance/radius.
Typed224 arrival is tested before the radius alternative. Arrival cancels the
query and writes200, then returns; turn/effect is a later frame. Expired224
requires target-event policy before access.201-set tasks retain the existing
separate completion phase and actual00535C00 pending/standalone policy.

`human_active_ai_task_execution.h` binds00535E70:

- NULL target genuinely returnsfalse. Human3 reads Human model before target
  model, retaining the borrowed matrix aliases across BOTH lazy updates, then compares extended (Z²+Y²)+X² against2.25 with NO FLOAT spill;
  equality and unordered fail.
- Door6 uses SAME A0 navigation/A4 point and sorted SECOND-family0051E520
  reference lookup. Actual typed-array bounds are checked without subtracting
  unrelated pointers. No model is read on this branch.
- Other selected non-Car targets read the Human model, target world matrix,
  actual engine inverse/point transform, then driver+A8 bounds (dummy6 uses120).
  Local AABB residuals accumulate X,Y,Z in extended precision, strict<2.25.
  Native unordered coordinates contribute no residual; no invented validity
  filter changes that behavior. Engine inverse/transform/bounds are delegated,
  receive no body credit, and failed inverse remains a checked endpoint failure.
- Type4 body-frame selection remains the reached H8.4 requirement before Human
  model access. It is not replaced with the Car's root frame.

## Turn and readiness ordering

201-clear chooses cached target224 position or task1E0 relative to cached Human
position, spilling desired X/Z to FLOAT. Target-present W comes from cached
Human W (raw528E9F); NULL224's native W temporary is unwritten, so the adaptation
retains160 W with no native-value claim.209 instead copies all1F0 into160 before
flattening the temporary Y.160 otherwise stays unchanged. Desired normalization
then actual target/current direction precede a fresh Human model forward read.

Forward XYZ normalization stores its native FLOAT results. Its SECOND horizontal
normalization retains X/Z products in extended precision until the dot; Y alone
spills. The final dot spills to FLOAT before comparison with native.95. Unordered
also waits. Actual DD0 action gate005AC2B0 can block turning unless real query50
bit10000000 is set;207 bypasses both angle/action gates. Only then201=1 and214=1500
publish.174 bit200 returns before task effects. Bit8 close/equal/unordered distance
publishes206 then rejected speech; its far00560750/005AC200 ranged branch remains
required after the earlier201/214 stores. No invented ranged effect is supplied.

Actual005374A0 mode mapping comes from raw5375F8/53765C jump/byte tables; C
omitted every00538550 argument. It calls the existing SAME-state second-mode
setter, preserving1D4/1B8 behavior and default holes.005387C0 then uses actual
Door locks, ExplosivePlace94, target Actor00538A60 capability, Human availability/
friendly predicate, Door phase and DocumentPlace148 where selected.006029F0 is
**locked ?4:Door224**, not Door94.00538A60 receives Actor ECX: Human signed healthEC
and genuine defaultfalse are bound; type4/type25/type30 remain H8.4 requirements.
Target16 capability and reached Car22 reset remain exact requirements as well.

A failed readiness test queues actual Active speech68/1/300/0 after second-mode
publication, without completing the task.204 retains native branch order:
32 returns; otherwise214=0 before208's required Players00456F00 call. Action3's
attachment seat selection is the reached H8.4 boundary before modes. Specialized
6/23/24/33 effects still stop after their native mode/readiness prefixes.

Other selected ordinary/default effects call actual00587540/DC immediately on
the SAME state factory, passing action220, saved identity20C and secondary0.
No deferred command copy is added.214=1500 precedes the callback. Only after it
returns, B060!=-1 replaces214 with actual B064 duration. Callback exceptions leave
the initialized201/mode/timer prefix intact. Later214 expiration drives existing
00528950 completion; normal pending processing can then append hold before lease
retirement. Common packet source is reused, not a second implementation.

## Evidence, verification and limits

Original SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Index was consulted before new bounded read-only/noanalysis exports:
WORK `human-h8-task-execution-tail`005374A0/005387C0 and
`human-h8-task-door-ready`006029F0. Existing00535E70/00538A60 are reused from
`human-h8-command-execution`;00528950 from `human-h8-active-frame-workers`.
No ERROR, saved project/type mutation, or original execution. Raw evidence:
`human-h8-task-execution.asm`, `human-h8-task-target-arrival.asm`,
`human-h8-task-ready.asm`, `human-h8-task-second-modes.asm`.

Focused ASan/UBSan/leak audit passes29500 checks across100 lifecycle matrices.
Actual AI resources/query/path/direction, task initializer, MissionNavigation/
records/references, Door properties, task/turn/effect and completion kernels run.
Human/frame/World/readiness/common-command/name endpoints are explicit doubles.
Mode assertions use an oracle extracted from ORIGINAL jump tables, independent
of reconstructed source. Covers native ordered/unordered/precision boundaries,
Door identity, bounds/inverse call ordering/failure, target/current-before-model,
borrowed model alias updates, W provenance/209/raw160 preservation, action gate/query bypass/207, second-mode
before readiness, callback failures, no extra queue, duration overrides, real
multi-frame on-foot task progression and reached H8.4/specialized failures.

Strict actual-host i386 instantiation binds pending/task/completion and existing
packet/common path to concrete World/B0 owners. Final launcher/probe, tracker and
ledger results and fingerprints are in
[receipt](checkpoints/p2.2-human-h8-task-execution-20261116.json). Windows fixtures
are linked only; temporary audits are fingerprinted and removed. Five new
partial bodies and extended00528950, no complete promotion or delegated credit.

## Next H8.2 work

Finish specialized task/effect/producer families and actual00456F00/00457820
Players receivers, full00536120 follow, then containing00526550 timers/
attachment-aware dispatch/navigation/aim/movement/output/events/self-treatment.
Authority still routes to the existing required Runtime boundary; this checkpoint
does not enable application loading. Existing authority C4029bytes and timer
workers were indexed/reused, not exported again. H8.4 is assigned elsewhere;
its requirements remain visible at reached calls. See [plan](p22-hum-h8-plan.md).
