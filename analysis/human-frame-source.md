# Human frame execution — source checkpoints

Continuation of FULL first P2.2 checkbox after Sight checkpoint595f195. Current
source covers the0059FE30/0059FFD0 containing order, pooled perception/commands,
E0 directions/feedback, item poses,230 equipment and periodic stat updates.
[Verification receipt](checkpoints/p2.2-human-frame-20261110.json). Required derived
B0, DD0 presentation/control, item fire, positive blood/contact/formation and
005647C0 movement receivers remain. World services expose the SAME owners, but
the live full frame graph is NOT instantiated. Application still declines loading;
no actual mission frame or desktop claim. Both P2.2 checkboxes remain OPEN.

## Original boundary

Human presentation virtual14=0059FE30 (absent frozen): unconditional timer17C,
254/flag214 and nonoffline death/respawn work precedes registered14>=0 virtual64
004D0E80. Active path clears178, refreshes hand item005A6A90,005A0480, actual B0+24,
current48/position30/target4C into inline98,005ABC20, motion005A0660 when !mode1,
and B0+80 with actual inline state005AC0D0. Do not turn offline B0/passive no-op
entries into generic successful update callbacks.

Authority virtual18=0059FFD0: registered gate, item refresh, timer005A0420,
perception005A50A0/005A5380, offline roster alignment+.5, command queues005AD700,
B0+28, native authority/offline directions,005ABC70, mode2-clear B0+80. Alive tail
samples motion, actor contact query/damage, offline kind1 periodic stats and
formation update. B0/update/DD0/230 and positive collision/formation receivers
remain actual required bodies. Render/fixed/update lifecycle order stays unchanged.

## Recovered helpers

HumanActivationFields now types154 as FLOAT,17C as DWORD timer plus cold180/184
DWORDs. Original0059F2C0 zeros all three; the prior rotation17C label was misleading.
005A0420 zero-extends elapsedDWORD, subtracts elapsed*FLOAT.001, stores154, then
compares extended result and clamps below0 only. NaN remains unordered.

005A0660 preserves previous velocity1C0, old model position1D0 and actual current
model world pose before difference/scaling. Native difference is OLD-minus-NEW;
scale008112C0 is NEGATIVE1000/UNSIGNED elapsed, not positive1000. Extended square
compares0080F948 FLOAT1e-16. No elapsed clamp: nonzero movement with elapsed0 has
original floating infinities/NaNs. Attached path zeros both XYZ and does not query
a model; native W stores are unwritten stack values, so source retains them without
credit. Unattached copies preserve all four original lanes.

005A5380 expires actual Human2AC pooled records in reverse. SIGNED wrapping
(timeGetTime - timestamp)>3000, strict. Return to SAME process pool BEFORE vector
erase/compaction; survivors retain order and refresh producer's timestamp basis.

005AD700 advances FIRST command deque then SECOND; each reverse. Age+=elapsed,
lifetime-=elapsed DWORD wrap, signed lifetime<=0 returns SAME record to process
pool BEFORE erase. Key/priority/other event fields remain intact. Unknown raw
queue producers still reject; actual typed Sight/sound events now have positive
ageing/expiry, no successful empty substitute.

0046F320 wrapper→Human0046DB60 query (latter absent frozen): mode0 returns actual
target marker count, previous count=same and visible=true. Other modes return
relation counter1, previous2, BYTE3 feedback; no flag24 suppression. Actual Human
observer/indices are required.005A50A0, only alive Humans: collect dead hostile
indices reverse, then neutral reverse. Dead/now-hostile neutrals enqueue; friendly
neutral queries actual Sight feedback and applies alignment-1 if visible. Then
queued indices reverse, loss BEFORE acquisition, fresh actual feedback receivers.
This preserves the collection boundary when callbacks mutate perception lists.

005A60D0 alignment update uses actual offline7/game34/174 gates, holder order
0,4,3,2,5 and repeated entry getter before descriptor44. An occupied descriptor44=0
resets16C. Otherwise add requested amount, upper clamp100 only; negative values are
retained. HumanWorldLoadingServices binds actual item descriptors, shared Sight,
real timeGetTime and command owners; top-level frame invocation remains required.

## Focused checks and next work

