# Car glass mesh fragments —2026-11-12

Source checkpoint for the same selected Human reactions/damage/ragdoll dependency.
Full checkbox, actual application loading/rendering and desktop acceptance remain
OPEN. Item12 detachment, timed-DM and full Human entry/frame remain required.

## Original evidence

Original client SHA256 is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK `analysis/work/mission-world-20261103` keeps
`human-car-glass-fragment-index.txt` (lookup/reuse),
`human-car-fragment-math-index.txt` (reached helpers), existing bounded
`car-world-part-mesh-effect.asm` and new bounded
`human-car-fragment-triangle.asm`, `human-car-fragment-parameters.asm`.
Rollback-only `ExportHumanCarFragmentsABI.java` exports
`human-car-fragments-abi/{00484f60,00485ba0}.{c,json}` without saving the read-only
project. Earlier default C is unusably aliased. Even the corrected C has remaining
stack names/virtual-call artifacts; raw instructions define arguments/order.

00484F60 is FASTCALL: positionsA/B in ECX/EDX; stack holds positionC, six UV FLOATs,
source visual, material POINTER and maximum area FLOAT; **RET40**. No GEffects
receiver or third register position.00485BA0 is RET8(frame,area), ignores ECX and
is **absent frozen**, as is004D4A10: both get ZERO new function credit.
004066C0 is pointer-scalar vector multiplication.0040E080 is subtraction with
hidden result;00454660 is extended Z/Y/X dot;0040E0B0 length has native signed
bit threshold322BCC77 and near-one epsilon. Math/engine callees earn no new credit.

##00485BA0 and00484F60

The actual Car69C=004D4A10 still checks visible frame and mesh, invokes GEffectsB4
(frame,.01), THEN hides. `GameCarCallbacks::emit_car_contact_fragments` now supplies
this positive path instead of an unconditional missing-fragments guard.

00485BA0 checks visual1D0 bit800. For a mesh, read LOD0/group0, captured group8 face
count, read LOD0 again and vertex pointer, capture material group4. Reverse faces,
re-reading groupC(0) face-index pointer on every iteration. Positions XYZ and UV
at18/1C use actual32-byte vertices and16-bit indices. Typed bounds/format guards
reject invalid data rather than manufacturing triangles. No invented read unlock.

00484F60 selects longest-edge subdivision with native ties/order:
AB→apexC/edgeA,B; BC→apexA/edgeB,C; CA→apexB/edgeA,C. AB/BC lengths spill to FLOAT;
CA remains extended during comparison. Rebuild normalized edge and projected
height using native scratch/threshold rules. Only area **greater than** maximum
recurses. Children are(apex,midpoint,first),(apex,midpoint,last). UVs interpolate
with those same vertices; Z midpoint sum retains its native extra FLOAT spill.
Invalid nonpositive/nonfinite area or depth>512 fails explicitly; reached Car
producer always supplies positive.01. No capped successful shard count.

Leaves use original input order, not the apex reordering: six vertices A,B,C,C,B,A,
indices0..5. Compute native scaled cross((C-B)*10000,(A-B)*10000), retaining extended
X and Z-related square/product spills and fallback normalization. **Both faces
receive the same normal**, not a corrected negated back-face normal. Source
shading/UV/material pointer are preserved; no material-ID substitution.

## Actual engine and Physics ownership

Actual frame endpoints build visual OBJ_, mesh and LOD, six32-byte vertices,
unlock, create face group, SetMaterial(material pointer), allocate two faces,
write six WORD indices, attach/finalize LOD, attach mesh and assign visual mesh.
Face-group method0 is **SetMaterial, not Release**: preserve the native driver/LOD
transfer and do not manufacture a COM-style deleter for that pointer.

Link the shard to the SAME containing Scene root with flag0, set position from
source world, re-read source world for quaternion Make/Normalize/direct flags,
call visualB0 then24. Required factories/data/root failures throw. Engine factory
references use matching existing owners; malformed/nonfinite native undefined
math receives checked source failure, without claiming original unsafe behavior.

`PhysicsContactLink` expands its existing checked borrower: create and impulse
bind to the SAME actual module2 alongside contacts, validate SAME frames and
unbind all receivers before module retirement. No second Physics registry,
independent object/handle list or new world is introduced. Car callback uses
actual `GamePhysicsObjects::create/impulse`, existing type4 PhysicsObject, frame
userdata, active handles and shared GEffects fade lifetime. Wider module behavior
is reused, not credited or audited again.

Construct native44-byte parameters: frame=shard, type4, first_list=true,
sight_pass=true, material10=ZERO, other defaults/empty callback. The mesh material
pointer above is separate. Real Physics create precedes all randomness. NULL
handle consumes no random draws/impulse. For a valid handle, six **ENGINE RandFloat**
draws: first three produce Z/Y/X position offsets(r-.5), read current shard world,
then last three produce X/Y/Z force(r-.5)*10. Actual impulse(handle,force,point,.1)
follows. This is independent of the CRT stream used by Car parts and Versions.

Only afterwards release local LOD, mesh, visual references in that exact order;
Physics/fade retain their actual frame ownership. Do not release the published
Physics handle when the producer returns. Typed failure cleanup retires factory
references without rolling back a successfully published Physics object. No
invented extra actor ownership or delayed Car destruction.

## Verification and boundaries

Temporary `human-car-fragments-native.cpp` passes ASan/UBSan/LSan100 repeated
families using actual subdivision/PhysicsContactLink/parameter source. Check equal
area, BC branch/child order/UVs,64-shard area conservation, original reverse faces/
shared normals, six engine draws/order/point/force, null handle, source nonmesh,
reverse face callbacks, Physics failure and failure after handle publication.
Engine frame builders, face/vertex buffers and Physics create/impulse are explicit
boundary doubles. Native test does not execute actual engine factories or Physics
motion/fade, and does not establish original trajectory/whole-body equivalence.

Strict actual nested World/Car/Physics/contact/DM/Human/frame i386 composition and
launcher/probe compile/link pass in `human-car-fragments-*`; heavy jobs sequential.
No original EXE/DLL/Proton or Windows fixture executes. Actual application loading,
rendered mission frame, full Human entry/failure/re-entry/retirement, timed-DM,
positive Item12 and user desktop acceptance remain required. No full checkbox
closure or complete-function promotion.
