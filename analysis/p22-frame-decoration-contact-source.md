# Actual Player decoration contacts — 2026-11-17

Source-only checkpoint on feature/p22-frame, base e667395. User candidate
a27a24a5 passes activation loaded1/status5, Scene78 render, fixed-update, input
and full update; a later script command154 DOOR_Lock causes handled result1.
No access violation in that run, stable scene/control/presentation acceptance or
whole FRAME/HUM/OWN closure. Exact executable/logs and verification hashes:
[receipt](checkpoints/p22-frame-decoration-contact-20261117.json).

Original0047F4E0 captures actual Player position/velocity/posture. Posture2 starts
1000ms interpolation; repeated first-person queries and late publication retain
native ordering. Stationary/nonprone returns; nonzero vertical velocity still
reaches contact. Posture0 uses inner/outer .25/.4, posture2 .75/1, others .3/.5;
the circular grid scan radius3 is independent.0047FAE0 scans X then Z, validates
actual group/entry/model and writes first visual A8 before flattened flag10.
3D distance uses the strict outer boundary. Existing bounce requests are reused;
new typed requests publish to their cell before insertion. Actual world-forward
getters repeat;007885B0 computes wrapped second-minus-first atan2. Inner hits
fade base model EC opacity to .9 of base; annulus hits damped bend/bounce.

0047F200 steps angle by signed elapsed, speed, .001 and bounce .3/one-shot3;
old/next crossing tests clamp to target. Bounce target reverses at -.9 with .9
speed damping until double00810178 (.017453293005625408) threshold, then final
roll0. Model local rotation-axis getter precedes model1C direction/roll, model24
update and one-shot model50 alpha decrement .0003*float elapsed. Bounce opacity
fields stay absent/unwritten; one-shot alpha work keeps the request alive through
its final write.0047F150 updates reverse and destroys a retired request before
swap/pop. Grid refresh queries ground before retirement/pool return.0047EA60
retires requests before ascending Y/X model releases;0047E1E0 cell destructor is
a genuine RET. One-shot retirement retains flag10 until refresh; bounce clears it.

Host axis receiver mirrors native matrix10 lazy update: mark bit, normalize actual
quaternionC0, SetRot3 matrix80, return A0. Existing-bit path normalizes actual A0.
Opacity requests use model50 directly, independent of the grid mode gate. Cells
borrow requests and requests borrow actual models; cleanup removes borrowers
before model deletion. No extra successful receiver, fake resource, changed input
or disabled quality path. Vehicle0047F730 and wind mesh paths remain required.

15229 focused native ASan/UBSan/leak checks use actual Tutorial definition parser
and87x87 grid with explicit engine/ground leaves: contact excitation/reuse,
one-shot fade, bounce damping, conversion, repeated frame getters, flags, reverse
update, posture/FPV/visual writes, query/recycle and teardown ordering. Full actual
i386 O2/Werror application and startup-probe build plus stage-only PASS. Native
fixtures/input were fingerprinted then removed. Agent did not execute Windows,
original game, DLLs or Proton. OWN source manifest and prior binary are unchanged.
Interactive review overlay stays uncommitted pending desktop acceptance; full
recoverable tracked/untracked patch is fingerprinted in receipt. No new function
credit or promotion;0047BCC0 remains partial.

Research index preceded lookup. Reused C/ABI exports in WORK/loading-decoration-
abi-original and grid-original, H8 geffects-cold-owner-dependencies/0047ea60 and
frame-overlay-and-volume-owners/007885b0. Raw0047FAE0/0047F200 and cell-retirement
assembly plus corrected DOUBLE threshold/inline target constants are retained
under WORK=analysis/work/p22-frame-closure-20261117. H8 original PE SHA and export
provenance follow the previous loading-decoration contract.