PASS two temporary native ASan/UBSan/leak fixtures: countdown/NaN/unsigned elapsed,
model movement copy/W/attached/zero-dt behavior, pooled perception strict/signed-wrap
expiry, two queue LIFO/age/expiry order, query feedback modes/BYTE, queued perception
callback mutation/order and alignment holder/gate/clamp behavior. Geometry and
actor feedback leaves are explicit doubles. Strict actual-host i386 instantiation
passes real Human/World/Source/item/command/perception receivers. App build is
recorded in frame-human-samples-application-build.log. No Windows/original game/DLL/
Proton execution. Temporary fixtures retained until containing Human checkpoint;
no permanent tests. Later checkpoint checks and ledger scope follow below.

Original exports/static evidence under ignored durable
analysis/work/mission-world-20261103: frame-human-update-dependencies,
frame-human-state-updates; frame-human-presentation.asm, frame-human-countdown.asm,
frame-human-motion-samples.asm, frame-human-sample-constants.json;
frame-sight-query-{wrapper,feedback}.asm. Original Human0059FFD0 and movement005647C0
are in prior frame-human-actor-consumers/frame-item-actor-consumers exports.

Next: real Human B0+24/+28/+80 and inline98/DD0/230 callbacks together with
005647C0 movement, then containing actual full frame services/application pipeline.
New exports map player/AI/guard/script/area callbacks. Do not replace populated
jobs, action commands or formation/control with success because fixtures are cold.

## Feedback, poses and E0 frame writes

005839D0 base/player and005783D0 AI write BYTEAD/AC and preserve B0+50 except
bit08000000, refresh the actual hand entry, then clearAC when missing/not ready.
Only base/player has the nonoffline/nonplayer early return, AFTER those writes.
Action priority is1000→2000→4000→20000; a selected branch suppresses all lower
branches even with no item. Commands6/7 clear cachedA8 only AFTER actual item
execution and B0+08. Kind9 lookup/command4 retains the cache; command8 passes
the current item even NULL. Shared005A6730 already existed: it now lives in
human_holster_lookup.h and is reused by player mode and frame feedback, with no
duplicate ledger credit.

Actual0058B6C0 consumes raw B0 direction10 for nonplayers (not virtual AI aim),
actual mission-camera selector1 for player direction, and selector0 for position
unless mode2/selected valid kind9 uses actual FPV weapon48.00492790/00492880
decompiler stack arguments were wrong; static assembly establishes both selectors.
NULL camera output is unwritten in the original; source rejects consumption.
Non-FPV position uses borrowed itemA4/shdum or actual item model fallback; no item
uses Human head260.0058BA80 caches only a real shdum frame, repeats missing lookups,
and has no model fallback itself. A4 is now a typed borrowed Frame rather than a
DWORD. For player kind9, strict-.9<Y<.7 adds FLOAT.3, clamps .7, then normalizes
XYZ with stored FLOAT squared length and original X/Z/Y fallback; W is retained.
The remote nonoffline kind1 early return leaves BOTH vectors untouched. The real
item virtual24/28 is mandatory and its result/failure propagates. NULL fire is
rejected after pose writes, never reported as successful execution.

005785B0 is genuinely RET in offline7, AFTER the item fire call.00542920's
offline frame-emitted commands4/6/7 have no local effects;8 sets alignment16C=-10
through005A6250. Other player commands and nonoffline0x70 packet publication remain
required. The bounded offline bridge explicitly rejects unsupported nonoffline
execution rather than consuming the early-return's unwritten vectors.

0077F260 compares XYZ only: a W-only change retains the old complete vector.
0077F460 honors D0/D4/B0 locks and authority/player gates, normalizes the horizontal
direction, and updates only when BOTH physical240 and old E0+20 differ by>.01 in
XZ. It invalidates physical240 XYZ to-10000.099609375; native24C is unwritten stack,
so source retains W without credit.0077F690 invokes geometry007B8E80 BEFORE copying
actual DD0+940; it does NOT normalize its input. Locked setters publish deferredC8.
005AC080/A0/C0 bind these real E0 receivers.005AC0D0 returns OLD CC in EAX before
clearing withFFFD0FEF; the decompiler incorrectly described a void return.

Geometry007B8E80 acts only with6304bit1: bound player snaps all three directions
and clears62C0; other targets clamp dot, snap when1-abs(dot)<QWORD1e-8 (including
opposite vectors), otherwise seed acos/sin and trunc(FLOATangle*95.4929657) timers,
retaining current6360 until later interpolation. __ftol is QWORD FISTP; indefinite
has LOW DWORD0. Original62C4 and angle/sine remain untouched on snap.

## Equipment and containing frame order

