# Sight and ShotMgr frame source — 2026-11-10

Continuation of the FULL first unchecked P2.2 checkbox. Source-only and
verified at the [source receipt](checkpoints/p2.2-sight-frame-20261110.json).
Application loading still declines; no actual engine frame or desktop acceptance.

## Sight scheduler and observer policies

Module5 authority0046F000 is bound through GameSightLoading to SAME source lists,
relation records and Startup-owned process FLOAT00839390. PE initial value is1.0;
0046EFD0 resets cursor2C/cursor30/delay34=4000, never that shared accumulator.
A tick crossing the delay consumes that tick without scheduling. Signed DWORD
time, FLOAT stores, signed wrapped pair product, signed64 truncation/lowDWORD
budget, fresh records, off-by-one budget test, target double-charge and ignored
reverse-pair marker predicate result follow the original. Target/observer vectors
and relationship storage remain those from actual registration, with no new map.

Human0046DCF0 and Dog0046DEA0 wrappers preserve mode2/flag24, previous counter,
marker bitmask and BYTE-only writes, Human change-only counter updates, callback
order and deferred feedback suppression.0046DCF0/0046D840/0046DC70 are absent
from frozen inventory; no new credit. Actual Human metrics and geometry now use SAME World owners; positive Dog leaves remain required.

Human0046DF80 increments/wraps one marker. Its deadline uses unsigned now<deadline;
future work returns the OLD aggregate and suppresses feedback. Zero score uses
elapsed; positive score uses now-last and publishes last. EDI is explicitly zero
before the comparisons, so negative/unordered Human score yields interval0.
Deadline publishes BEFORE head260/marker queries. NULL target marker returns false
without clearing cached score. Human targets exclude SAME inline98/DD0 collision
set, then restore after ray; matching attachment160 with flags40 is visible, as is
NO hit. Dog targets exclude actual actor+26C two-descriptor set (if first nonnull),
force layer2, then always clear shared exclusions. Layers2CC100/flags100000/no
normal. A blocked ray clears only selected marker score. On source query failure,
restore exclusions and preserve first failure. SAFE debug0069BFE0 is genuine RET.

Dog0046E3B0 uses signed(deadline-now)>0, returning false while deferred. Unlike
Human, a negative/unordered score leaves the native interval unwritten; source
rejects that state. Actual005D64C0 must update interval/score before deadline
publication. Score100 proceeds to head544, target marker and ray208000/100000.
Only HIT+flags40+SAME target actor is visible; a clear ray clears cached score and
returns false. No Human exclusion bracket. Assembly confirms NULL544 with a model
first reads model position, then STILL dereferences NULL544. Do not silently turn
that into a successful model-origin fallback; actual receiver must reject it.

005D64C0 is only a wrapper: requires actual Dog534, returns without outputs when
534+14==1, otherwise tailcalls534 virtual1C. That owner remains required.
Human005A59D0 delegates B0+38;005A59A0 player case writes interval500 ONLY, otherwise
B0+34. Base005830B0/C0 are genuine RET12 and leave outputs untouched. Source uses
an absent optional score and rejects its consumption by the observer; no score0
or100 default. Existing state owners now select their actual vtable family:
player+38=00542320, AI/Active/Guard/Script/Passive/Area+34=0056FC80,+38=00570280,
base/remote+34/+38 are RETs. PE table evidence covers all eight constructors.

## Actual Human metrics and query binding

Human005A59E0 computes TARGET range, not observer range. Absent B0 returns-1;
dead target returns actual DD0+DA4 field0 times50. Live targets use actual B0+50
flags/+B0 signed clock and SAME Players control roster or auxiliary roster under
native nonoffline/remote/authority gates. Strict7000/8000 clocks govern300/380/450
minimum ranges. Base ranges50/70/100, low-byte F8 gate and ordered1000/8000/2000/
4000 scales retain FLOAT stores and minimum selection. Kind2 comparison with30
is preserved, though the initial ranges already exceed30. Actual inline98/DD0
opacity field0 is used, distinct from target/smoothed opacity. NULL754 skips
surface adjustment; a populated754 still requires its actual material producer
and GEffects94. No pointer is invented. Optional material policy preserves the
flags2 half adjustment and final extended product.

