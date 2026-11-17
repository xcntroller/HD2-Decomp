# Dog containing ground motion source — 2026-11-17

This completes005C3420's typed composition of the existing SAME World multisphere
solver,005C2260 fall response,005C43D0 slope response and005C26D0 terrain alignment.
It also supplies the reached007884F0 X-rotation wrapper and0071F700 diagnostic-line
producer. **H8.4 remains open:** actual selected controllers/activation/outer frame
and the conditional separate normal producer are still required. This is source
reconstruction with precise required failures, not live activation or desktop
acceptance. Reuse [prior Body/clip](human-h8-dog-motion-bootstrap-source.md) and
[gravity/frame/Sight/audio](human-h8-dog-frame-source.md); unchanged kernels were
not broadly re-audited.

## Complete containing motion and response

[Body motion](../recovered/game/dog_body_motion.h) snapshots anchor/category and
ENTRY shared iteration budget before the timer/clip queries. Death timer expiry
publishes374=4 and returns before later operations. Signed counters wrap. Actual
clip displacement supplies movement. Stationary pairs use real Body anchors/radii,
ray masks80002/10000 and the same material table; a positive pair skips the solve
while retaining the normal model/terrain/line tail. A missed pair proceeds with
original gravity9.81 and its pure85ms integrator.

Sphere selection retains category, signed count and DOUBLE.05 vertical gates,
including unordered directionY selecting both. Radii*1.5 and Y+radius*.25 are
native, with actual Actor-to-sphere offsets. Native B10000 flags/88002 layers and
same descriptor exclusions feed existing0071BAF0. Action5 sets budget4 and clamps
downward movement to-.1; otherwise ONLY speed>FLOAT1.65 or unordered chooses4,
including equality choosing3. The source restores ENTRY budget after the standing
branch; earlier query callbacks cannot replace the saved value. The simple-pose
branch instead uses its original static ray. Step retry shifts the same spheres,
scales radii*.9, preserves contact storage/count and resets only actual first
contact normal/position XYZ. No private collision solver or collider list.

Support005C6380 updates actual Body24C before response. [Slope response](../recovered/game/dog_slope_response.h)
preserves its initial contact/action/height/gravity gates, FLOAT vertical and dot,
retained X/Z normalization, FLOAT speed/rise spills, exact signed elapsed in
smoothing, retained fraction AFTER its FLOAT1FC store, .65 rate floor and differing
position product spills. The actual clip group0 rate call occurs LAST; its failure
retains prior fraction/position changes.

[Fall response](../recovered/game/dog_fall_response.h) uses actual first Body
anchor/radius, downward100 ray, depth+DOUBLE.5 or50 fallback and sphere sweep,
with native thresholds and plane/timer stores. The real gravity producer advances
before reached geometry callbacks. Native XYZ movement factor is FLOAT.9; X/Y
products stay extended through add, Z product spills first. Counter264 updates
only its native branch. Supported-hit and timer0 plane clearing retain the exact
-10000.099609375 sentinel. Shared exclusions clear before later movement; source
failure cleanup retracts borrowers without hiding the first required exception.

After response, the containing function publishes same Actor position/material/
gravity flags and evaluates actual model-forward Y with native normalization.
Live dogs call terrain alignment, then a FRESH model position supplies the yellow
movement diagnostic. It always submits that line, including zero-length motion.

## Separate local-normal requirement, without invented data

Raw005C3420 consumes a separate stack normal/byte1 whose producer is not classified.
The source does NOT substitute a zero or infer that ray/contact normals establish
it. Its only possible observable consumer is005C43D0;005C2260 consumes the actual
support LOW byte, not that normal-present byte. If no hit, current action>=5,
height gate rejects or gravity gate rejects,005C43D0's outputs are independent of
BOTH unknown values. The composition uses its existing late normal requirement:
a possibly present normal +NULL causes failure only if all other native gates
admit influence, after solver/support/counter/budget effects.128 concrete arbitrary,
NaN/Inf normal and flag comparisons verify the rejected-gate postcondition.
This narrows a required dependency; it does not add a D11 default or claim a
walking-on-contact success without the actual normal producer.

