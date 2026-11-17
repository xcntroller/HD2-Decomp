# Dog frame history, turn, ground query and shared Sight/audio — 2026-11-17

This continues [Body bootstrap](human-h8-dog-motion-bootstrap-source.md) on the
SAME Dog/World owners. H8.4 remains OPEN: actual selected controllers, activation,
containing collision-motion and damage/retirement are still required. No live
activation is enabled and no application/desktop observation is claimed.

## Native contracts

-005D64E0 now uses actual Actor580/590/5A0 in DogFrameHistory.590 is copied to580
before the model callback, then fresh model XYZ590 is subtracted with FLOAT
stores. The sum(X²+Y²)+Z² remains extended. Only strict ordered square<original
FLOAT1e-16 skips scaling. Otherwise factor=-1000/exact SIGNED PREVIOUS5A0 remains
extended until XYZ stores; new elapsed5A0 is published last. First moving-frame
zero division retains native NaN/Inf. Missing actual position/elapsed producers
fail at their consumption, preserving the earlier copy/matrix/delta writes.
Activation's history producer is supplied as a callable owner API, not yet wired
as a successful live005D4DB0 entry.

-005C89C0 turns supplied Body180 toward snapshotted actual Actor40 using existing
007885B0 planar angle and actual engine quaternion/matrix/vector rotation. The
late controller-kind read, posture query, DOUBLE epsilon, FLOAT angle/step spills,
extended step comparison and original-Y restoration are retained. Kind1 factor3,
posture1 factorFLOAT.01 and other factorFLOAT.0025 are multiplied by100 after
(abs(angle)+DOUBLE1). This helper publishes neither model nor Actor direction.

-005C62C0 integrates copied speed/85ms phase and returns the retained extended
displacement. It does NOT mutate actual Body250/254. Separate005C6250 advances
those actual FLOAT speed/phase references. Strict elapsed+phase>85 steps add
FLOAT9.81*FLOAT1/3; remaining time uses native signed __ftol/low32 conversion.
The pure integration retains extended speed between steps; the producer spills
speed FLOAT each step. Elapsed<=0 returns before phase/speed consumption.
Body250/254/1FC are typed FLOAT fields rather than generic integer zero slots.

-005C6380 consumes the actual CollisionMotion request, NOT Body. It tests EVERY
contact, including after an earlier false, using (normalX+normalZ)*FLOAT0+normalY
>FLOAT.707 or flags40 AND mask100. Multiplication by0 is not simplified: nonfinite
X/Z preserve unordered behavior. Mask is queried only after flags40. Missing
contact slots and oversized storage remain honest failures.005C8B10 uses signed
Body24C>5 to return0, otherwise actual4.5.

-0046E3B0 and0056FC80 now reach SAME Dog marker544 through actual World/Runtime
Sight bridges. Unproduced544 is distinct from producedNULL. Dog-originNULL544
reads model position if model exists, then still requires fresh544; a model
callback may replace it. NULL model returns absent. Human-to-Dog head directly
requires544. No successful model-as-head fallback or private marker registry.

-005C5BF0/005C5030 emission now has a concrete World→existing SOUND adapter.
Actor position is copied BEFORE actual Body3EC marker matrix callback; XYZ
marker-minus-position spills FLOAT; model-relative pointer is reloaded afterward.
Actual004374D0 request defaults, flags/entry, group11 and sourceNULL are retained.
The SAME Startup SOUND owner stops actual voice references. Live controller/action
composition still must call these adapters; no fake emission or listener owner.

## Verification

Strict native ASan/UBSan/leaks PASS1641 checks:300 history/previous-time cases,
first moving zero-division, signed phase semantics, retained failure prefixes,
turn gates/late control/posture/Y restore, all-contact traversal and descriptor
short circuits, null/fresh Sight markers, sound position snapshots and fresh
model relative. The85ms gravity oracle independently sums per-millisecond steps.
Engine model/math/audio/descriptor endpoints are explicit doubles.

Strict actual i386 full World module authority/presentation link exercises these
new APIs on real Dog/World/frame/grid/sweep/SOUND types. The turn controller in
this compile-only fixture remains an explicitly named throwing endpoint, not a
concrete kind4/5 claim. Forced O2 launcher/startup-probe compile/link PASS.
No Windows fixture, original EXE/DLL, Proton or desktop execution. Temporary
audit source/binary pairs are fingerprinted/removed in the
[receipt](checkpoints/p2.2-human-h8-dog-frame-20261117.json).

Static evidence in analysis/work/mission-world-20261103: dog-frame-history index,
aligned asm and constants; dog-body-turn asm/constants; dog-body-gravity and
-gravity-step asm/constants; dog-body-support asm; dog-ground-query index.
Original files are linked by those index results. Raw instructions govern x87,
spill and late-read details; neighboring absent functions receive no credit.

## Continuing containing motion

005C3420 and response005C43D0/005C2260/005C26D0 remain next. They reuse the
already implemented0071BAF0 multisphere,007183A0 ray and00718530 sweep on SAME
World. Temporary typed exportv2 corrects these THISCALL stack arguments and shared
exclusion006EFA70/006EFB10 calls. The first typed export used incorrect fastcall
query declarations; it is retained as NONAUTHORITATIVE provenance, not source
truth. Final script aborts on absent starts and rolls back its entire transaction;
no original function is created or project saved. v2 improves stack aliases but
still leaves register/stack-vector ambiguities: use full aligned raw and actual
constructor/rotation ABI before translating them. Unwritten local normal/flags
must not be silently zeroed from decompiler names. Do not repeat the solved clip,
bootstrap, gravity or history audits while resolving this remaining path.
