# H8.4 Dog head presentation/resource source — 2026-11-17

005D7A10 now runs on SAME DogHeadLook4E8..52C, with actual World frame/math and
animation-resource receivers. This completes the head-look request/timer/
presentation source component. It does NOT enable live Dog activation, controller
or containing actor frames; full H8.4 remains OPEN and work continues.

## Original behavior

Research index consulted005D7A10/007885B0 and Body dependencies before bounded
raw reads. Original SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
C in WORK/h8-4-dog-boundary-originals; raw and FLOAT/DOUBLE constants retained in
WORK/human-h8-dog-head-presentation. Raw overrides C's lost arguments/temporaries.

Inactive4E9 returns before parent/target/resource work. Active path reads fresh
4E4's parent matrix, zeros ONLY scratch translation XYZ and inverts it, then
reads actual model direction, normalizing Y+Z+X. Request504 publishes4F0XYZ as
normalized model direction*10 plus fresh marker position. Fresh marker position
is read again to build the XZ target delta. First presentation reads/normalizes
actual marker direction, publishes yaw500=0, then computes signed007885B0 angle;
subsequent frames subtract actual yaw500 from model-to-target angle. Yaw500 is
optional source provenance until the genuine first-presentation producer.

Native mode50C0 uses FLOAT .0028;1 clamps magnitude .1..FLOAT pi;2 clamps only
minimum .1;both use log(magnitude)*DOUBLE .002+DOUBLE .006. Other modes use FLOAT
.002. Signed elapsed/speed produce a FLOAT step, clipped to actual extended angle.
The cumulative yaw cap is DOUBLE .9707963466644287. Completion uses DOUBLE
.003000000026077032 and original AND4100 unordered semantics. Completion with
504 clears4E9/4E8/504 BEFORE the engine first-animation08 reset, then returns;
other completion sets505; moving steps store500. Final direction is the original
normalized model direction, rotated by500 and transformed by the scratch parent
inverse, then published through animation1C;4E8 publishes only AFTER success.

The World adapter reads actual4E4/model20 at each original callback boundary.
Required unproduced4F0/500, NULL marker/parent and missing animation owner retain
source failures; no camera/direction/current-target successful defaults exist.
Scratch fourth lanes receive typed padding with no native provenance claim;
explicit prior head-look request words retain their own documented source.
The existing engine inverse wrapper rejects a singular matrix instead of
consuming failed inverse output; this safety guard is not original ABI equivalence.

## Actual animation-set lifetime

New DogHeadAnimation owns actual4E0 resource. Creation calls SAME driver78;
its f animation uses driver74("f"), collection04; modelB4 binds collection,
then actual model Find("f",FFFF) supplies4E4. Native model20 is re-read AFTER
Add and AFTER binding; callback replacements are not hidden by a cached model.
The complete owner is published before model/resource binding callbacks; failed
bind retains its acquired set prefix for cleanup. The original performs no
separate release of the newly-created entry after collection04.

Presentation reads the real set's FIRST entry from08 for reset08 and direction1C
(vector,0). Explicit close repeatedly removes current first entry through set08
while actual count10>0, then releases set0. Failed close retains its owner for
retry; final destructor cleanup records errors before guarded resource unwind.
This implements the resource portion of005D4DB0/005D5390, not either full body;
neither earns a new count. No private engine animation or second model is created.

## Verification

Native42 strict ASan/UBSan/leak assertions cover query order, all four rate
modes, independent logarithmic step oracle, first/subsequent yaw, cumulative cap,
minimum/maximum clamp distinctions, zero elapsed completion, failed direction
publication, completion flags before throwing reset, model replacement between
resource callbacks, and actual source resource-owner bind/close/failure lifetime.
Engine frame/math/resource endpoints are explicit doubles. No original engine
or loaded Dog is executed; native numerical policy and owning classes are real.

Strict full actual World i386 compile/link instantiates engine resource creation,
request/update/presentation and explicit close on SAME Dog/process/frame owners;
fixture explicitly calls initialization, not live005D4DB0. Final forced launcher/
probe and tracker results plus removed audit fingerprints are in the
[receipt](checkpoints/p22-h8-dog-head-presentation-20261117.json).
Only frozen005D7A10 adds one partial claim; no complete/engine/absent credit,
application/desktop/matrix or full H8.4 claim. Continue Body presentation, live
activation/retirement, controller decision and containing actor source.
