# Items transfer and physics loading — source work, 2026-11-05

P2.2 remains open. These owners compile against actual host adapters but are not
constructed as a containing world. No desktop test or original/Windows execution.
Continue the full checkbox in plan.md. This batch used focused verification only.

## Loading inventory and the Items module

`loading_item_transfer.h` reconstructs007D81A0's exact007D8B40 call: null source,
source slot0, destination actor/slot, index−1, null observer, notify1 and remaining
flags0, with Items7E4 temporarily false. It uses actual actor capacities
(Human0059C2B0→005A6600(1), signed propertyF0; Car004D0E20=500;
StaticWeapon00602F60=100), float weight7C and epsilon0080F53C. Compatibility,
weight, volume, signed definition category and nested pack contents are checked
before ownership publication; failures retain original result−2/−4…−8.

Concrete discrepancy corrected: slot storage native1C counts occupied stacks;
it is not an ownership flag.007D7390 increments it only when filling an empty
stack,007D7470 decrements only when a stack empties,007D7630 checks zero.
Insertion groups definition0C AND instance family08, appends to the first match,
otherwise uses the first empty stack; detach swap/pops inside the matched stack
and rotates an emptied slot to the end. Scalar/volume totals preserve x87
intermediates with float stores between groups. Human slot6 is borrowed pack
storage published by transfer, beyond its six primary owned holders/stacks.
007D32A0 returns signed definition category1C;007D32B0 returns category0→1,
otherwise category×2. Transfer updates actual holder/slot pointers and weight.

`GameMissionItems` borrows the SAME catalogue, frame services, entry registry,
model services, game word28, files and clock. Its007DC6E0 membership index is
borrowed;007DC8A0 first-match swap/pops before matching entry destruction.
An entry destructor observer retracts module/placement indices before nested
pack destruction.007DCB70 originally frees only its raw pointer list. The native
owner additionally guarantees tracked entries die before services/catalogue.

007DCEA0/007DD010 use bool(mode&1)==authority, reset actual identity state,
and authority reads optional missions\\<exact resource>\\items.dat.007D81A0
uses actual record definitions, identity allocation, frame userdata and transfer;
missing definitions/frames and rejected transfers diagnose/continue as original.
Placed records enter the actual shared queue. Field16 drains positive-capacity
weapons; it is not a refill. The destructive007D7BE0 fallback that deletes the
original items.dat is deliberately excluded. Sound/network remain required
concrete consumers, with no default success.

Focused ASan/UBSan check passes425 actual selected inventory records, zero
capacity/compatibility rejections, no missing required actor names; real selected
definitions, Human capacities and holder/stack owners are used. It also checks
stack counts, grouping, detach and teardown. Model endpoints are fixtures.

## Runtime activation and the static physics producer

`GameRuntimeActors::activate` reconstructs00477660's required branches:
reset runtime identity state/field74 each pass; authority OR !mode1 consumes the
static physics queue through004794D0; authority AND mode1 consumes the SAME
Items placement queue, sets actual pose, creates runtime11 and binds real rigid
body/entry. Nonmode7 kind20 records are retained. The flag1D/kind10 ground/effect
receiver still rejects; all24 captured placed records have flag1D zero.

Important correction from constructor assembly00407934…0040794B: the three
00479540 handles are frame, **00407390 physics object**, collision descriptor.
The second handle is not param2[8]'s inventory entry. Runtime35 native90 now
borrows the physics object.004794D0 assigns it only after actual runtime actor
construction/activation, then descriptor48 and006EF7F0. Native rollback removes
queued actors before destroying their physics object/descriptor. No new pickup
or simulation implementation is implied.

## Physics module and ordinary object loading

`physics_object_parameters.h` implements00410B40's lowercase substring tests
via00410AF0/strstr. Empty metadata and model kind9 skip. `can` selects type2 and
first owner list, bypassing subsequent tags. `cross` selects type6. `free` OR
`axis`, `tree`, `break` and optional following model token/dot stripping retain
the original order. Missing break model uses plotdira. `angle` uses atof times
0080F6E0 =0.008726646192371845 (half-angle degrees), not pi/180. Metadata≥1024
rejects the original stack overflow.

`GamePhysicsObjects` reconstructs00410A40's authority OR !mode1 activation,
00410830's reverse first-list then second-list cleanup,00410B00's actual live
scene+44 enumeration(maskFFFF), and0040FB00's stable reusable handle/list
publication through SAME frame userdata word4. Type5 appends its borrowed scene
index. Frame userdata is cleared before releasing the retained frame/handle.
No second frame-data or collision store is created.

`PhysicsObject` provides0040B5E0 common reparent(root scene1F8,preserve-world1),
update, retain, world position/rotation, and0040B7E0 matching unpublication,
collision removal and frame release. Type0's0040E320 disables registration.
Type2's0040E810 uses aggregate bounds, selected/default radius, flags42, mask0
and actual world position. Type5's00407390 uses visual bounds1A0/centre1C0/
radius1CC or actual driver aggregate bounds, flags40 plus parameter17's100000,
mask200000, copied world/inverse matrices, transformed bound centre, and actual
shared bucket registration. It preserves the original opposite-corner XZ span,
centre Y taken from transformed minimum, float stores, cached rotations and
004079F0 restoration. Invalid aggregate type5 bounds reject an unwritten centre.
Pickup parameter1E publishes the actual physics object queue before registration.

