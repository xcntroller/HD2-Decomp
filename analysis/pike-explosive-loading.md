# Pike and ExplosivePlace loading — source, 2026-11-09

P2.2 remains open; GameApplicationState still declines loading. This checkpoint
recovers required type28/type22 collider, visibility and initial update paths
using the SAME world frame/actor/grid owners. No original execution, live world
or desktop acceptance is claimed. Continue the full checkbox after committing.

## Required original behavior

Pike vtable00811C90:14→005B9470,18→005B9340,3C→005B9670,40→005B9BC0.
The captured Tutorial Scene2 contains one `Zavora1.Cylinder01`, state184=0,
axis=(1,0,0), all three sound IDs=-1. It is genuinely stationary initially.
005B9670 calls common activation on both passes and performs derived work once:
save actual local quaternion, zero angle1A0, construct flags40/mask800 descriptor,
then005B97B0(insert=true), emission world position/group1/zero flags and voice=-1.
Mode1 with state1 needs the unrecovered command/rotation receiver and rejects;
states2/3 also reject their required movement rather than passing as stationary.

005B97B0 uses Dummy6 frame+120 bounds; centreX retains extended addition until
half scaling, Y/Z store the sum first, and radius is half the local Y extent.
Other kinds use actual aggregate bounds/centre/half-diagonal radius. Bounds
multiply corresponding frameD0 local scales. The actual world matrix is copied
to owned storage and its three basis XYZ rows divide by those same local scales,
then invert. Descriptor matrix pointers target these stable owned matrices.
The sphere centre uses the ORIGINAL full world transform; radius multiplies the
largest actual world scale. Original inserts when requested, then always moves.
Zero/nonfinite local scales and invalid aggregate X bounds reject undefined
native geometry.005B9BC0 performs common deactivation first, restores/normalizes
saved quaternion, updates, removes collision and clears130, then stops live voice.

005B9340 genuinely returns for states0/1 and mode2. Other authority states require
actual rotation/obstruction work and reject.005B9470 first subtracts wrapping
elapsed from nonzero188, returns while its signed result>0, otherwise resets it;
stationary states return. Moving states explicitly reject missing rotation/audio.
No successful motion substitute or future gameplay completion is claimed.

ExplosivePlace vtable008134F0:14→0060C370,18→0060C330,3C→0060C3B0,
40→0060C6B0. All three captured Tutorial records (`d_ebplace_1/2/3`) have
flagB8=0, caption9C=0x517, actual itemA0=67, delayA4=30000 and flag98=1.
0060C3B0 calls common activation then once builds flags40/mask2 collider with
actual aggregate bounds, live world-matrix pointer, owned inverse, transformed
centre and scaled radius. Actual frame+48 resolves `explosive_on`/`explosive_off`.
It calls0060D380(state0 unless B8). Initial unarmed state clears countdown,
hides on, shows off unless B9, localizes caption9C and inserts only (no move).

0060D380 writes94 before the native missing-on/off-pair return. That genuine
branch makes no visibility/caption/registration calls. State1 shows on/hides off,
sets actual delay and uses the SAME catalogue item's caption14, inserts unless
flag170. State2/3 hide both, clear countdown, use exact original exploded/hidden
strings and remove. State4 clears countdown, shows both and removes. Equal state
returns; unknown state only updates the actual model.0060C6B0 clears90, selects
state4 before common deactivation. Source failure guards detach published
collision descriptors even if insertion threw before170 was set, or a later
visibility/removal callback fails. Bucket owner must outlive actors.

0060C330/0060C370 preserve flag98/signed-countdown guards and wrapping elapsed.
Presentation skips mode1 and clamps expiry to0. Authority expiry explicitly
rejects its unrecovered0060D870 detonation/effect receiver. Captured initial
state0 leaves countdown0, so the actual cold first update does not detonate.
Arming state alone does not imply detonation/gameplay support.

## Verification and continuation

[Receipt](checkpoints/p2.2-pike-explosive-20261109.json) records focused native
ASan/UBSan/LSan using actual reconstructed common actors, properties and dynamic
buckets: captured one Pike/three ExplosivePlace records, scaled Dummy/aggregate
geometry, matrix ownership, state/captions/countdowns, cold update, repeat and
partial insertion/removal/visibility failures. Engine frame/matrix/localization/
audio endpoints are boundary doubles. Strict actual-host i386 instantiation
includes GameWorldActors' SAME catalogue/registry/buckets; application compiles
and links. Windows fixtures are never run.27 tracker tests pass.

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Ignored WORK=analysis/work/mission-world-20261103 contains actor-activation/,
pike-explosive-loading-required/ (ten selected bodies including existing
activation exports), pike-collision-publication.asm, explosive-place-activation.asm,
pike-cold-defaults.asm, next-derived-scene2-properties.json and
next-derived-actors-properties.json. Research exports/helpers and delegated
engine calls earn no credit; six implemented frozen bodies receive partial only. Four implemented entry points
(005B9670/005B9340/0060C330/0060C370) are absent from the frozen inventory
and receive no new credit.
Continue Fowl/Animation/SupplyBox/StaticWeapon/Car and actual containing Human/
session/VM/FPV/HUD/SOUND owners, then ordered loading/first-frame/failure/repeat
and user desktop acceptance. P2.2 checklist scope is unchanged.
