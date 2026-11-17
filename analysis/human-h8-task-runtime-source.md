# H8 placed-item task16 and retreat continuation source

Task16 now uses the actual runtime placed-item35 motion/navigation owner.
Task40's reached pending continuation and00525260 spatial cover choice now use
the existing navigation, cover, engine rotation and Players roster owners.
This is source progress on H8.2/H8.4, not containing/application completion.
The delayed Player dispatcher and its move/retreat command producers remain the
next integration dependency; 00537C60 remains research for that producer, with no implementation credit.

## Original evidence

Research-index lookups preceded new read-only/noanalysis exports. WORK is
`analysis/work/mission-world-20261103`; original client SHA256 remains
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Existing `human-h8-command-producers/00534c40`, task-effect005387C0 and
`human-h8-active-frame-workers/00528950` were reused. New exports are
`human-h8-retreat-position/00525260`, `human-h8-retreat-navigation/0051f830`,
`human-h8-retreat-spatial/0051dbd0` and `human-h8-retreat-edges/0051d460`.
The last directory also retains00535A60/00537A40/00537C60/00537D20 research for
the following actual Player dispatcher; exporting them alone earns no credit.
Raw placed-motion-test, placed-task-producer, retreat-position, retreat-spatial
and retreat-continuation assembly resolves comparisons, arguments, spills and
signedness. All new export logs passed. No original or Windows execution.

## Placed35 task

005E24C0 reads actual A0/A4/A8 motion, not arbitrary actor words. It computes
extended (X²+Y²)+Z²; strictly below FLOAT0080F948=1.0000000168623835e−16 returns0,
equality/above/unordered returns1. The existing motion producer005E2510 writes
these fields only after Physics/contact/hide/navigation/packet/Script succeed.
No new physics owner, successful missing target or replacement point cache exists.
World queries require the SAME live runtime registry and its actual ItemActor.

00534C40 rejects already-started motion before task publication. Otherwise
00534620 creates task16/radius.75 with actual model position, Human cached
direction and empty-order notification. If A4 and BOTH placed94/96 indices
exist, it runs two actual first-table routes with mask0, clearing scratch between
them; each length spills FLOAT. First is selected only when strictly shorter
AND nonnegative. Otherwise native C0 comparisons reject unordered, first<second
or second<0; an equal valid length selects second. In particular a failed route
can cause rejection even when the other route is available. Rejection retains
the already-published task/notification and skips the focus-exit tail. Selected
XYZ overwrites1E0; native temporary W is unwritten and the typed lane is retained
without a value-equivalence claim. Focus exit and214=1000 follow only on the
original continuing branch.005387C0 uses !motion_started() for task16 readiness;
actual00528950 now admits16 to the existing turn/action/effect pipeline.

## Spatial cover and retreat40

0051F830 appends ascending point indices to the caller's existing vector. Empty
first table returnsfalse unchanged.0051DBD0 builds a separate local mark buffer:
zero radius marks every point; otherwise FLOAT(radius²+0080F514=.0010000000474974513)
is the strict distance limit. Search visits current ten-unit X band, then left
before right. Each band uses actual0051CBA0 pivot, forward then reverse visits;
strict Z² cutoff and (Y²+X²)+Z² test preserve native order. Only actual nonzero
cover bytes17/18/19 survive00524B10(7). No volume radius or graph-route metric is
substituted. Native uninitialized marks for invalid height/nonfinite traversal
are explicit required failures. Source vector RAII owns temporary marks instead
of reproducing the native allocation leak; no allocator/ABI equivalence claimed.

00525260 normalizes the supplied direction using native (Y²+Z²)+X² and the
existing spill/tiny-axis policy. Spatial candidates retain ascending order and
00524A30/0045ACF0 filter for posture mask3. Empty candidates return supplied
proposed position. Otherwise UNSIGNED actor-id modulo selects a candidate, while
SIGNED actor id converted to FLOAT is the actual LS3DF yaw angle. Existing
Frames.human_y_rotation and collision_sweep_rotate supply the real engine calls.
Normalize again, scale/spill XYZ by point byte13/16, then add actual point centre.
Native output W is unwritten and remains a typed unclaimed lane.

00528950 admits40/41/42 and executes task40 continuation AFTER ordinary206/202
publication. Nonzero190 attempts query actual Human model, form direction150*24*
FLOAT(1/3) proposed position with native X/Z spills, then run00525260 and actual
nearest-volume10/−1. Clamp outside the selected point's radius horizontally;
Y is initially0, the distance uses negative pre-overwrite Y, THEN destination Y
becomes point Y. Extended (Z²+Y²)+X² admits >=4 and unordered. Failed/too-close
selection clears190 and continues existing completion.

A continuing retry initializes task40 with saved1F0 direction and current21C,
which resets206 before actual roster live-count lookup. Raw529DE4 is SIGNED
IDIV: count1 explicitly selects remainder0; otherwise signed actor id modulo
(count−1). Delay is wrapping remainder*800, plus(count−2)*500 after the first
attempt. Publish214,204 and decremented190 before callbacks. Nonzero delay
rereads150 for target/current direction, extends signed19C only if smaller,
then actual stance and item-ready receivers. The containing caller rereads206
AFTER callbacks, retaining native completion if callbacks changed it. Native
INT_MIN/−1 division overflow is an honest explicit failure. No private roster or
unsigned replacement of IDIV is used.

## Verification and limits

[Receipt](checkpoints/p2.2-human-h8-task-runtime-20261117.json) pins source,
original contracts and temporary audit fingerprints. Strict native ASan/UBSan/
leak checks PASS3481 on actual PlacedItemActor motion and MissionNavigation
route selection; PASS1280 on actual spatial queries/retreat code, including
independent finite-grid spatial expectations, ordered append, empty/invalid
queries, candidate modulo, signed ids/count0..6/attempt1..3 and failure prefixes.
World/AI/engine endpoints are explicit doubles; engine rotations are not executed
by the native audit. Actual World/module authority and presentation instantiate
and link strictly for i386; launcher/probe rebuild is sequential. Tracker and27
tracker tests are recorded in the receipt. Successful temporary pairs are
fingerprinted and removed. No application/desktop acceptance or mission absence
claim is made. Six new frozen partials, no complete promotions or engine credit.
H8.2/H8.4/H8.5/H8.6 and the HUM matrix stay open.
