# Derived actor activation — source work, 2026-11-05

2026-11-09: [Door activation/first-update continuation](door-loading.md) now
provides actual rotating type6 portal/descriptor, packet/sound, motion and
separate dynamic query source. Human source is linked from
[Human state/loading](human-state-loading.md). Remaining families below still
need their real receivers; full P2.2 and containing-world delivery remain open.

P2.2 remains open. The containing world is not constructed; actor activation is
source-only. Continue the full plan.md checkbox. This batch used focused checks,
not another broad audit of unchanged reconstruction.

## Scene2 module traversal and retirement

`SceneActorRegistry::activate` reconstructs00476210: reset native4C/50, snapshot
identity count, reset each visited reserved generation toFF and call actual+3C
for nonnull entries. Newly appended identities are outside this pass.00476260
first invokes actual00476860(100000) unless mode&7==7, snapshots identity count,
calls+40, then fetches the reserved slot AGAIN and removes it when authority OR
!mode1. Cleanup exceptions are collected while remaining cleanup continues.

00475C50(false) removes active/identity membership before00476680 appends actor/
20000ms, then calls005A7FE0. The new retirement owner retains actor/frame binding
until expiry.00476860 subtracts wrapping32-bit elapsed and expires only when the
signed result is negative, not zero. It checks mode afresh for locked server
recordA2=0, presentation deactivation(false), authority deactivation(true), and
0049EF70(1,actor), then destroys the matching owner and swap/pops the queue,
revisiting the swapped record. Actual server/presentation/Human endpoints remain
mandatory.005A7FE0 cancels real Human control/Sight/sound/effect state; it is NOT
a general no-op or an arbitrary-actor receiver. The host wrapper rejects other
families and requires its real callable. Native guards unbind frame userdata
before destruction, keep pending actors discoverable through base references,
and clear retained pending ownership on failed-world teardown.

Focused sanitizer checks cover seven activation-count/generation/reserved-cleanup,
expiry, mode/order, callback queue growth and failed-endpoint cases. Engine/
controller endpoints are fixtures; this is not Human desktop verification.
Evidence: scene-actor-pending-removal/, scene-actor-removal-producer/,
scene-actor-retire-notification/, scene-actor-retirement.asm and original exports
under derived-activation-cleanup-receivers/.

## Four derived families now have actual bounded activation receivers

Null33:005E2B90/005E2BA0 are genuine common activation/deactivation tails.
Spawn10:00611970/00611990 call common activation/deactivation then set9C=true.

Switch13:00613920 calls common activation, then authority OR !mode1 constructs
nativeF0 collision: actual aggregate bounds, mask1/flags40, live frame world-matrix
pointer, owned inverse, radius times maximum actual world scale, transformed
bounds centre and actor138. Insert/move uses the SAME collision buckets. Tail
clears notification suppression and snapshots actual stateD4→D0.00613BD0 performs
common deactivation, removes collision regardless of authority, then authority
restores the snapshot through actual006136F0 visual/script/sound consumers.
Invalid aggregate bounds reject their original unwritten centre. Native lifetime
guards retract partially published descriptors and reject a changed bucket owner.

Ladder8:005EAD20 snapshots the parent BEFORE common auto-parenting, then restores
that parent with preserve-world1. Once per active lifetime it scales actual local
aggregate bounds by frameD0, copies world matrix and divides basis XYZ rows by
the corresponding local scales, keeping x87 reciprocal intermediates. Invert that
normalized matrix. Top bound uses actual up-marker worldY and transformed topY,
plus0080F5CC=.30000001192092896. Two owned descriptors have flags40/mask4000,
actual frame/actor and normalized world/inverse pointers. First radius is scaled
height plus another0080F5CC. Second minimumY is inverse-transformed upY minus
00812B10=.007499999832361937; minimumZ subtracts0080F550=.10000000149,
maximumZ adds0080F5D4=.25; it shares the first radius. Original inserts both
without006EF7F0.005EB120 performs common deactivation, removes/destroys both
in order, clears flag90 and the borrowed164 list. Native destructor/failed-load
guards detach every published descriptor before freeing it. Zero/nonfinite local
scales reject instead of exposing undefined collision values.

Focused ASan/UBSan passes16 Null/Spawn/Switch mode/authority cases, ten endpoint
failures; Ladder passes both authority values with scaled/normalized descriptors,
repeat activation/cleanup and12 endpoint failures including after first insertion.
Strict i386 explicitly instantiates actual host activation/deactivation and collision
adapters. Affected application compile/link passes; no Windows executable run.
Assembly: switch-actor-activation.asm, ladder-actor-activation.asm and
ladder-actor-deactivation.asm. Original family vtables/targets were already
preserved in actor-activation-map.json; no repeat family census was performed.

## Remaining boundary and immediate work

GameSceneActors now dispatches3/6/8/10/13/22/28/29/33 (Human requires its actual
bound loading owner). [Pike/ExplosivePlace continuation](pike-explosive-loading.md)
supplies actual type28/type22 initial geometry/state/cleanup; required others
reject. [Generated Fowl](fowl-loading.md) now supplies actual mode1/2 initial/update/
hearing/cleanup; borrowed mode0 rejects. Car, SupplyBox, StaticWeapon, Animation
and Dog still require
actual derived receivers. Containing Human/session activation and full P2.2
remain open; historical Human dependency notes below are superseded by
[latest Human state/loading source](human-state-loading.md).

Immediate Human work follows0059F2C0/0059FA60. Assembly0059F5A2…0059F5BA proves
005AB330 initializes the INLINE owner at Human98, with Human pointer and property
1A0 as two stack arguments. Its slotsA0/A4/A8 are three real control owners, not
another actor pointer. It allocates0077DD80(E0),007855C0(DD0),0078CA00(230), then
binds through0077DDA0/DDC0,00785B20/BE0,0078CA60 and0077FC30. Matching005ABA90
must be recovered before implementation; new exports are in
human-loading-dependencies/ and human-model-control-owners/. The cold source's
neutral98/A0/A8 words have not yet been replaced with these live owners.

005ABA70/005ABA80 and005AFD50 are genuine constant1/RET bodies, but Human
activation has real model/controller/Sight/sound/effect requirements.005AFE00
lazily parses Tables\\speech.set by positional entry, with root1000 and member
0(count),1(name),2(categorized definitions); the record is F24 bytes with usage
counterF20. Reading names alone is not a speech service. Preserve evidence and
recover complete dependencies before implementing their containing activation.

Human speech definitions are now source/native verified; [latest Human contract](human-loading.md)
records the actual filename, random ordering and remaining model/control owners.
