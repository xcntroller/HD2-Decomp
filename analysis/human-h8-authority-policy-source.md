# H8.2 Active authority helpers, event dispatch and medical production

Continues eb44e0b. Actual Active APIs now bind00537690/005376C0 temporary
mode expiry,0052D460 navigation gait,005325C0 direction preset,00526400 self-
treatment and00528570→005A8480→Active F4/00528580 event dispatch. Selected
0045BD10 action15 and00535630 medical task production now use the shared actual
005A7E00 holder/holster query. These source helpers are available on the SAME
owners; **the containing00526550 is still required and does not call them yet**.
Actual command15 production is reachable through the existing command dispatch.
H8.2/H8 and application/desktop acceptance remain OPEN; occupied/Car/seat/absent-
actor policy belongs to the other agent's H8.4 work.

## Navigation, preset and temporary modes

Mode1B4/1CC and1B8/1D4 expiry subtracts from every nonzero timer with DWORD wrap,
including initially negative values. Signed result<=0 clamps timer0 and mode-1.
No timer/mode store occurs for initial0.

0052D460 selects slow/fast from174 bit2,188 and native leader1B0 velocity/B0
movement mask. Actual query active state chooses native hysteresis: distance
strictly>36 fast, strictly<9 slow, middle/equality retain previous170. Nonzero
1B0 changes thresholds to2.25/.25. Distance retains extended (Z²+Y²)+X²; unordered
retains170. Query-active,202,18C,174 and189 gates publish query50 bit2000 before
overrides. Native1C0 velocity read and discarded stack arithmetic publish no
leader velocity or movement mask.180 override clears1B0 before stamina0/focus174/
nearby-actor244 overrides.170 stores before masked query54. No path movement or
occupied policy is fabricated.

005325C0 samples first pending order0C BEFORE actual item refresh. If189/202,
leader and formation exist, actual member-position getter consumes88 even when
unchanged; native unwritten leader/slots/member outputs remain requirements.
Model follows getter; borrowed formation/Human lifetimes are checked after lazy
frame callbacks. Preserve Z FLOAT difference spill, extended X/Y, DOUBLE5.0
vertical suppression and final FLOAT (Z²+X²)+Y² against strict49.

Item/E0/query/nav-posture/task/follow/distance gates choose actual flag134 preset1
or4. First-order kind1 reads the CURRENT first order44 after refresh, defaults
missing movement bits to4000, and honors10000 and own400 gates. Actual target128
actor/vector and198 choose keep-direction when needed; target vector squared
sum stays extended. Changed preset performs actual005A81C0 name lookup BEFORE
00563A20 publication; unchanged preset skips it. Retail debug receiver is RET;
nonoffline name transport remains required.

## Self-treatment and target medical task

00526400 preserves early234, posture bit1, signedB0+B0>=5000 and action52/206
gates. Raw code accepts5000 exactly, despite decompiler text. Ineligible action
sets234=2000; focus174 returns without a timer. Stat7 signed<10 sets60000;
actual signed health percent>=95 sets5000. CRT rand runs only after these gates.
Native divisor `(100-skill)/2+10` uses signed truncation and wrapped subtraction;
zero divisor retains an explicit native IDIV failure instead of host UB.

A successful modulo draws actual005A7E00 medical query: holder5 kind13 bypasses
BOTH holder and runtime-item cooldown, otherwise existing005A6730 checks reverse
occupied holsters, FRONT item only and unsigned runtime field14. Returns actual
item identity, including identity0; no artificial truth test loses it. No item
leaves234 unchanged. Positive subtype35 calls immediate00587540/DC with captured
item identity and secondary0, through existing common-command owners. Only AFTER
it returns does ENGINE RandFloat run (separate from CRT), then234 receives
FTOL(`RandFloat*4000 + wrapped signed(100-skill)*200`). Owner replacement/retirement
is checked before post-callback stores. No additional deferred packet is made.

