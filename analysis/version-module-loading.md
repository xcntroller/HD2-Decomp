# Versions7 loading — source checkpoint, 2026-11-10

Full first P2.2 checkbox remains open. No containing application world, ordered
loader, first frame or new desktop acceptance.
[Receipt](checkpoints/p2.2-version-module-loading-20261110.json).

## Original loading and owned model parts

0043DD40 clears/rebuilds on authority OR !(mode&1).0043DC90 tail is RET4;
0043DD90 finish uses the same predicate and clears only.0043E3E0 enumerates the
actual scene (not its root sector), callback0043E400, userdata0, maskFFFF.
Callback lowercases a bounded1024-byte copy of frameF0, requires substring
version and kind!=9, then parses version(default100), particle(default-1),
matid(default0) via0043E240. Embedded keywords count. trans is another substring.
The material0 diagnostic does not stop registration. Native decimal scanf
prefix/sign/whitespace behavior is retained; overflowing integers and the
original buffer overflow reject safely.0043E400 is absent frozen: no credit.

0043E560 requires nonnull frame100 model owner/kind9. ModelEC word5 identifies
the SAME containing model master; an unknown owner rejects rather than adopting
foreign storage.0043C220 retains the model, publishes modelEC5, creates its
0043DDB0 collision snapshot, and records the master index. Constructor flag28 is
FALSE, independent of the trans argument: trans goes to the collision constructor.
0043C440 skips an already-bound partEC5; otherwise0043ADB0 retains the frame,
publishes masterEC5/partEC6, records version/particle/material/trans/index and
starts its four resource arrays empty. Source masters/parts use unique owners;
no second ActorFrameData store or scene/grid is created.

No version-change/mesh extraction producer is supplied. MasterC and flag28,
part20/30/40/50 resource arrays and the borrowed model-reload list remain genuinely
empty. Loading does not report success for a missing populated mutation receiver.
The source world facade borrows actual Startup registration7/mode; its empty
native vector capacity is a source STL/lifetime adaptation, not ABI equivalence.

## Actual model collision snapshot and retirement

0043DDB0's reached input is kind9, guaranteed by0043E560; its separate kind6
branch is not offered. The actual driver aggregate bounds supply centre/radius.
Descriptor flags40 (or100040 for trans), mask20000, model frame, min/max scaled
by LOCAL frameD0 scale, centre transformed with world matrix. Copy world matrix,
normalize its three basis rows by LOCAL scale (x87 reciprocal retained extended),
then take the actual inverse. Radius=max WORLD scale × local aggregate radius;
recompute transformed centre, insert/move in the SAME world collision buckets.
The existing frame/backend bounds, transforms, inverse and scale bodies are reused.
Invalid aggregate X bounds or singular/nonfinite local scale reject the native
undefined/unusable branch. No alternate physics body or origin is fabricated.

0043C320 queries actual model world bounds lazily with frame58, then compares
sphere radius3C<=FLOAT0080F550=.10000000149011612. Immediate collision retirement
calls0043E1F0→006EF5E0; otherwise0043E830 APPENDS the collision owner for later
retirement. Clear modelEC5;0043C1A0 destroys parts in reverse then releases the
model.0043AEF0 clears each partEC5/6 before frame release; no active resource
arrays need invented retirement.0043E2C0 traverses masters in reverse, then
pending collision owners in reverse, then borrowed model reloads (genuinely
empty while cold flag28 remains false).0043C9F0 is also vector INSERT, used for
that future reload list; earlier research labels calling it unregister were
incorrect. It receives no credit here because the positive producer is absent.

Source cleanup additionally removes registered descriptors after a failed
construction/bounds query and retracts matching metadata before releasing owners.
Unknown foreign metadata is not overwritten. Typed source guards/extra retained
frame-store lifetimes do not earn original body credit.

## Evidence and verification

WORK=analysis/work/mission-world-20261103; original client SHA starts8c52ce66.
Read-only captures: remaining-module-world-boundary/0043E2C0,0043E3E0;
remaining-module-enumeration-callbacks/0043E400;
version-and-decoration-module-dependencies/0043E240,0043C320,0043E1F0;
version-frame-physics-required/0043E560,0043E830,0043C1A0,0043C9F0;
versions-construction-required/0043C220,0043C440;
versions-part-collision-owners/0043DDB0,0043ADB0,0043AEF0.
The constructor export stopped at absent0043DD90 after two valid outputs;
versions-finish.asm supplies its exact scalar gate/call. Other static evidence:
versions-master-constructor.asm, versions-collision-constructor.asm,
versions-callback-argument-order.asm and versions-callback-parameter-abi.asm.
Native callback register arguments and constructor trans placement were checked
against actual instructions. No original EXE/DLL/Proton/Windows fixture ran.

Focused native ASan/UBSan/LSan uses actual module/model/part owners,
ActorFrameDataStore and DynamicCollisionBuckets with explicit frame/geometry
endpoint doubles. Covers positive multiple models/parts, decimal defaults,
metadata publication, scaled bounds/normalized matrix/inverse/radius, trans
collision flag, duplicate part, both mode passes, immediate/deferred retirement,
foreign metadata, invalid bounds/scale and all20 reached frame failure prefixes.
Repeat starts from cleared metadata/grid; every retained owner is released.
Actual original files are not claimed to contain those synthetic matches.

Strict complete actual i386 Human/VM/SOUND/ShotMgr/Particles/HeavyFire/Versions
type graph and application compile/link pass. WorldActors/Car runtime/session
remain borrowed construction boundaries. Temporary audits/binaries hashed and
removed; research/logs kept.27 tracker checks pass.13 new partial bodies,
zero complete promotions; totals34 complete/1,915 partial(1,910 client/five engine)
/18,783 frozen. Continue GEffects1, actual containing dispatcher/session/loader,
meaningful guard observations and camera/update/render/failure/repeat.

## Positive transition continuation — 2026-11-12

The earlier cold-array/resource/changed-reload limitations are superseded by
[actual damage transitions](version-damage-transitions.md). The SAME Master/Part/
metadata/buckets/category5 cache/script/session/Physics/effect owners now supply
lazy stages, visual replacement, broadcast/received stages and ordered positive
retirement. Save/replay and actual application/desktop delivery remain open.