00792CE0 now uses SAME230 state, real attachments/events/6950 weapon/light/lens.
Signed elapsed scales angle by FLOAT.0007 and translation by FLOAT.0012. D4 expiry
attaches hand3, emits event0 unless selected kind9/2, and may request actual weapon
blend300 when E0+5C==1. Death resets the timer. Translation and rotation preserve
clamps and restoration order. Assembly00792FD3 uses NEGATIVE TRANSLATION STEP for
the hide rotation, although BC advances by the angle step; restoration uses the
positive angle step. Do not replace this with a symmetric rotation policy.
Static frame flag80 suppresses position/dirty writes while230's scalar advances.
Actual light6C expiry resets BEFORE reread-count/index++ sector removal; lens70
expiry invokes007933D0(false), whose missing model/lens branch retains expired70.
007B8D00 uses actual retained weapon, flag62C8/6304 gates, signed duration and
FLOAT6318, then publishes6308/630C and40000001 flags. Geometry/frame engine calls
are delegated; there is no new engine-function credit.

005ABC20/70 preserve native player/mode ordering over SAME DD0/230. Actual
00787270 and00787D50 remain mandatory runtime receivers. They are NOT replaced by
animation-only updates. Presentation always invokes00787D50 and230 after its
conditional00787270; authority mode2-clear invokes control then230, while mode2
nonplayers invoke control only.0059FE30/0059FFD0 use these boundaries, fresh B0
after state transitions and real world services. Presentation timers17C/254 run
BEFORE registered14>=0;254 clears214 on signed<=0. Nonoffline death/respawn timers
precede that gate; request publication must succeed before writing204=-4000.
Authority perception precedes command ageing/state+28; feedback uses OLD CC;
alive motion/contact/stats/formation follow. Contact action flags come from
005A5C80 returning B0+5C, NOT+50. Remaining collision/actor005FEFB0/formation and
derived state leaves are required. No containing frame has executed in a mission.

005A0480 uses actual154 and signed158 thresholds>1500 moving/>2500 stationary,
two ordered ENGINE RandFloat draws and required blood0044F4F0. Mode2 preserves154;
otherwise it decrements. No second154 gate is added after decrement, so an expired
timer can still emit that call. Native XYZ-only stack W lanes remain uncredited.
Positive particle allocation/reuse0044F520 remains required, with its separate
CRT rand()%3 texture choice; do not merge the RNG streams.

005A88F0 uses activity B0+50&F8 and action flags+5C. Endurance10000/activity8
suppresses stealth even before its signed E8 timer exceeds25000; otherwise
action1000 advances signed E4, strict>20000. Exactly one subtraction/award per
call.005A93A0 adds FLOAT.1 to HumanC8, clamps upper5 only;005A94F0 adds FLOAT.05
to D4, clamps upper2 only. Mutators gate offline7 independently of timer updates;
negative/NaN values retain original comparison behavior. Debug-format-only calls
ending in genuine0069BFE0 RET are omitted, without extra credit.

## Checkpoint verification and next work

Eight focused temporary native ASan/UBSan/leak fixtures PASS: prior samples and
perception plus feedback, item pose/FPV queries, E0 look, equipment, containing
scene ordering and periodic stats. Remaining frame/fire/geometry leaves are
explicit doubles. Strict actual-host i386 instantiates the SAME World services,
real state factory/current B0, E0/DD0/230/items/catalogue/camera, feedback/fire
bridge and both containing scene paths. Unrecovered B0/DD0/combat/formation/fire
leaves are declarations in that compile fixture. App link and tracker verification
are recorded in the receipt. No Windows fixture/original EXE/DLL/Proton execution.
No full function or desktop promotions. Temporary fixtures are fingerprinted and
removed at this larger checkpoint; logs and original evidence remain in WORK.

Continue SAME full P2.2 checkbox: actual B0+24/+28 families and positive
DD0 control00787270/presentation00787D50/geometry007A7470, movement005647C0,
remaining particle/FPV/HUD/text/volumetric consumers, meaningful guard observations,
actual application loading and user first-frame/failure/repeat acceptance.
Passive AI005A4330/4340 are confirmed RET4 (absent frozen); common AI0056F710
returns immediately in offline7. Positive jobs/formation must retain real receivers.
New durable original exports: frame-human-{feedback-dependencies,feedback-items,
item-pose-dependencies,inline-updates,control-updates,geometry-target,
drip-and-geometry,actor-tail,periodic-stats,ai-presentation}; associated assembly
and constants are fingerprinted in the receipt. No diagnostics marker added.

