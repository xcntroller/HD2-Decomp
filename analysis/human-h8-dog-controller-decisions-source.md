# H8.4 Dog follow, target and action decisions — 2026-11-17

Actual kind4/5 controller now binds complete005CD890 follow,005CC5F0 direction,
005CC8D0/005CC7A0 target scan,005CD120 target clearing,005CDFF0 heard retirement,
005CEC60 action publication and005C1E60/005C6C70 Body direction requests. Actual
Body005C5A90 bite cancellation uses the existing SAME order owner. These are
components of the remaining containing005CE190/005D5620 pipeline; H8.4 stays OPEN.
Target pursuit005CD1F0, route005CBF00/005CA5A0 and selected controller5 frame
remain required; no idle successful containing frame has been introduced.

## Evidence and source

Research index consulted the containing pipeline, reached dependencies and new
exports. Reused C in WORK/h8-4-dog-live-originals and
WORK/human-h8-dog-controller-body-dependencies. Three previously absent exports
005CC7A0/005CCCC0/005CCEC0 were recovered from the matching read-only Ghidra
project under a rollback transaction (no original definitions persisted), in
WORK/human-h8-dog-controller-targets. The latter two are research dependencies,
NOT implementation/credit. Raw/constants and export receipt are under
WORK/human-h8-dog-controller-decisions. Original client SHA256
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.

005CC5F0 writes direction40 planar XYZ before its exact FLOAT square, unit/
tiny/root normalization, including native zero-vector Y=+1 fallback. W is native
unproduced scratch and remains optional/unclaimed.005CD890 starts with20000/
null-follow gates; no flag10 only aims at005CD690's actual follow point. Positive
following uses the original point and separate X/Z deltas (the decompiler
aliases them). Raw FST leaves extended Z before multiplying its FLOAT copy;
distance then spillsFLOAT and reciprocal stays extended. It clears6000 before
Actor direction/dot reads, uses FLOAT dot to select spacing1.2/.2 or double
(1-dot*1.17)+.20000000298023224, gait4000/2000 and speed1/1.1/1.2/1.4. Unordered
distance takes the native far3.5 branch. Near one-point routes clear through
actual route pool/Script owner, then current flags/follow are re-read for
stop/forward-facing/aim decisions. Original nonzero origin does not turn the
follow point into a delta. Actual World speed updates SAME Body actions248.

005CC7A0 excludes nonpositive Human health/attachments, takes physical Actor30
positions and FLOAT (Z²+Y²)+X² with native tiny/unit/extended-root selection,
clamps root to FLOAT1000000 and computes (1000000-distance)+distance. Keep that
original equal-score/first-candidate behavior; do not invent nearest targeting.
005CC8D0 damage_source8C overrides and returns before force/list/event work.
Force resets an existing88/90; otherwise existing target's different XYZ square
order and FLOAT distance spill establish its score. It scans SAME live574 then
564 records with strength>19, retaining signed conditions and source provenance.

Raw vtable34/005CB2C0 is the current follow18 GETTER, NOT a null/default callback.
After selection, positive Human follow receives an actual process AI event:
10000017,10000,0,Actor10 identity,target88 identity, captured physical target
position and word12=0; unwritten words remain retained pool payload. The second
getter is fresh after production. Actual Human005A8B30's B4 command-null return/
first-event queue admission uses SAME command and process pool. Typed leases
supply source exception cleanup; original leaks/allocator ABI are unclaimed.
NonHuman follow B4 remains a required selected-family receiver. No extra credit
for005CB2C0 or005A8B30 is added at this checkpoint.

005CD120 resets target90/followA0 XYZ, clears30000, clears88/8C, then optionally
completes actual kind5 Script100000+40000 request and clears100000 AFTER callback.
005CDFF0 preserves unsigned(now+2500000)<record.clock18, each actual performance
clock query, pool return before swap/pop and unconditional index increment that
skips the replaced record. No clock-unit/expiry correction is authorized.

005CEC60 zeroes movement30 XYZ (W unclaimed), requests action3 BEFORE checking
buffers, then reads current first/second slots in original order. It binds
actual Body priorities/requests for1/0/2, posture,5/4, LAST stance toggle and
conditional turn6C70; buffer mutation/failure preserves each completed prefix.
005C1E60 gates on XYZ change then LAST stance!=1; source copies produced W only
and retains typed padding for native unproduced W, with no numerical claim.
005C6C70 borrows SAME process008AC4A8 counter. Its initial0 is proven by the
PE zero-filled .data tail (virtual extent exceeds raw bytes), not an arbitrary
read beyond raw section bounds. Nonzero decrements low32; zero rotates actual
Actor40 by2.799999952316284 about Y through existing engine math, requests Body
direction and republishes0. Counter persists across World/animation table resets.

005C5A90 sets36C for active5/6 before queue access, takes a first queued record
only if current exhausted, consumes at most ONE5/6, and optionally copies the
next record. It does not filter or rewrite the whole actual order queue.

## Verification and limits

Native91 strict ASan/UBSan/leak assertions execute new follow/target/action/
retirement policies, SAME BodyActions/Orders/State, actual process pools and
actual AI event producer. Actor/frame/query/Script/registration endpoints are
explicit doubles. Checks include zero/tiny/NaN direction, nonzero origin,
spacing/speed/far/route/stop cases, Script mutation/failure prefixes, first valid
candidate and forced/damage override, heard threshold/live scans, real event
fields/retained words/pool cleanup, target clear callback order, unusual heard
clock wrap/expiry/skip, all buffer requests/NaN/priority/partial failure, counter
wrap/rotation/stance/W provenance and one-command bite cancellation. Fixture
states are explicitly initialized, not proof of live game activation.
Strict actual full World i386 fixture instantiates selected controller methods,
actual pool/AI event/Body/World bridges and cancel path; compile/link ONLY.
Forced strict O2 launcher/probe and tracker/27 tests are in the
[receipt](checkpoints/p22-h8-dog-controller-decisions-20261117.json).
All temporary audits are fingerprinted before removal; no permanent tests added.

Ten frozen client partials only:005CD890/005CC5F0/005CC7A0/005CC8D0/005CD120,
005CDFF0/005CEC60/005C1E60/005C6C70/005C5A90. No complete/engine/absent/new-export
research-only credit, native object/vtable/allocator/SEH ABI, scratch W values,
full005CE190/005D5620, application entry or desktop acceptance. Continue the
containing pursuit/route/controller decisions on these SAME owners.
