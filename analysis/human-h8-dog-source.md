# H8.4 Dog shared animations and damage source — 2026-11-17

H8.4 and HUM remain open. This checkpoint reconstructs the Dog animation table,
its real process/cache owner, skeletal damage geometry and D4 damage policy.
It does **not** enable Dog activation, its live AI/Body frame, or the HUM melee
Dog callback. Those guards remain until their required owners compose. D11 is
already source-resolved under the user-authorized four-state compatibility
policy; this work adds no new compatibility defaults.

## Original evidence and completion boundary

Image SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research index lookups precede new exports. Evidence is under
`analysis/work/mission-world-20261103/h8-4-dog-*`. Read-only/no-analysis Ghidra
exports use temporary transactions, rolled back without saving the project.
Original EXE/DLL/Proton and compiled Windows artifacts were not executed.

Vtable008120E0 pins activation005D5290, deactivation005D5390, authority005D5620,
presentation005D5810, D4 **005D7180** and material005D4CB0. The D4 leaf is absent
from the frozen function inventory and receives no new count. Its native ABI is
thiscall, **six** stack arguments and RET24: kind, amount, impact, direction,
source actor, extra. Direction/extra are unused; source is the fifth stack
argument. Raw005D71C1..71E3 proves FLOAT old-health spill, subtraction of the
ST0 scaled amount, then `__ftol`; the untuned C export omits that subtraction.

Typed-v1 export is rejected: Ghidra's explicit Float10 return type introduced a
false hidden result pointer. Typed-v2 uses ST0-compatible double to clarify
pointer arguments, but its inferred floating spills are not authoritative.
The raw damage/factor instructions establish extended arithmetic and actual
spills. No typed export establishes original object ABI equality.

## Animation producer005D6700

`DogAnimationTables` decodes **23 rows of24 bytes**, `Tables\\dog_anim.sav`,
read size0x228. Aligned raw005D688B proves each row's name starts at byte0 and
DWORD metadata at byte20. Earlier C's apparent name-minus4 is stack drift.
The suffix pointer0084E8E4 targets an empty string00867980. No extension is
invented. Metadata copies before bounded name decode/resource resolution.

Only a nonempty row with a NULL clip is processed. Acquired rows remain owned
across later failures/retries; publication is not Human-table transactional.
Resources use existing `GameAnimationResources` and the actual category3 cache.
Missing resources remain NULL and the native table routine can still return
true.005D5290 gates repeat loading on **row1 clip008AC4BC alone**, which says
nothing about other required clips. GameAnimations preserves this test without
publishing Dog readiness.

For a clip, virtual14 finds root **"anim"**; negative root index or virtual18
position count writes stride0. Count0 is accepted, as in the original. Native
virtual1C receives clip/index/key0/time-output/position-output.0040E0B0 uses
XYZ sum, FLOAT spill, signed-bit tiny threshold0x322BCC77, unit epsilon and
extended sqrt; position Z<0 negates the stride before storing FLOAT.

The table lives in the actual process `GameAnimations`, beside Human tables,
and releases before cache/engine teardown. The containing World binds that
SAME owner to frame services before any Scene2 Dog construction.005D4B00's
69DWORD reset now reaches this table instead of an unused per-World mirror.
Unbound cold construction fails explicitly. Source reset releases owned refs;
native construction only zeroes raw pointers and can leak old table references.
That ownership repair is not original behavior credit. Hot/mixed-world Dog
activation is not claimed.005D5390's future containing teardown must invoke the
same release/reset after Dog controller/Body retirement.

Malformed names and successful short reads would expose out-of-bounds or
uninitialized native stack bytes. The source rejects them; neither condition
is interpreted as an empty successful table. Engine sampling of a present clip
remains the actual checked engine endpoint, not a synthetic root stride.

## Damage geometry and policy