00542320 player Human metric gates800 only;00570280 AI gates40000800. Both use
actual target/observer head260 matrices and actual target model origin. Player
range multiplies by FLOAT1.4; AI first stores target range, then multiplies1.3 for
kind3, else caps by signed observer110. Both cap at actual active scene camera128
through00465DE0 (controller58), rather than the controller's saved240 camera.
Player's symmetric triangle uses QWORD00810D30=1.0471975803375244 radians; its
FLOAT low word2 is NOT the angle. AI's asymmetric triangle uses actual head or
required attachment98/A4 direction versus B0+10,50/60 degree widths, native
positive-only difference wrap and +/-FLOAT1.4 limits. Inclusive triangle sides,
normalization fallbacks and extended sqrt/FLOAT stores are preserved.

0056FC80 Human-to-Dog metric preserves flags/state-kind3 and observer kind2/3
exclusions,100 range cap, real target544/head-model queries and symmetric triangle.
The original005A59A0 player wrapper leaves score unwritten, which is retained.
Actual positive Dog534/544/+26C producers remain required when reached. Baseline
Tutorial has no Dogs; live overrides remain authoritative, never blanket-skipped.

GameSightLoading now binds actual Human metric/state/feedback bodies and
HumanWorldLoadingServices head260/marker poses, SAME session28 clock, actual
inline98 exclusion/restore and CollisionRay over SAME loaded World tree/buckets/
resource geometry. Actor matches use actual descriptor+48 identity; normals are
not requested. Dog exclusion arrays, head and534 metric remain explicit required
frame leaves. Attachment and populated754 paths reject until their real positive
receivers exist. No application readiness or first frame is published here.

## ShotMgr cold frame path

Registered virtual0C00451EC0 and virtual1000451FB0 are absent frozen inventory.
Module14 now ticks SAME ShotManagerLoadingState. Authority requires empty transient
114. Presentation, unless SAME application mode bit1, adds elapsed to field140 and
uses SIGNED strict>100, subtracting100 once BEFORE authority0C(100), then transient
124. Always visits transient114/transient134 and render B0/A0/90 in native order;
render80 is not part of this tick. Positive opaque queues still reject; no shot,
decal, projectile or render acquisition has been fabricated, no new function credit.

## Focused verification and evidence

PASS native ASan/UBSan/leak actual scheduler/observer policies, both ray policies,
unsigned versus signed deadline wrap, strict boundaries, retained cached fields,
attachment hit/clear ray distinctions, null marker timing, shared exclusion cleanup
and first failure. Separate native sanitizer fixture covers actual Human range and
player/AI/Dog metric policies, roster thresholds, scales/material adjustment,
asymmetric angles, state gates, inclusive/exclusive edges and unwritten scores.
Explicit engine/owner doubles do NOT establish desktop visibility. Earlier cold
ShotMgr fixture passed native sanitizer checks.
PASS strict i386 actual World/module dispatch instantiation of BOTH18-module
routes; remaining unrelated frame/Dog leaves are declarations. Actual Human metric and
ray binding is instantiated. PASS application link. No original
binary, DLL, Proton or Windows fixture executed. Eight temporary fixture source/binary/object files were fingerprinted and
removed at this checkpoint; logs and original evidence remain. No permanent tests.

Durable evidence under analysis/work/mission-world-20261103:
frame-sight-scheduler exports/asm; frame-sight-positive-boundary exports;
frame-sight-state-metrics exports; frame-sight-human-observer.asm;
frame-sight-dog-marker-predicate.asm; frame-sight-{human,dog}-marker-ray.asm;
frame-sight-{base-metrics,dog-metric,debug-hook,range}.asm;
frame-shot-module-ticks.asm. Current focused logs: frame-sight-native-{build,run}.log,
frame-shot-native-{build,run}.log, frame-sight-host-build.log,
frame-sight-application-build.log. No new diagnostics marker or parser change.

Additional evidence: frame-sight-range-dependencies read-only exports;
frame-sight-metric-constants.json (typed FLOAT/QWORD and B0 vtable mapping);
frame-sight-player-triangle.asm, frame-sight-ai-{human,dog}-metric.asm,
frame-sight-ai-angles.asm. The metric application build log is
frame-sight-metrics-application-build.log. Twelve new partial entries; no complete
promotions.0046DCF0/D840/DC70,005830B0/C0,00465DE0 and both Shot ticks are absent
frozen;0069BFE0's debug RET and delegated LS3DF methods receive no new credit.

Continue FULL first P2.2 checkbox: remaining actual Human/movement/Dog actor ticks,
positive render/runtime/volumetric consumers, guard publication and actual app
loading/activation, followed by failure/repeat and user desktop acceptance.