0045BD10 action15 retains existing model/sort-first pointer-only ordering, skips
the target Human, and chooses the first actual medical-item holder.00535630
checks target signedEC>0 BEFORE own medical inventory. Failure queues existing
speech68/1/300/0. Success calls actual00534620 action15/disabled/radius1.5/notify1,
then focus exit uses saved20C and writes214=1000 only AFTER the immediate common
command returns. Existing arrival/readiness/common15/completion owners are reused.
Other specialized producers/effects and populated Players groups remain required.

## Active F4 and actual event ownership

00528570 delegates005A8480: pop actual command FIRST event, dispatch current
Active virtualF4/00528580, then retire or transfer the lease into actual SECOND
processed queue. Newly enqueued events drain in the same pass. No second pool,
queue or event copy is added. Native failed callback paths become checked owner
failures; scoped leases release safely on exception without claiming native SEH
or allocator ABI equivalence.

00528580 implements its native switch/default:10000002→19C10000/merge3000;
10000003→19C5000, clear source/target IDs, merge2000;20000004/12→19C10000;
30000001→19C16000 and repeated identity lookup.3000000B sets5000 and remembers
nonhostile resolved actors for10000.3000000C repeats identity/type/health/B0 gates,
reads actual selected D4 through B0 virtualA4 (Base/Player/Remote NULL), then
compares lazy model coordinates against strict900 in extended (Z²+Y²)+X².
BOTH matrix aliases remain borrowed across both updates. Near actual roster
member and action-mode0 select16 before19C10000/remember.

40000010 selects16 only from mode0, sets19C15000 and merges1000.40000011 does
mode first, remembers a resolved nonhostile for10000, and only on transfer writes
19C15000 AFTER merge.40000015 checks actual attacker-Human friendship direction,
reuses existing0045F110 same-roster nearby notification, selects16 from mode0/2,
queues speech6C/1/2000/0, writes19C15000 then merges6000. Callback failure retains
native earlier stores. All other events retire without new effects. Selected
Actor and Human/B0 owners are guarded across re-entrant model/mode/Sight/speech
callbacks. No H8.4 actor/seat policy is implemented.

## Evidence and verification

Original SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Research index preceded bounded read-only/noanalysis exports to WORK
`human-h8-authority-policy`:00528580/005325C0/005276E0/0052B810/0052E310/00478AE0/
005A7E00. Unimplemented005276E0/0052B810/0052E310/00478AE0 receive NO credit. Existing mode/self-treatment/task/gait, common event drain, health,
friendship and notification C were reused. No project mutation, original game
execution or H8.4 worktree changes. Raw authority-prefix/mode-expiry/preset/gait,
active-event and treatment-producer ASM settle precision, no-op and gate details.

Focused ASan/UBSan/leak PASS45900 checks across100 lifecycle matrices. Actual
resources/query/target/flag, formation/nav records, pooled pending orders,
ActorStorageOwner/holders/holsters, HumanColdCommandControl/event pool and queues,
mode/gait/preset/self-treatment/medical producer/event kernels run. Human/frame/
item catalogue/World/command/mode/Sight/name endpoints remain explicit doubles.
Includes hysteresis/equality/NaN/extended thresholds, vertical5 and49, stale
order-kind vs fresh order flags, changed88, name-before-preset failure, medical
identity0/cooldown/front-only/target-health short circuit, immediate command failure
before engine RNG/timer, full event lease transfer/re-entry and borrowed matrix
mutation regression. Strict actual-host i386 instantiates helper/event/medical
command bindings; final launcher/probe links only, no Windows execution.

[Receipt](checkpoints/p2.2-human-h8-authority-policy-20261116.json) records exact
fingerprints/results. Eight new partials; extend0045BD10. No complete promotion,
00528570/005A8480/delegated engine/new0045F110 credit. Temporary audits removed.
Next H8.2: actual populated group producers, other specialized tasks/effects,
remaining authority avoidance/decision/movement bodies and containing00526550;
then remaining offline receivers. H8.3 follows H8.2; H8.4 stays with its agent.