`DogDamageGeometry` recovers005C7D30's actual borrowed bones in order:
Actor478=`f`,47C=`d`,480=`c`,484=`h`,488=`p`,48C=`t`,490=`l`, child-find maskFFFF.
It does not manufacture a retained model or imply that cold005D4D70 produced
Body state. Every damage calculation updates/reads all seven live world
matrices before selecting distances. Missing frames remain required failures.
Only native XYZ lanes are consumed; undefined W padding is not meaningful.

005D73B0 starts with bone480/0.65 and checks478/2,47C/1.5,484/0.2,
488/0.2,48C/0.2,490/0.2 using strict closer comparisons. Earlier winners persist
on exact ties. The first four distances use extended differences and Z/Y/X
sums; closest-distance values spill FLOAT. The final three use XYZ constructor
spills, with0041CFD0's XYZ order for the last bone.0040E080 writes `point-l`
into a local; the caller's impact is unchanged. There is no averaged bone pair.

Actual initialized `MissionIntegrityTables` index39, from ushort008CAB7E and
008CB27E, compares expected/observed values. Mismatch multiplies the factor by
FLOAT0.1 in extended precision, then damage, without forcing equality or
adding a protection-service success. Exact amount**-1** bypasses geometry and
integrity and sets health0. Other amounts subtract scaled damage from FLOAT
old signed HP and truncate toward zero. Nonfinite/out-of-signed-range results
fail explicitly, consistently with existing source damage adapters.

`DogDamageView` borrows actual fields; it supplies no default producer for
initial HP4B0/current action3F4/Sight53C/540/SOUND548. Mode bit1 and positive
signed current HP gate all effects. Lethal damage clamps HP0, marks controller
dead005CE6E0, requests Body action5/priority0, removes Sight source then listener
then SOUND. Each ID becomes-1 only after its real removal returns. Source and
listener IDs require nonnegative values; SOUND uses !=-1, including negative2.
Survivors request action6/priority2 unless current or next action maps to
category6 (19/20), then dispatch actual control+54(kind,source), then request
Body timed action2/10000 only below signed initial HP/8 truncated toward0.
Action categorizer005D86F0 is reconstructed for all signed action values.

These mandatory receivers have no successful defaults. Death005C8490 must
still cancel the actual five-record queue, run005C88B0's ground/death setup and
remove real Body collision descriptors. Control+54 must use the actual selected
controller and source actor. Standalone source verification is not their closure.

## Verification and remaining work

Temporary native ASan/UBSan/leak audit passed **45,408 checks /32 repeated
families**, including actual cold Dog/common actor storage/lifetime, table
reset/retry/resource and sampling failure prefixes, every bone/missing frame,
nearest-distance oracle1024 cases per family, ties, real integrity-table state,
mode/dead gates, lethal teardown failure order and survivor action/control/timer
ordering. Boundary doubles: engine frames/math/resource cache and the required
live Body/controller/Sight/SOUND callbacks. Backup/live fields in the damage
view are fixture inputs, not claimed activation producers.

Strict actual World constructor/destructor/process/table/cache/geometry i386 O0
full link passed, as did launcher/startup-probe compile/link. Windows artifacts
were not executed. Tracker exact checks and27 existing tests passed. Three
new frozen partials005D6700/005D73B0/005D86F0; no complete promotion or absent
D4/engine credit. Counts34 complete/3474 partial, frozen18783. Temporary audit
sources/binaries were fingerprinted in the receipt and removed.

The next boundary is the full005D5290→005D4DB0→005C8370→005C7D30 live owner and
005D5390 retirement, then selected kind4/5 controller and Dog containing frame.
005C6E10 calls still-required005C6D10. Body005C0F50 takes one pointer: raw005C8069 pushes its3B0 argument well
before005C80A2, and the callee reads EBP+8 at005C0F59. A prior no-argument
inference from only the immediate call prefix was incorrect. The remaining
vector call signatures still need review before translating that body. Kind4 +04 is005D5B00;
005CF0B0 is **not** that vtable slot. Do not infer a NULL-input constructor bug
from005CE110 without following the real +04. Dog SOUND005D6000 is a separate
receiver and cannot register the Human callback as a substitute.