## B0 AI timers, steering and task selection

00588010/00571680 now have base/AI timer bodies. Typed B0+68 handover preserves
the actual action-target identity;74/78 are typed frame-input borrowers, separate
from network7C. Invalid target sends virtual0C(false) before decrementing64;
timeout stores64 then sends true. Actual process membership0089B4E0/target94/life
and derived completion remain mandatory. Base bind clears74/78 borrowers as native.
The actual existing stance gates now inspect those pointers rather than cold DWORD
guard words. Native Base::bind handover plus timer sanitizer fixture and real-host
i386 pass.

00547E70 Guard and00559C60 Area prefixes use SAME Startup008AAF48/008AAE48 scratch.
Guard validates19C/clears scratch BEFORE common timers; Area runs common timers,
validates15C/ages action BEFORE clearing scratch. Watched pointers are typed and
resolve through SAME Scene2 identity00475DD0. Guard invalidation clears actual
AI128 target/accumulators. Area clears its watched target alone. Pooled action
ageing updates 0C/10 for the typed kind 1, 2 and 3 leases.

Guard00547DA0 uses real reverse closest-enemy distance005733E0, then when needed
reverse ENEMY B0+4C aim-dot00584890 toward the Guard; this is not Sight visibility.
Strict dot>FLOAT.9848099947, extended normalized X/Y, stored Z and distinct sum
orders are preserved. Distance starts65535, uses extended differences/stored FLOAT
square and signed bit-pattern epsilon test. Independent200/400 mode bits draw from
SAME CRT and may overwrite the first timer.005AE940 is constant2, so5000 cadence
adds2-hostile_count to actual Human stat3 through005A64F0, upper base clamp BEFORE
lower0. Other timers distinguish nonzero from positive starts and preserve wrapping.
00575850 reconciles query and direction flags. 00575940 steers from accumulated
direction or a typed borrowed aim frame and publishes both direction vectors to
B0. 00570A10/00570980 emit the original inline look command before updating
the B0 look flag. 0055D2B0 fires on signed overshoot, with one interval addition.

Startup owns the separate kind 2 and 3 process pools alongside kind 1. Growth
0058A660/0058A790 uses 0x50/0x40 byte records in batches of 16.
00582890 resets kind 2 age and leaves lifetime intact. 00582990 preserves both
age and lifetime. 00582C50 returns each lease to its own pool.

Guard 00549360 scores the actual processed event queue by signed lifetime,
distance and priority (005AE310), resolves actor IDs through Scene2 (005ACA50),
and follows the last 0x10000016 link in reverse (005AE270) when called for by the
selected event. Its pooled event clone lives in native 148. Guard reset 00548040
returns that clone before returning the action, leaving the source event in the
processed deque. The no-target branch creates kind 2 mode 2 with lifetime 5000.
The fallback vectors have three known lanes and leave W unspecified.

Area 00559EB0 selects kind 3 mode 3 via 0055A130 on the initial decision. Later
decisions consume the actual first queue through 00559F60, merge selected events
into the second queue, publish the target's actual head frame and extend the
reaction using the shared CRT stream. The low-skill 0055B6A0 branch is a genuine
RET. The 0055A0E0 reset checks flags 4 and 8 before returning the action.

Three native sanitizer fixtures, strict i386 instantiation over the actual
Guard/Area host types, and the launcher link passed. See the
[checkpoint receipt](checkpoints/p2.2-human-ai-task-selection-20261110.json) for
artifacts and coverage. Guard authority decision/worker bodies, Area kind 3
workers, the navigation job and common B0 output receivers remain required,
along with player/script/active frames and DD0 integration.

## Guard and Area waits and navigation

Guard 00549AE0 kind 2 mode 2 issues look 3 before the lifetime test. On expiry
it clears readiness through E0 0077F3D0 and the current player's FPV receiver,
requests posture, then publishes the 00560EA0 destination and enters mode 0D.
Completion enters mode 0C with one shared CRT draw for the next wait. Area
0055AD50/0055A6E0 use 0055B860 over the actual hostile list and processed event
queue. Their waits take four separate CRT draws, each shifted right by two
before the sum. The positive action workers and common B0 frame dispatch
remain open.

The Human allocation at +2C4 feeds 0057F610 and 0057EF30. Its cold storage
and reset remain with the same Human. The separate Scene2 process 005647C0
still needs its actual movement producer. 0057A750 now advances request timers,
handles direct destinations and restricted points, and binds path callbacks
to the loaded table, actual Human model and Startup job pool.

