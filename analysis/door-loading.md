# Door loading and first update — source work, 2026-11-09

P2.2 remains open. GameApplicationState still declines loading. This source
connects required Door6 activation/deactivation, local packets and ordinary
rotating first-update receivers; it is not an integrated or desktop-tested game.
Continue the full checkbox after this subsystem checkpoint.

## Original required boundary

Door vtable00812FF8:3C→00600CC0,40→00601350,14→00600940→005FFFE0,
18→00600920 (skip when mode&2, otherwise005FFFE0),1C→00601F90.
Tutorial's captured Scene2 has25 Door property records, all rotating220==0;
two have initial224==1. Those two **emit a local presentation packet** during
activation. Registering only their colliders would omit required first-frame
state and movement. The temporary audit consumes the actual captured records.

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Canonical inventory fingerprint is recorded in the checkpoint receipt; no
original executable/DLL or Windows fixture was run.

## Activation, portal and descriptor lifetime

00600CC0 calls common activation first. FlagA0false restores the saved parent
with preserve-world1. First local activation clears portal FC/100. Presentation
mode2/false/A0 refreshes the actual portal binding, reparents to scene200,
updates and saves world position/local quaternion; state0 opens both portals.
Later local work runs once: resolve named linked actor through scene+7C/C0001,
publish actual localization114, initialize voice/timer/flags, find handles, save
pose and snapshot224/234. Authority initial224==1 invokes005FF4B0(2,1,0,0).
Flag90 is set and9C cleared after that call, as native.

00600950 searches short names case-insensitively for substrings `dhandle` and
`d2handle`. Reverse children are tested before recursively visiting their
subtrees; the current frame is tested last. No engine references are retained
for these actor/model-owned borrowers. Oversized32-byte native names/hierarchy
cycles beyond the bounded depth reject rather than overflowing native buffers.

Descriptor130 uses flags40, optional flag190's100000, mask1000, actual model/
actor, aggregate local bounds, a **live frame world-matrix pointer**, owned
inverse, transformed bound centre and geometry radius times largest world
scale. Insert only; no invented move after initial registration. Invalid X
bounds reject an unwritten native centre. The emission value takes actual
world position, group1/flags1 and null relative/source. Constructor005FF210
initializes238 to0; historical serialized bytes do not populate that timer.

006007B0 invokes portal virtual00(closed) on BOTH FC and100; this is not frame
Release or SetOn.00601350 closes both, clears linked borrower/90, removes the
descriptor, restores224 snapshot, clears94, then common deactivation. It does
not restore234. Source destructor/failure guards detach published descriptors
before destruction; retry after partial activation requires actual retirement.

## Initial state and local packet receivers

005FF4B0 implements the reached linked-Door/notification/state producer policy.
Initialized linked Door propagation precedes this Door's actual context Use
notification. Command2 honors234 unless forced; locked emits2, ordinary opening
emits0 and sets224=2/10C/portals; closing3 emits1 and sets224=3. Auto-close
counts use238+15000. Offline mission ordinals>12 require the unrecovered script
value protection guard and reject before mutating state; no guessed comparison
or bypass is installed. Forced calls preserve the original guard bypass.

00601F40 writes actual shared module13(5bits), physical Scene2 identity(10bits),
then command(3bits, ceil(log2(7))). It leaves the stream pending; the existing
SessionLocalPackets flush/FIFO/decoder consumes it. It does not directly change
a model's presentation or manufacture a successful transport receiver.

00601F90 receives0..5, with6/7's native default. It emits actual defined sound
IDs, acquires/resumes the loop voice, updates224/10C/10D and portal state;
unlock3 also clears the linked Door lock. The source emission maps the real
stored group/flags/scalars into SpatialSoundRequest. Nonempty raw relative/
source/filename fields require actual typed producers and reject. StartupSession
now exposes actual voice frame enable/cursor receivers through shared SOUND;
native handle lookup failures reject their undefined dereference branch.
This receiver is **absent from the frozen inventory** and gets no new credit.

## Ordinary rotating first update and its actual query

005FFFE0 advances handle roll using signed elapsed*.005 and strict crossings
at0/pi/4, then the actual frame+1C(direction, **FLOAT roll**) and update. Cached
handle normalization preserves native tiny-vector component replacement and
FLOAT/extended stores. Auto-close checks the actual current player's distance
and unsigned scene/frame visibility stamps before deferring2000ms or closing.

Rotating state2/3 advances angle264 using unsigned elapsed*.001*speed22C,
strictly clamps to228/zero and controls the actual loop voice.005FF700 composes
axis-angle with saved local quaternion using native FLOAT cross/scaled-vector/
sum/scalar intermediates and extended dot, then actual engine Normalize/flags/
update. Opening2 and exact +/-X axes skip obstruction probes. Other states
exclude the SAME descriptor, transform the expanded local box's eight corners,
probe the12 original edges plus1→6/2→5, clear exclusion and restore the prior
quaternion on collision. Blocked update restores the previous angle, waits200ms
and saves/pauses the loop cursor; the next unobstructed move restores/resumes it.

006007F0 refreshes actual world bounds/inverse, mask1002 for rotating/opening
non-X axes (otherwise1000), sphere centre and shared bucket movement. Descriptor
world-matrix pointer remains live. Sliding220!=0 still rejects its missing
005FFBD0 receiver. This exclusion is explicit even though all25 captured first
mission records select rotating behavior.

DoorCollisionQueries supplies006F78F0/006F3250's **dynamic-only** boolean path.
It has no static traversal, camera vertical dispatch or length+radius reach test.
Native ranges expand one cell on both ends, preserve Z/X/bucket order and full
initial XZ bounds, filter descriptor/layer flags and use the actual line/sphere
broad phase. Native denominator, projection, length and scalar stores differ
from the camera path. Primitive40/41 reuse the recovered slab/transform bodies;
42/44 preserve the Door query's inside-start (including zero-length) hit and
strict extended best-parameter comparisons. Known49/4A reject missing receivers;
unknown tags follow native skip. Raw output normal/W/pointer ABI is excluded.

## Evidence, verification and continuation

Ignored WORK=`analysis/work/mission-world-20261103`: door-loading-required/,
door-loading-packet/, door-loading-receiver/, door-loading-first-update/,
door-motion-required/, door-motion-math-ray/, door-ray-query-required/;
door-actor-activation.asm, door-frame-find.asm, door-command-packet.asm,
door-opening-quaternion.asm, door-handle-update.asm, door-dynamic-ray-query.asm,
door-vector-length.asm and door-tutorial-properties.json. These exports are
static research, not credits for every referenced/helper body.

[Checkpoint receipt](checkpoints/p2.2-door-loading-20261109.json) records native
ASan/UBSan/LSan against actual source Door/common actor/grid/bitstream/FIFO/
query lifetimes and captured property records, plus strict actual-host i386
Door/world composition and application compile/link. Engine geometry/frames,
script/localization/audio/lock endpoints are native boundary doubles. No actual
world rendering, engine execution or desktop acceptance is claimed.

Continue required other derived activation, actual containing Human/session/
FPV/HUD/SOUND/VM owners and first-frame orchestration. Known remaining Door
limits: sliding, campaign protection guard, dynamic primitives49/4A and full
interaction/save/update ABI. No successful placeholder receiver supplies them.
