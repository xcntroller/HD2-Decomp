# Dog motion and Body bootstrap source — 2026-11-17

The actual Dog now owns the same clip displacement/forced motion/deceleration,
Body transform and bootstrap fields. This completes the source of005C71E0,
005C7600,005C75B0,005C0F50,005C0D10,005C0D60 and005C8370 and the remaining
005C7D30 initialization sequence for defined producer states. It does **not**
enable live activation or close H8.4. Controller/frame/collision-motion005C3420
and actual application delivery remain required. No desktop or native ABI claim.

## Motion

[Clip source](../recovered/game/dog_clip_mixer.h) implements005C71E0 from the
existing C and aligned raw instructions. Current stride is multiplied by the
current group0 rate, blended with old stride using005C7B00, then projected along
the supplied XYZ direction. Base seconds spills FLOAT. X products remain extended
through their final addition; Y/Z products spill at the native sites.

The SAME005C75C0 forced-motion owner is consumed. Its counter wraps and stores
before the strict duration<new-count expiry; expiry clamps only this call's
active portion, not the stored counter. Duration equality remains active.
005C7600 initializes the deceleration vector, its normalized per80ms reduction,
counters and duration. Deceleration expiry uses inclusive >=, clamps the portion
and stores the actual accumulated active time. Exactly80ms does not reduce
velocity; the next excess does. Neither zero-duration branch invents a guard:
native unordered results remain unordered. Getter005C75B0 returns actual XYZ.

## Body transform and bootstrap

[Transform](../recovered/game/dog_body_transform.h) implements005C0F50's ONE
vector-pointer request, not navigation or ground projection. It preserves the
FLOAT delta/square and DOUBLE distance spills, DOUBLE2 branch, native reference
selection, flags3E0 alias, zero-distance epsilon and signed interval166 clamp.
005C0D10 snapshots before clearing bit2.005C0D60 snapshots desired direction,
normalizes actual model forward with X retained extended, compares the fresh
input after the matrix callback, then calls model0C and rereads model20 for24.
Unproduced prior390/reference3A0 fail at their actual consumption, after prior
request/delta/previous writes. Large distance differences retain the original
zero-vector normalization behavior.

The bootstrap-specific005C7D30→005C0F50 composition has a proven postcondition:
flags=0, current=previous=cached finite model position, so first distance=0;
both reference branches choose that SAME model position. The old unwritten3A0
cannot affect defined outputs. The helper publishes only these proven outputs,
without assigning an invented prior reference.10000 arbitrary/NaN/Inf reference
cases compare full requests against this composition. Nonfinite actual model
positions remain an explicit unsupported state. No additional D11 default.

[Body state and bind](../recovered/game/dog_body_state.h) compose actual clip
publication/reset, five same-grid descriptors, anim2 lookup, queue/scalars,
model-position snapshot/publication, surface and timer fields, eight marker
lookups in native d,w,f,c,h,p,t,l order, voices, action/posture/stance requests,
then anim2 XYZ/dirty-bit stores. Model20 is reloaded at each callback. Seven
Actor478..490 damage slots are shared with the borrowing DogDamageGeometry
constructor, not looked up again by live damage. Undefined W lanes are unclaimed.
Original00811FBC is **-10000.099609375**, distinct from timer-10000; retained
static byte evidence prevents conflating them. An inserted collision prefix
survives a later initialization failure until actual owner cleanup.

Native Body278/27C/280 are still not written by this initialization. Their
existing provenance is preserved: a truly cold state fails at the first native
priority read AFTER bones/voices and BEFORE anim2 reset. This is not a successful
cold activation. An actual producer or separately authorized compatibility policy
is still needed if the selected mission reaches that native-invalid state.

## Verification

Strict native ASan/UBSan/leak audits pass319938 clip-motion checks,123365 transform
checks and9563 Body graph checks. These cover20000 forced families, an independent
integer80ms deceleration oracle, strict/inclusive expiry, native-invalid prefixes,
20000 independent-axis requests, the bootstrap proof,300 full Body/clip/collision
graphs, all21 endpoint failures and the cold priority failure. Frame/resource/grid
endpoints are explicit doubles. Earlier grid/clip callback audits are unchanged.

Strict actual i386 full module authority/presentation+same World Dog bootstrap,
actual process callbacks/grid/sweep/transform/borrowed damage-bone graph link PASS.
Forced launcher/startup-probe rebuild and tracker checks are recorded in the
[receipt](checkpoints/p2.2-human-h8-dog-motion-bootstrap-20261117.json). Windows
outputs are compile/link only; no original EXE/DLL/Proton or desktop execution.

Evidence in analysis/work/mission-world-20261103: dog-clip-motion and body-bind
index results; human-h8-dog-clip-motion.asm; dog-clip-deceleration.asm;
dog-body-transform-full.asm; dog-body-direction.asm; dog-body-bootstrap-full.asm;
dog-body-motion-constants.json; dog-bootstrap-constant.json. Existing originals:
h8-4-dog-required-originals/005c7d30.c, boundary-originals/005c8370.c,
body-dependencies/005c71e0.c, controller-body-dependencies/005c7600.c and
body-dependencies/005c0d10.c/005c0d60.c; raw is authoritative for ABI/x87 spills.
