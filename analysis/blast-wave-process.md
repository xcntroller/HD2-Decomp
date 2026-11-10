# Shared blast-wave process — 2026-11-10

Source continuation of the FULL first P2.2 checkbox, after
[frame/render consumers](frame-render-source.md). Application loading still
returns failure. Both P2.2 checkboxes remain open; no first-frame/desktop claim.
[Verification receipt](checkpoints/p2.2-blast-wave-process-20261110.json).

The previously required008AED48 transient owner manages delayed blast hits,
not projectile trajectories. It is DISTINCT from proximity008AED54. Startup now
owns one `BlastWaveProcess<MenuDriver::Owner, actual SceneActor>`; the actual
World bridge borrows it for006DF390(elapsed,false) and the actual ordered finish
receiver006DF390(0,true). `GameBlastWaves` implements GameSessionTransientFinish;
it is no longer necessary to supply an invented empty finalizer. The application
still does not construct this World/bridge, and fixed frame services remain open.

## Construction and retained ownership

006DF170 constructs006DDFC0, advances100ms immediately, then appends only on a
true result; an immediately consumed/empty wave STILL returns1. The process can
hold a record with an empty node list until the following tick retires it.
Record copies centre, power14, range18, source1C, same-sector24, and actual scene
sector28; progress10 starts0. Candidate mask isD8000.006FCB60 gathers unique
nearby descriptors in native Z/X/bucket order using horizontal centre distance;
006FB920 supplies the first containing descriptor separately. Its frame is
added first. Radius descriptors stop at the first null frame. Human direct-EC
kind3 maps to actual child 'a'/FFFF with the original repeated lookup/dedup order.
Camera command1 receives centre and truncated ORIGINAL range before node work.

Candidate frames are retained in reverse. Actual world matrix/inverse transform
the blast centre into frame-local coordinates. Kind6 bounds come from120;
other bounds use driver+A8 (DISTINCT from Car+AC). Node14 is the SQUARED distance
to local bounds, with negative guard to0; it can increase record range18. Node
XYZ is refreshed frame-world translation minus blast centre. Do not replace this
with ordinary distance or a geometrically improved bound.

Native linked insertion compares every node EXCEPT the tail: a one-node list
always appends the new node, even when its threshold is smaller. Source preserves
this observable ordering; a nearer node can wait behind the farther head. Node
retirement releases its retained frame BEFORE recursively retiring next18.
Record retirement clears nodes before freeing its raw hit-actor vector. Full
clear pops records in reverse and retains process vector capacity. A World
failure guard clears remaining retained frames before World/Scene destruction;
normal00698F80 still invokes its source-ordered clear later in the finish policy.
Startup close has a final process cleanup guard while engine owners are alive.
No second per-World blast list was added.

## Advancement and hit dispatch

006DE780 empty-head at ENTRY returnsfalse. Otherwise signed elapsed*FLOAT .2
advances10. A threshold above progress returns true; eligible nodes run and
retire. Consuming the final node returns true; the NEXT tick removes its record.
006DF390 visits original records in reverse and removes false results by
swap-LAST, including when callbacks append/reallocate the process vector.

006DDD00 resolves direct frameEC actor, otherwise model100(kind9) actor.
Human same-sector24 can bypass occlusion using SAME inline DD0 sector.
Otherwise006DE560 visits actual 'a'/1 skeletal214 indices24,3,14,10,17,21, stopping
on the first clear ray; later markers are not read after success. Rays shorten
by normalized direction*FLOAT .05. NonHuman rays start at centre+node offset and
point back to centre. Native final argumentsFFFFFFFF/B10000 are dynamic-layer
and static-flag MASKS; B10000 is not a pointer. Existing source collision ray uses
the SAME actual tree, buckets and resources; no copied/fake geometry.

Eligible power is abs((1-node14/range18)*power14), with native normalized offset.
006DDD40 ignores nonpositive power, deduplicates actual actor pointers, bypasses
material absorption for kind4, otherwise reads virtualD8 and subtracts unsigned
material row28 only when strictly below power. Authority bit1 gates virtualD4;
source word0 selects hit kind4, other source selects2. Actor pointers append even
when material/authority gates did not fire. Actual frameEC+14 Physics receives
truncated remaining power, flag1 for first eleven eligible hits and2 thereafter;
actor deduplication does not suppress Physics impulse. Counts reset per advance.

**Required positive runtime leaves:** actual actor virtualD8 material selection,
virtualD4 damage and0043C6A0 Physics-force receiver. The actual World bridge rejects
these when reached; it does not silently consume a hit. Generic source policies
exercise positive dispatch with explicit endpoint doubles. Existing script/weapon/
projectile producer guards remain until their required native positive consumers
are integrated. Save restore006DF690/006DECE0 is not implemented or credited.
Malformed/nonfinite centre/power/range, nonpositive range and invalid integer
conversions reject rather than reproduce original unsafe behavior. Original
allocator/SEH/object/register ABI is not claimed equivalent.

## Dynamic point/radius queries and camera

006FCB60 radius queries use the SAME actual grid/registered descriptors, layer8
mask and horizontal radius test, deduplicating pointer identity. Bounds use
original extended reciprocal/step expansion and truncation, then clamp begin
below0/end above count.006FB920 uses point cells[-1,+2), first qualifying object;
40 transforms through actual descriptor inverse,41 uses world bounds,42/44 use
sphere distance; other tags genuinely skip. Box lower bounds are STRICT and
upper bounds INCLUSIVE. Unbound point queries require a real grid; the actual
bridge requires loaded session/tree before generation/update.

0046CB20 command1 now uses SAME camera240/world matrix/state284. Presentation
bit2 gates first; range is an unsigned DWORD converted to stored float, amplified
by FLOAT2.5 when2B4 is nonzero. Native distance comparison is strict; a nearby
blast sets284=2000. Existing follow-pose code consumes this actual shake timer.
This extends an existing partial entry, no duplicate function credit.

## Evidence and verification

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored durable work/mission-world-20261103 retains read-only exports:
shared-trajectory-query-contracts/006ddd00,006ddd40,006ddfa0,006de560,006fcb60,
006fb920; prior session-finish-required/006df390, frame-shared-records-and-ticks/
006ddfc0,006ddf60 and session-finish-shared-resources/006ddee0,006de780. Historic
export names retain the earlier trajectory interpretation. Constructor decompiler
has numerous unreachable-block/stack errors; shared-trajectory-constructor.asm
and shared-blast-*.asm resolve actual stack cleanup, complete node construction,
tail insertion, signed timing, mask arguments, query arithmetic and camera1.
The frozen inventory/project was not changed; no original code was executed.

Focused native ASan/UBSan/leak fixture passes actual process/query bodies over
retained frames: delayed/initial consumption, anomalous tail order, material and
authority gates, Human same-sector/occlusion, reverse release/failure prefixes,
callback append/reallocation/swap-LAST, fourteen-frame actor dedup and heavy-hit
flags, strict point/radius/transform/mask boundaries, unsigned camera range and
2B4 amplification. Engine bounds/matrix/actor damage/clock endpoints are doubles.
Strict actual full i386 graph constructs loading/activation/World plus the REAL
GameBlastWaves finalizer, and instantiates positive create/update/ordered finish;
no Windows fixture is executed. App link and27 tracker tests pass. Temporary
fixtures/binaries are fingerprinted and removed; logs retained.11 new partials,
no whole-body/desktop promotions. Continue FULL P2.2 into actual frame/module/
visibility/guard integration and any reached required positive receivers.
