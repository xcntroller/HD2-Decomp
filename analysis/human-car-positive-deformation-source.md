# Positive Car deformation and Door visual restoration —2026-11-12

D2 and D3 are complete in source, following603dd1b/D1. H2/P22-HUM/P2.2 remain
OPEN. Worktree `HD2-Decomp-p22-hum`, branch `feature/p22-hum`. No application
loading, Windows output/original EXE/DLL/Proton execution or desktop acceptance.

## Original evidence and corrected boundaries

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Ignored WORK `analysis/work/mission-world-20261103` retains
`human-car-positive-deformation-workers-index.txt` and the refreshed
`human-car-positive-deformation-frozen-index.txt`. Initial new-worktree lookup
through external cache links lacked inventory entries; this was NOT evidence of
absence. After the local copy-on-write research snapshot, the actual frozen
inventory confirms004FEFD0(2272bytes) and00500CB0. Both now earn partial credit.

Read-only rollback `ExportHumanCarVariantFinalABI.java` produces
`human-car-variant-final-abi`, using bounded raw
`human-car-positive-variant-worker.asm`, `human-car-positive-light-worker.asm`
and `human-car-deformation.asm`. Earlier impact-damage-abi mis-types004FEFD0
as one-argument/RET4; actual500818..50081E pushes index, optional impulse and
allow-detach, and004FF8AD returns12. Use the final THREE-argument export.
694/698 are FASTCALL: ECX context, EDX index, TWO stack pointers/RET8.
Earlier C's invented extra callback arguments are not real inputs. FTOL's ST0
expression is still lost by the decompiler; use raw004FF267..004FF298.

FA8+28 points to the existing seat-associated **Door**, not to an occupying Human.
004E2E30 matches its visual20 to a deformable part and writes index72. Existing
CarAuxiliaryGeometry discover_doors/discover_seats/bind_parts already produces
that graph. Its mutable Door.frame is the current visual. The earlier plan's
assumed Human seat-entry prerequisite for D3 was incorrect; D3 also applies to
unoccupied Cars. Actual Human occupancy/control remains H3/D5 work.

## Same-owner dispatch and lifetime

CarColdReceivers binds actual light/part/wheel damage functions over its SAME
CarColdInitialization, geometry, body, wheels, dirty flags, paired projector
flags and installed callbacks. GameCarCallbacks hit58 now calls the existing
Car loading lifetime's deform_hit at the original post-velocity deformation
point; no second stack damage graph or random stream is constructed. Its
Car→Car force, mass restoration, health/passenger ordering remains unchanged.
The virtual C++ adapter is typed source composition, not original vtable ABI.

Glass delegates to the SAME loading owner's existing00500DA0 implementation,
including applied names, texture exchange and actual69C fragments.694/698 use
the SAME installed Item12 callbacks; their missing real producer failures are
not replaced.6B0 remains installed genuine empty004D49F0. No arbitrary new6B0
payload/ABI is claimed. Hooks fail if reached without their actual binding.
Body/receiver members are destroyed before the initializer allocation; they
remain available during the existing retirement phase and are retracted before
reactivation replaces the initializer. No external borrower is published early.

## Actual004FF8B0 positive damage

Initial light damage computes the common amount ONCE, before reading the light
list, with native568 absorption/558 scale/560 maximum and one FLOAT spill. Reverse
eligible lights use current world positions and the native extended squared
radius test. Subtract the stored amount, publish health, set bit1 below564, clamp
negative health, then update paired presentation when the sign flag is set.
Signed part1A filtering is preserved; no unrelated owner preflight is added.

After mesh capture/deformation and6B0, type2 updates the running impulse, dirty1
and mesh rebuild precede positive part damage. Unless part2 is already set,
subtract absorbed/14*1C from health, FLOAT-store/clamp0, then004FEFD0 BEFORE
publishing part2/dirty2. Glass uses00500DA0. Positive type5 health updates matching
light records; zero health calls actual694 before extinguishing matching lights.
An exception preserves these native earlier mutations and prevents later work.

Wheel processing uses the actual reverse axle-sorted runtime vector after point
conversion by the existing inverse pose. Skip40000400, require240<threshold
impulse and strict local distance<1. Dirty2 precedes the10000000 latch. Absorption
238,23C and234 arithmetic stays extended until the health FLOAT store; native
FSTS keeps the unspilled result for comparison. Detach ONLY for a strictly
negative result, with velocity from actual wheel40/current COM/angular motion.
An exactly zero result latches without detachment. Repeated latched hits retain
health and avoid another callback. Fresh feature4000 reads are retained.