## Terrain and native publication

[Terrain alignment](../recovered/game/dog_terrain_response.h) snapshots actual Actor
position/direction before exclusions/queries. Two downward2 probes retry separately
with Y+.4. Two normals average; a single hit adds world-up then halves. After
exclusion clear, actual engine AngleTo, Y matrix rotations/operator*= and the
client X wrapper reproduce orientation, direction weight and bounded Y change.
FLOAT/extended normalization and product spills are retained, including retained
X before blend. The reached X wrapper uses FLOAT sine/cosine matrix slots5/10,
6/9 and actual engine operator*=, without a quaternion replacement. Its verified
caller supplies only +/-FLOATpi/2; wider x87 FSIN range/exception equivalence is
not claimed. Positive-path W remains unclaimed.

**Raw no-hit005C31C3..31D3 publishes X,Y,Y,Z to Actor40/44/48/4C**, including
produced W=Z. The decompiler conceals that repeated Y load. Source retains this
original behavior, not a repaired XYZ/W result. Engine methods here are STDCALL
with receiver ON STACK; the native client helpers have different conventions.
Corrected terrain typed-v2 eliminates earlier spurious unbound registers; earlier
terrain typed-v1 uses wrong THISCALL engine signatures and is nonauthoritative.

## Same process diagnostic lifetime

[CollisionDebugLines](../recovered/game/collision_debug_lines.h) belongs to the
SAME Startup HumanPhysicsGlobals as iteration budget0084411C. It represents native
720000-byte BSS buffer008CBC18, count0097B898 and cursor00844168 initially at its
start.15000 lines cap submission before input consumption, with two24-byte vertices,
forced FF alpha and secondcolor0 repeating the first color. Native untouched UV
retains static-zero storage across resets. The source allocates that process
storage through a typed heap owner rather than increasing stack object size.

Actual driver44(0,0),84(permanent identity00816310),88(1,count,vertices,0) endpoints
and reset are supplied for FRAME to bind at its original reset/draw stages.
Those two original starts0071F850/0071F8A0 are absent frozen entries and gain NO
credit. This is a real shared producer/consumer API, not a per-Dog discarded queue.
FRAME installation/ordering and actual rendered output remain external acceptance.

## Verification and evidence

Strict native ASan/UBSan/leaks PASS657 focused checks. These execute actual
CollisionMotion/0071BAF0 and all new Body policies, same typed Body/clip/frame
interfaces, gravity/material/position tails, death expiry, stationary shortcut,
normal failure prefixes, speed equality/unordered and sphere selection, budget
changes across callbacks, 128 normal-independence cases and15000-line capacity/
color/reset/draw ordering. Ray/sweep/engine/controller/descriptor endpoints are
explicit doubles; this is not a loaded actual World or engine run.

Strict actual full World i386 module authority/presentation plus Body motion,
terrain, same descriptor anchors/radii and shared driver-line APIs compile/link
PASS. Its selected-controller endpoint remains explicitly named and throwing;
no concrete kind4/5 claim. Forced O2 application-launch/startup-probe and tracker
checks are recorded in the [receipt](checkpoints/p2.2-human-h8-dog-ground-motion-20261117.json).
Windows fixtures were not executed; original EXE/DLL/Proton/desktop were not run.
Temporary audit source/binary pairs are fingerprinted and removed at checkpoint.

Evidence is retained in analysis/work/mission-world-20261103: aligned
human-h8-dog-body-{motion-full,fall-response,slope-response,terrain-response}.asm;
matching constants JSON; body-terrain-rotate.asm; corrected terrain-typed-v2 and
motion-typed-v2 originals; debug-index/raw-candidates/neighbors. The raw-address
candidate file names potential containing starts, not proven function boundaries.
No absent function start was created. All Ghidra typing transactions rolled back.
Raw controls stack ABI, x87 and publication details; prior exports are not assumed
equivalent by date. New function credit stays frozen partial only.
