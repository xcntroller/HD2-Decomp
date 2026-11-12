# Car mesh backup capture and restoration —2026-11-12

D1 is complete in source; D2/H2/P2.2 remain OPEN. Worktree
`HD2-Decomp-p22-hum`, branch `feature/p22-hum`, created at the user's request from
clean master2c96a7d. Main and the other worktrees are preserved. No application
loading, original executable/DLL/Proton execution or desktop acceptance.

## Original evidence

Client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Ignored WORK `analysis/work/mission-world-20261103` retains indexed lookup in
`human-car-backup-owner-index.txt`, `human-car-backup-methods-index.txt`,
`human-car-restore-fuel-checkpoint-index.txt`, `human-car-deformation-producer-index.txt`
and `human-d0-containing-index.txt`. Read-only rollback script
`ExportHumanCarRestoreABI.java` produces `human-car-restore-impact-abi`.
`human-car-part-restore.asm`/`human-car-deformation.asm` are authoritative for
virtual stack arguments and FLOAT spills. Bounded PE constants are recorded in
`human-car-deformation-constants.json`.

Native004E2900 initializes the654 vector's658/65C/660 pointers to zero within
this physical object.004FF8B0 invokes004FC320 with ECX=this+654 and the current
65C end. This is evidence for an owned per-physical-object vector, not a process
global cache. Each16-byte record retains mesh0, usage count4, part-name8 and
resource-nameC.004E2E30 produces resource7B8 from the actual model134.
Part records remain the SAME668/3C records already owned by CarDeformableParts.

The first impact export incorrectly types694/698 as thiscall; raw5008CB/5008D2
and500C2A/500C2C show TWO pushed values plus ECX context/EDX index. The later
`ExportHumanCarImpactDamageABI.java` corrects these to FASTCALL/RET8 in
`human-car-impact-damage-abi`. Those callbacks are beyond D1's damage boundary;
do not interpret the initial C's invented extra argument as a real input.
Engine mesh/LOD methods put the receiver on the stack and use stdcall.

## Capture and positive mesh mutation004FF8B0

The existing CarColdDeformation is now supplied the SAME dirty word, installed
6B0 callback and a borrower of the existing CRT stream. Cold solver receivers
borrow their existing random function; GameCarCallbacks borrows StartupSession.
No private random generator, physical state or callback registry is constructed.
Installed6B0 is genuine empty004D49F0; source preserves its changed-part call
BEFORE dirty/rebuild. Wider original callback arguments are not newly claimed.

After detached/threshold gates, use native part28 absorption,14 deformation scale
and2C/30/34 displacement limits. Lookup resource/part keys case-sensitively in
reverse. Existing records provide original positions; a fresh request uses the
current mesh. Lock source first and destination second, even for the same LOD.
Only after both locks, transform the point by the actual inverse world matrix;
zero translation before transforming/clamping direction. Hit tests use current
positions, the native FLOAT radius square and strict squared distance comparison.

On the FIRST affected vertex of either LOD, a fresh request allocates copied
names, creates/copies the actual mesh and drops LODs above1, then publishes the
record with count1 BEFORE part flag4 and vertex mutation. The native copy status
is deliberately ignored here, unlike ordinary deformable mesh construction.
A previously matching record increments its count only when this part's flag4
was clear; that flag store precedes increment. A hit in LOD0 suppresses duplicate
capture/count increment in LOD1. A fresh request deliberately keeps its current
source for LOD1 after capture; a later request reads the saved original mesh.

Reverse vertices consume one shared CRT draw each. Preserve native.1/.3/.7
factors and distinct FLOAT spills in X/Y/Z updates, then clamp displacement
relative to ORIGINAL positions. Only the first12 bytes of each32-byte vertex
are written: normals/UV/other lanes remain unchanged. Variant parts perform the
native hit observation without vertex writes, draws or mesh backup capture;
their later health/variant switch is D2 work.

Changed parts notify6B0, update type2's impulse for later consumers, then set
SAME dirty bit1 before LOD rebuild, mesh refresh, visual invalidation0/frame24.
Source retains earlier vertex/backup state if notification or rebuild fails.
The original health/glass/variant/light and wheel consumers still fail explicitly
at their remaining D2 boundaries. A default damage-enabled hit is not falsely
reported complete merely because its geometry was changed. No-hit and damage-
suppressed geometry paths can now return through the actual body.

## Reverse restoration004FE910(0)

Actual004EC5C0 retirement passes the SAME geometry resource to restore_cold.
For each reverse nonvariant part with flag4, reverse-match BOTH resource and part
name. A nonmatching record is a genuine native skip; flag4 stays set. This replaces
the former blanket required error now that the actual cache owner is present.
It does not assert that an unmatched mesh was restored.

Lock saved LOD0 then current LOD0, restore XYZ in reverse and unlock saved before
current. If the saved mesh has LOD1, copy its optional pair the same way. Native
rebuild/invalidation is still attempted for destination LOD1 when saved count>1,
even if its optional copy was skipped. Then rebuild current LOD0/1, refresh mesh,
invalidate visual0/1, frame24, CLEAR part4, decrement count, and only at zero
release mesh and erase/shift the record. Later glass/variant/visibility/health and
6B4 behavior remain the existing source. High WORD flag bits are preserved.

Backup resources belong to the existing part/physical geometry lifetime and are
released on discarded geometry too. Model ownership still outlives dependent
resources. Explicit lock cleanup preserves the primary C++ failure, releases all
successfully acquired locks in source/destination order and records secondary
cleanup failures through the actual cleanup recorder. Native engine/allocator/
SEH equivalence is not asserted for these checked failure adaptations.

Occupied FA8+28 attachment mesh+20 repair before visibility remains D3. D1 does
not claim to provide a missing seat producer or a successful occupied restore.

## Verification and credit

`human-car-mesh-backup-native.cpp` passes ASan/UBSan/LSan over100 repeated families:
actual source impact→capture→deformation→restore, first hit in either LOD, strict
radius boundary, repeated original-relative clamp, shared-key counts, XYZ-only
writes, high flags, factory/notification/lock/rebuild failure, reverse partial
restore, resource mismatch, retry and release. Engine/frame/geometry coefficient
endpoints are explicit doubles. `-fno-access-control` only seeds known physical
coefficients; no backup record is manually supplied. No real geometry-loader or
engine acceptance claim follows.

Independent Decimal100 oracle `human-car-mesh-spill-oracle.json` supplies a fixed
nontrivial update with expected XYZ bits3FA55876/C14A137E/42F7D5D2; source matches.
Strict actual nested World/Car/Human/module13 i386 composition, with explicit
mesh/cold deformation/restore instantiation, passes. Final launcher/probe links
pass in the NEW worktree. No Windows output was run. Compilation and Ghidra were
sequential. Temporary audits remain ignored until the ongoing H2 closure checkpoint.

[Receipt](checkpoints/p2.2-human-car-mesh-backup-20261112.json) extends ONLY existing
frozen partials004FF8B0/004FE910. No new entry or complete promotion: merged
baseline remains34 complete/2997 partial(2992 client+5 engine), denominator18783.
004FC320 is STL vector insertion adaptation, not new recovered function credit;
engine mesh/LOD/math callees receive none. D2/D3/D4/D5, D10/D11/D12 and the reached
entry dependencies remain visible in the [Human plan](p22-hum-plan.md).