The raw mesh-hit sum is (X²+Z²)+Y²; light/wheel tests use (Z²+Y²)+X². D2 corrects
the earlier D1 sum association while retaining its known XYZ spill golden.

##004FEFD0 variants and00500CB0 paired lights

A dead type8 Door first kills attached, undetached glass using the actual parent
relation and existing00500DA0. Variant selection uses current health/max1C and
count: the original reciprocal interval and reciprocal reconstruction precede
FTOL; do not algebraically replace those divisions. Invalid FTOL ranges fail
explicitly rather than introducing undefined C++ conversions.

Reverse alternatives hide/update except the selected count-minus-FTOL row. Its
visibility is health>0 OR type1. Copy the base XYZ only if destination flag80
allows it, preserve original frame flags, then frame24. Only AFTER that returns,
update the matched Door's current visual. Hide base without a new base24.
Kill matching/parented lights in native predicate order. Then reverse child
parts: use their final variant row, preserve position-copy/update order, zero
health/set2, glass-dispatch or allow-detach694 with the original optional impulse.
No new child Door link or pose owner is invented.

00500CB0 resolves the SAME paired projector through its existing index/visual
mapping. It ANDs flags with mask1/2, chooses texture2 for remaining1 or texture1
for remaining2; remaining0 retains the current texture. Always update the visual,
set the companion light power1 and update it, even when no texture was selected.
The existing paired_flags owner/reset remains shared with CarColdInitialization.

## Restoration and a corrected D1 integration miss

Both004EC5C0 retirement and004E9DB0 physical reset now invoke ONE
restore_car_deformable_parts binding with current resource/parts/physics and
Door graph. D1 had updated retirement but missed the physical-reset lambda,
which still omitted the resource key and therefore skipped keyed backups.
That concrete missed binding is corrected here.

004FE910 reverse restore retains D1 mesh and glass ordering, then repairs the
first reverse matching seat-associated Door.frame BEFORE alternative/base
visibility and health/flags. It does not require an occupying Human. Source
uses the already populated seat→Door indices and Door.part, not a raw pointer
cast or new attachment registry. A failed selected frame24 leaves the previous
Door visual; subsequent restoration repairs it through the same owner.

## Verification and limits

`human-car-positive-deformation-native.cpp` passes ASan/UBSan/LSan:100 D1 regression
families and100 positive damage families. Actual source capture/damage/variant/
paired-light/restore functions compose over SAME part/physics/body owners.
Door discovery, seat association, part binding, initial variant visibility,
variant replacement and restore are exercised through their actual source
producers; no synthetic occupant or manually seeded Door record is required.
Tests cover FTOL thresholds, position locks, attached glass/children, allow-detach
and optional impulse, paired masks0/1/2, strict wheel zero/negative behavior,
reverse wheels, dirty/latch state and interrupted frame/detachment/presentation.

Engine/frame/physical coefficients and glass name/hide/detachment endpoints are
explicit doubles. Access checks are disabled only for fixture setup of known
physical/projector coefficients; there is no loaded-geometry/engine/trajectory
or complete native object/container/allocator/SEH/virtual ABI claim. Independent
Decimal100 oracle `human-car-deform-damage-oracle.json` matches light health
427F7084 versus wheel427F7083 for identical inputs; D1's XYZ golden still passes.

Strict actual nested World/Car/Human/module13 i386, actual loading/callback damage
hooks and common restoration binding pass. Final launcher/probe compile/link
passes. Heavy compilation and Ghidra were sequential. Temporary audits remain
ignored for the ongoing H2 milestone. No original or Windows output was run.
[Receipt](checkpoints/p2.2-human-car-positive-deformation-20261112.json) adds2
client partials/extends5:34 complete/2999 partial(2994client+5engine), denom18783.
Engine callees, unchanged glass/Item12 bodies and vector insertion get no new credit.

D4 authority004D2790 periodic6/pending damage/timed DM and presentation004D3110
queued voice/other reached frame consumers remain open. D5 non7/occupied control,
D10 containing Human, D11 confirmed unproduced5F9/695/EXPL8/smoke14, D12 closure
and actual application/desktop delivery remain explicit dependencies.