Detached motion and break/contact producers remain unavailable. Constructors
1/3/4 were recovered in the continuation below; type6 still rejects before
publication. Type6's second descriptor/register
sequence needs resolution; assembly is preserved without a guessed correction.
A targeted scan of captured Scene.4ds finds126 Tutorial printable metadata spans
containing phobj, all choosing type5; these are string spans, not an asserted
engine object count. Alps1 contains no such spans in that one file. This does
not establish metadata in uncaptured/referenced model resources.

Focused physics ASan/UBSan passes six registration/ownership cases,16 endpoint
failures,126 selected metadata spans and tag precedence/half-angle boundaries.
Strict i386 explicitly instantiates actual Items, runtime actors and physics
owners/adapters. Application compile/link passes; no executable was run.
Evidence: physics-object-loading/, static-item-queue-producer/,
physics-base-lifetime/, physics-finalize-and-queue/, physics-other-constructors/,
physics-*.asm and static-physics-constructor.asm under work/mission-world-20261103.
The failed physics-object-base export requested mid-body0040B710; the log is
retained, and the actual base starts0040B5E0. No behavioral claim uses that failure.

## Continue full P2.2

Recover/integrate actual Scene2 derived activation/colliders and ordered module
cleanup, script contexts/VM, sound/effect/weapon receivers, then construct the
world and wire loading presentation/first-frame consumers. Preserve explicit
unsupported branches. Existing physics/transfer checks need no repeat without
new changes or contradictory evidence. No ledger credit is added for these
partial loading bodies, absent functions or engine delegates.

## Physics1/3/4 and shared fade continuation — 2026-11-09

Types0..5 now have source loading owners.0040F6A0/type1 retains the selected
or default radius at170, but overrides the descriptor radius with geometry
radius times the largest actual world scale. Its descriptor centre is the
aggregate local centre transformed by the world matrix, unlike type2's world
position. Both use flags42/mask0.

0040DAA0/type3 uses flags40/mask2 and transformed aggregate centre. It writes
four local principal-axis endpoints at170/180/190/1A0 **after** registration:
minimum then maximum on each chosen axis. X wins a first-axis three-way tie;
secondary ties choose Y for X-first, X for Y-first, Y for Z-first. X/Z extents
spill to FLOAT while Y remains extended through the comparisons; midpoints
spill to FLOAT. Invalid aggregate X bounds reject an originally unwritten centre.

0040BD70/type4 preserves common root/update/retained-frame lifetime, reads the
actual cached inverse-world component below.5 into170 and sets171false. It
requests5000ms fade-out without hiding. It does **not** register a collision
descriptor and rejects a missing shared fade owner before frame mutation.
Type1/3/4 destructors simply reach common0040B7E0; no invented cancellation.

`effects_frame_fades.h` reconstructs00488D60's out-only200 queue and the
fade-out pass within00487240. Positive durations retain each frame, allow
duplicates and do not set initial opacity. Nonpositive duration sets opacity1
and optionally hides immediately.00486E60 uses ungated **frame+A8(1-amount)**,
distinct from model+50/visual+A4. Reverse update subtracts elapsed as DWORD,
tests signed expiration, applies original FLOAT amount/alpha stores and
retires references by swap/pop. Physics close may release its frame while this
same queue still retains it until fade completion.

Opposite fade-in210 scheduling/cancellation is excluded. Native00488D60 removes
the last opposite-list pointer without releasing its allocation/reference; do
not invent corrected ownership or combine another independently owned fade-in
queue. Current type4 is out-only with cold opposite queue empty. Explicit
source clear/destructor releases retained records; original004888C0 finish
does **not** clear these queues, and that behavior is not falsely claimed.

Type6 remains unresolved: original00406217 overwrites a saved secondary-
descriptor pointer with inverse-matrix storage, and004062A2/004062A7 register
that overwritten pointer while destructor00406300 removes descriptor230.
Actual grid insertion reads descriptor+0C/+30. A guessed safe230 substitution
would change native behavior, so this branch still rejects. Captured Tutorial
printable metadata spans select5; this is not proof for referenced models.

Evidence in WORK: physics-type1/type3/type4-constructor.asm,
physics-cross-constructor/destructor.asm, physics-effects-fade-required/,
physics-effects-fade-lifetime/, physics-effects-opacity-required/ and
physics-effects-fade-finish.asm. Client/engine fingerprints remain unchanged.
[Receipt](checkpoints/p2.2-world-owners-20261109.json) preserves native sanitizer,
actual-host i386 and application results and hashes/removes temporary fixtures.
Six new frozen client partials cover these actual bodies, no complete promotion.

[Shared world composition](world-scene-ownership.md) now binds Physics to the
same frame-data, buckets, runtime actor queue and GEffects fade owner. The
containing production Human/session/module/first-frame receivers remain required.
