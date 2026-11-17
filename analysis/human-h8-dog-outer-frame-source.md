# Dog outer Body frame and World damage — 2026-11-17

Source checkpoint after4d1aa41. H8/HUM remain open; no application/desktop claim.

`advance_dog_body_frame` now supplies the whole containing005C8B30 sequence on
actual Dog/World/Frames owners: snapshot180 →005C89C0 turn→same clip005C71E0→
005C3420 motion→clear268 BEFORE optional005C49A0 bite→signed-positive wrapping
370 decrement→005C0D10 position request→mode-bit1 material hazards→temporary
model/sector placement→same collision owner update→model/sector restoration.
Material46/47/5A/48 use unsigned elapsed times FLOAT.01/.03/.02/.5;5D uses
kind9/amount−1. Other material and mode gates genuinely skip D4. Actual World
`advance_dog_body` instantiates this entire sequence; actor activation/frame
entry remains guarded until its remaining owners are ready.

Raw005C8D34..9045 is authoritative over aliased C locals. Cached world model
XYZ is compared to fresh Actor position with extended `(dz²+dy²)+dx²` and
strict FLOAT.0001 threshold (unordered rejects). Positive temporary placement
calls actual frame08; fresh Actor position+FLOAT.5 feeds actual SceneE0. Parent
is captured BEFORE E0; native moved flag publishes AFTER E0 returns. Only a
nonNULL kind5 parent different from queried sector links through38, whose
status the original ignores. Frame24 precedes descriptor refresh. Descriptor
owner updates its anchors ONCE, alive first two and unconditional last three.
Restoration uses saved model position+FLOAT.5, repeats parent/E0/conditional38/24,
and does not move descriptors a second time. Source exceptions additionally
restore a placed borrowed model, recording cleanup failure while retaining the
primary exception; this is source safety, not native engine exception equivalence.

005C49A0 requires first actual control24 target, current5/6, ordered distance28
<=FLOAT2.1 and support24C==0. Modebit1 then compares actual integrity stage5
(ushort008CAB3A/008CB23A), snapshots actual Actor40 direction, calls control24
AGAIN, and reads fresh Actor4A4. Mismatch multiplies LOW32 by10 before signed
FILD/FLOAT amount. Actual target D4 receives kind8, zero point, saved direction,
NULL source/extra0. Only after successful D4, actual Players00461410 control
receives SAME Dog0045D4A0.0069BFE0 is confirmed retail RET; no invented logger.

World shot/common blast and actual item melee now dispatch Dog D4 into existing
005D7180 policy, borrowing SAME Body skeletal slots, health/current actions,
controller534, shared startup integrity, actual Sight observers/targets and
startup SOUND listener owner. Melee00451230 raw D4 kind is3, distinct from bite8.
Body death uses SAME actual ground404/damage-bone d, sweep, forced motion and
five descriptors. Listener removal returns before each−1 publication. Scalar
initial health4B0 and live indices53C/540/548 remain unproduced until real
activation. Optional provenance is checked at native READ, never earlier:
initial health only AFTER survivor Body/controller effects; source/target/sound
slots in death order AFTER controller-dead and Body request. Mode/dead-health
returns require none of these producers.

Actor534 is now a unique typed `DogControllerLifetime` owner, replacing numeric
presence. Its mandatory virtual queries/dead/damage have no idle implementation.
Cold initialize clears the owner as original. No controller factory/activation
success is fabricated. Actual005D5890 and selected controller frame remain next.
005D7180 is absent from frozen inventory: source integration earns NO new credit.
Existing005C8B30 partial expands; only frozen005C49A0 adds a partial.

Verification: native34 strict ASan/UBSan/leak cases exercise temporary descriptor
pose/restoration, every throwing engine stage, primary-vs-cleanup failure, ordered
bite/equality/NaN/mismatch/wrap/target reread/failure, and late damage optional
reads/partial handle retirement. Explicit frame/controller/damage endpoints;
previous actual solver/Body tests are retained and not repeated. Full actual
World/module i386 fixture compiles/links both passes, complete outer Body, actual
shot/melee/Dog D4 instantiations. Controller remains an abstract required boundary;
no live activation claim. Forced O2 launcher/probe compile/link PASS; Windows
fixtures and originals never run. Temporary audits fingerprinted then removed.

Evidence: WORK=`work/mission-world-20261103`, existing boundary005C8B30 and
body-dependencies005C49A0 C/JSON; aligned `human-h8-dog-body-authority-frame.asm`,
`human-h8-dog-bite.asm`; index records precede raw exports. Receipt:
[outer frame checkpoint](checkpoints/p2.2-human-h8-dog-outer-frame-20261117.json).
