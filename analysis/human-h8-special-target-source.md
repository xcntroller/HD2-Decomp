# H8 specialized target22 and vector aim source

The actual Active frame now admits and progresses task22 (Car/Animation attack)
and0052B580 special vector aim, instead of stopping at the ordinary Human-target
guard. Reuses actual inventory, controls, target/direction/query, Car occupants,
model/head and Scene2 owners. H8 and the HUM matrix remain open.

## Original evidence

WORK=`analysis/work/mission-world-20261103`. Research-index lookups preceded
new read-only/noanalysis exports `human-h8-specialized-target/{0052b580,004ddee0}`
and `human-h8-special-weapon/00538980`; original program SHA remains
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Existing `human-h8-authority-movement/0052b180.c`,
`human-h8-command-producers/00535250.c`, task-execution005387C0 and authority
0052B810 exports were reused. Raw `human-h8-special-{aim,nearby-target,weapon}.asm`
and `human-h8-special-target-constants.json` resolve x87/order/ABI details.

## Recovered behavior and owners

0052B180 task22 computes cached Human30 minus target30, stores the FLOAT square
(Z²+Y²)+X², then reads the actual cached item's equipment command. Strict ordered
thresholds are400 for commands1/4,100 for2,2500 then1.440000057220459 for3,
1.440000057220459 for6,900 otherwise. TEST AH,5/JP rejects unordered here;
the decompiler's Boolean/popcount rendering must not become NaN acceptance.
Actual00538B00 visibility runs BEFORE query-state progression. Inactive query,
out-of-range/live target/zero210 may replan only when the actual target model
is within squared0.04000000283122063 of query10, retaining the extended
(Z²+Y²)+X² comparison. It writes206 before00534620, whose actual initializer
clears it again, then204. Active query in range returns NULL if invisible;
visible sets query2000 and task200. Return rereads task224 after callbacks.

The nearby Dog continuation now reads actual4A0 health, reverse swap/pops only
encountered dead Dogs inside radius, and accepts live Dogs below squared16.
Radius comparison retains the native extended sum (52B488..A2). Far positive
Dog listener540 still requires its actual live Sight owner. No universal absence
or empty private list is substituted.

0052B580 handles Car4 (`body`/FFFF), StaticWeapon21/Animation25 model and Dog30
(`c`/FFFF). Other kinds genuinely return without effects. Query order is target
child-find, Human260 world matrix, target world matrix, THEN actual hand holder3.
Missing target frame therefore retains the completed Human-head update prefix.
XYZ differences spill FLOAT; unwritten native W remains only a typed guard lane.
Non-grenade hand: actual direction0055D2E0 first, then00560780 vector aim, which
clears target actor/accumulators. Kind9 hand: task1E0-to-cached-Human distance is
extended (Z²+Y²)+X². Strictly above36 binds actual position aim and0075CC30
animation4F duration into214. Otherwise speech precedes206=true. Neither branch
invents a Human actor target or a model-position fallback.

0052B810 now runs its original special-aim outer branch for a nearby target or
active task22; it does not run ordinary target-selection/interval work in that
branch. Unavailable task22 writes200/206; common original tail still follows.
Pending00528950 now admits22 and uses its native target-unavailable completion
policy, retaining controlled-seat waits and existing204 group behavior.

00535250 task22 producer rejects the SAME current attachment. On foot it calls
00538980 and actual00538770 inventory selection/transfer before task publication;
transfer return remains ignored. Attached control98 must succeed. Armoured Car
5E0 or AnimationD8=2 additionally requires real attached B4 payload-kind2.
Admitted task22 uses actual target model, cached Human direction, radius5,
203=false, empty-order notification, then204=true,174&~4 and actual focus exit.
Rejection marks202/206 BEFORE speech. No fictitious ammunition or successful
transfer is supplied.

00538980 reuses005304B0(kind6,variant=false,special=false) for those armoured
targets. Other targets use existing actual0052F750 priorities and00530930 lookup.
**Raw5389FD copies the LOW BYTE OF THE OUTPUT SLOT POINTER into priority byte0.**
The i386 typed adapter preserves this dependency using the actual borrowed output
address, then passes the resulting Boolean to the existing priority owner.
This is not silently replaced with false/current18A. Host stack layout is NOT
claimed equivalent to the original frame/allocator/SEH ABI. The source exports'
untyped signature alone is insufficient to infer a meaningful alternate value.

004DDEE0 scans actual occupants78 backwards: skip NULL/dead, require actual
005A63B0 and not005A63F0; roles2/5 return0,4 return1,1 delegates005A8820(false),
3/unknown continue, exhausted returns2.005387C0 task22 performs this call and
**ignores its result**, then returns false. The call's actual predicate/side
reads are preserved; a constant false cannot replace the entire receiver.

## Verification and limits

Temporary native ASan/UBSan/leak fixture passed **3,804 checks** using actual
HumanAITargetLoading/HumanAIDirection and scalar owners, with explicit adjacent
World/inventory/frame endpoints. Coverage includes all supported aim families,
empty/ordinary/grenade hand, strict36 and range profiles/NaN, visible/invisible
query progression/replan/reset order, on-foot/occupied/armoured attack admission,
transfer-before-publication/focus, failure prefixes, and reverse occupant role/
health/control/suppression classification. Missing include and typed declaration
compile errors were corrected before the pass. Required malformed/missing frame
failures remain failures.

Actual full module authority/presentation strict i386 O0 compile/link and
launcher/probe build pass; Windows fixtures are never executed. Exact function
tracker and27 tracker tests pass. Temporary successful source/binary pairs are
fingerprinted then removed. Five new frozen partials0052B180/0052B580/00535250/
00538980/004DDEE0; no delegated callee credit or complete-function promotion.
Actual live Dog/540/controller/Body, task16 actor owner, retreat40, delayed
Players/selected non7/Remote and application/desktop remain explicit boundaries.
