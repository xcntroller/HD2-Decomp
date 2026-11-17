# H8 occupied task, entry and follow source

This checkpoint connects the remaining ordinary00528950 occupied prefix and
entry3/seat-approach8..13,00536120 occupied follow, and Human occupied Sight on
the SAME World. It also makes the full module frame graph instantiable without
claiming live Dog implementation. H8 and the P22-HUM matrix remain open.

## Actual original behavior

Reuse WORK=`analysis/work/mission-world-20261103`: original
`human-h8-active-frame-workers/{00528950,00536120}.c`,
`human-h8-command-producers/00534510.c`, matching
`human-h8-follow-worker.asm`, `human-h8-entry-*.asm/.json`, and
`frame-sight-state-metrics/{00570280,0056fc80}.c`. The research index was
consulted before reading new leaves; no Ghidra project/type changes or original
execution were needed for this checkpoint.

00528950: controlled Car passengers wait for attack25/26/22. Other passengers
request common action5 before writing205; StaticWeapon passengers similarly
wait for attacks and request exit for other tasks. Driver0 sets205, binds the
actual0055E0E0 path, requests0055E160 position/speed1 and0055E130 movement flags
1000/2000 from immediate188. Driver arrival waits for the actual vehicle path
producer, without a premature on-foot model/distance query. Non-driver paths
continue the existing on-foot body. The initial normalized direction sums
(Z²+Y²)+X² as native. The reached pickup6 arrival also requests posture4 for a
Human target below squared distance16, even when the smaller arrival radius
has not been reached; equality does not trigger posture (raw528D81..DBF).

Native virtual **6C is claim,78 is occupant,88 is count**. Car00810778 maps
these to004D93C0/004D99D0/004D0E30. StaticWeapon00813130 maps them to
00604A90/00604590/00602F70. The previous Active command-selection adapter
incorrectly used the claim predicate for78; it now reads the actual occupant.
Entry3 uses6C: preferred210 first, then reverse free claim search. All-claimed
failure clears203 and speaks before the204/group gate. This is distinct from
00534510's occupied78 admission. StaticWeapon count now reads its actual live
seat vector, with no guessed fixed single-seat value.

Car entry queries004DA420 for requested210 first, then ascending alternatives
excluding that index. The selected unclaimed seat is separate from the pose
query index. Actual Human model position is read after the paired pose query.
The native FLOAT square/length thresholds are **4.5 and20**, not1.5. Within4.5,
005374A0 receives selected seat+8 before common action3 receives that seat.
Within20,214=1000 precedes00534510 re-approach; its task initializer may reset
that timer, matching the original. Failed/distant entry appends a hold for the
first pending order before trimming to two, then cancels, enables follow,
speaks and sets first mode18. The real queue/order owners retain those effects.

00534510 uses the supplied position, actual cached Human direction, task3,
203=true, radius2.5 and210=command-8. Car admission calls004DDD10, then actual
78 occupant. StaticWeapon also checks occupant78. Focus exit follows task
publication, then214=1000 and empty-order notification.00458E50 supplies target
cached Actor30, not a fresh model matrix. Offline004DDD10 checks actual254A,
004D5430(2) over the same initialized physics and reverse occupied seats: any
live Human outside the real Players roster rejects. Non7 local-player/team
eligibility remains a precise required boundary. Existing support-check math
is reused and earns no additional callee credit.

00598460 now reads actual formation slot80/84 (seat/actor identity). Missing
leader/slots/member returns0 and leaves the output unchanged. Occupied follow
uses these assignments, Scene2 identity and actual88/78. Different attachment
exit preserves the PRE-callback attachment borrower for the native early return.
Aircraft occupants retain their original exit exception. Car focus exit precedes
arrival/entry; nearby or stopped/arrived Car follow enters the assigned seat,
then005374A0(seat+8). When the leader is on foot, an occupied follower requests
exit and continues the common navigation tail. Missing Aircraft/type34 actual
owners retain their existing requirements; no synthetic seats are supplied.

00570280/0056FC80 attachment direction now queries actual98 before actualA4;
false98 falls back to the existing head direction. Car/StaticWeapon reuse their
same live controls, with no second aim state.00535E70 Car arrival now performs
the native fresh case-sensitive child-find `body`/FFFF (0083918C).

## Containing graph and required failures

HumanFrameRuntime previously lacked Dog/Sight C++ methods, preventing all-family
module instantiation even for scenes without Dogs. Those call sites now have
named **failing** Runtime boundaries for005D5620/005D5810, actual26C collision
exclusions,544 marker and005D64C0/controller534. This is no Dog implementation,
activation success, empty registry or score/origin fallback. Existing Dog
activation still rejects before publishing a derived prefix. No function credit
is claimed for these guards, and selected Dog support remains open. Removed the
stale occupied Active guard text; Active must use the actual World/factory body.

## Verification and remaining work

Temporary native ASan/UBSan/leak audit passed **1,487 checks** on actual path and
formation scalar owners with explicit World/engine/control endpoints. Coverage:
driver/passenger/attachment modes, claim versus occupant, all-claimed rejection,
preferred/reverse seats, commands8..13/focus/admission,4.5/20 boundaries, pose
failures, hold-before-trim lifecycle, occupied follow/focus/borrowed exit,
formation80/84 cold/failure/output behavior, pickup16 boundary and driver arrival,
and repeated exit-callback failure before205 publication. Test endpoint failures
and a corrected focus-path fixture expectation were resolved before the pass.

The actual full GameMissionModuleFrames authority/presentation i386 O0 link now
passes, replacing the earlier **missing-member** compile failure. It does not
execute those methods or close the explicit Dog requirements. Launcher/probe,
exact function tracker and27 tracker tests pass. Successful temporary native and
module fixture source/binary pairs are fingerprinted then removed; original/log
evidence remains. No Windows artifact was executed. Three new frozen partial
entries004DDD10/00534510/00598460; absent00602F70/004D99D0 and delegated calls
receive no new credit. No whole-function complete promotion or desktop claim.

Continue selected specialized Active callbacks (including22/16/retreat40), live
Dog/Animation dependencies when reached, delayed Players/current-command owner,
and application/FRAME/OWN/render. Source presence and full module linking are
not mission activation or actual desktop acceptance.