00521B60 expands links in reverse order, filters with the requested flags and
inserts before equal unsigned scores. It stops on first discovery of the goal.
00521480 uses the same open-list order with a distance penalty against the
actual hostile model positions and rejects points inside the exclusion range.
Both searches read the same loaded navigation records. The aligned 0x30-byte job
records at 008AB048+30 grow in batches of 16. The ordinary point, coordinate and
escape builders (0057B2E0, 0057AAF0 and 0057B740) populate both job position
lanes, native flags, kind and radius. The source retains the prior W lane when
00404260 writes only XYZ. 0057A8E0, 0057BDC0, 0057C2B0 and 0057C760 retain
separate tail projection and floor-height rules.

0057F670 now resolves the current point and drives the ordinary 0057DBD0
worker. 0057D5A0 handles the close radius and no-progress failure. On a reached
job the old current lease returns before moving the queue head into that slot.
Finish returns queued jobs in reverse and the current job last. 0057E030 writes
the same B0 input buffers and preserves the first facing gate. The reached
Door worker 0057CF80 uses real Door status, side and direction locks. It still
requires the Human kind 6 interaction receiver when the direct Door command
branch does not apply. The climb branch supplies the actual offline E0
animation6A request. The 0057CC50 interaction worker and populated floor
actor-list branch remain required.

The focused native fixture exercises waits, steering, query results, search
ordering and masks, cost relaxation, escape, floor smoothing, job reuse,
ordinary route progress and stall, Door approach and the climb request.
Strict i386 instantiates the real World service, Guard/Area, Inline, Door and
Startup owner types. The application link and tracker checks are recorded in
the [receipt](checkpoints/p2.2-human-ai-navigation-20261111.json).
The containing authority dispatch, specialized interactions and first live
mission load remain open.
## Guard and Area frame dispatch

Guard 00548170 and Area 00559DE0 now dispatch through the actual Human B0
owner. They share Startup's two frame input buffers and retain their distinct
direction and aim order. Input7C uses actual DD0+1AC/+9C0. The early 4000800
gate still reaches the deferred queue. Its populated serializer and transport
remain required. Guard's +24 and Area's +24 presentation calls follow the
original offline return from 0056F710. The owner retires a Guard or Area state
after its authority method returns, then rebinds the actual Script context
and calls 0041DA80 before the new look request (00590700).

Guard 00548610 and 00548520 reach actual inventory preparation, event
dispatch, wait and idle transitions. The pooled kind 5 action uses the
same Startup 008AB048+C0 list and retains age and lifetime on reacquire
(0058A9F0, 00582A40). The event path dispatches the current action before
merging retained events and applying stat and speech effects (005482A0,
005482F0). Live attachment, route refresh and other action modes still
require their original receivers.

005710B0 runs the ordinary detached hostile chooser. One candidate uses the
actual eligibility and health gates without scoring. Multiple candidates use
reverse traversal, live world matrices, the actual Sight relation mask
(0046F2A0 to 0046DC10), B0 direction and typed D0/D4 actor borrowers. Equal
scores keep the first candidate encountered. Guard 00548B00 and 0054AC40
use this chooser on actual Human perception lists. The kind 1 route includes
the CRT draw before the 0054E600 Door-list gate, missing-cover transition,
timed attack and lost-target branch. 00558AE0 reaches readiness and the
selected-target publication. Populated cover, Door, grenade and alternate
attack branches remain required. 00576350 and 00575D50 publish supported
no-item and weapon-deadline statuses and preserve actual stop timers
(00572C20). Aligned armed fire, line-of-fire and derived stop follow-ups
remain required.

00583C30 applies the actual shared movement input to B0 velocity, E0
movement animation, stance, posture, jump endurance and stop. Mode 2/3
command 6A maps to CE under the same native climb gate as mode 4.
0058BAF0 advances B0+B0 before checking the input buffers, suppresses the
first scratch trigger where required, and dispatches actual kind 9 actions
6/7 before current-player FPV requests 2/4. Attachments, the deferred climb
command, reload, positive fire and kind 9 release still require their actual
consumers. The shared 005647C0 Scene2 solver is a separate producer for
Human+2C4 and remains open.

The [source checkpoint receipt](checkpoints/p2.2-human-ai-frame-dispatch-20261111.json)
records native ASan/UBSan/LSan, strict actual-host i386 and launcher link
checks. The native fixtures used doubles for the game frame clock, input and
unimplemented runtime leaves. The application still declines mission loading.
