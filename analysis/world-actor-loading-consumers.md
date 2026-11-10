# World actor consumers — source checkpoint 2026-11-10

P2.2 remains open. This supplies the common containing-world consumer bridge and
its actual initial loading/retirement endpoints. It does not construct an
application world, enable loading or establish a first rendered frame. Car
callback completion and the containing session/ordered loader still remain.

## Shared owners and concrete consumers

`WorldActorLoadingConsumers` replaces the formerly declared common Consumers
methods in the strict actual WorldActors composition. It borrows existing
Startup mode/local identity/configuration, parsed WorldScene/EC registry,
Human owner factory, Players, session state, checked Script link, SOUND, HUD,
proximity, Car resources/tables and packet owners. It creates no parallel state,
successful empty service, transport or loaded flag.

SceneActorLoadingRuntime now composes with that same consumer. Switch/Door Use
resolves the actual context and invokes0041D960; the native state/command
argument is unused. Sound emission for Switch, Door, Supply, Animation and
Fowl goes through the SAME `WorldSoundLoading`/Startup SOUND manager. Listener
registration/removal, Door pause/resume/cursor and actor voice-stop use the
existing real endpoints. Car resources and current-player attachment are actual
borrowers. `car_game_kind()` reads session status04, distinct from configuration
04, request modes and packet kind1C. Fowl cull distance reads parsed gameC4.
Dynamic item lifetime policy is006B66B0, the existing application configuration
predicate; its RandFloat is the engine stream, separate from CRT. Nonoffline
sleep/network publication still explicitly rejects.

*005B7570* has an actual body: entry90 exists, signed cooldown6A4<50, signed
definition38>0, actual world position, default004374D0 SOUND request with
flags1/group5/entry38. Ignore emission return and only then publish cooldown2000.
Missing/nonpositive definitions and cooldown>=50 return without world/SOUND
calls. This does not reconstruct later loose-item simulation or network events.

## Retirement and the original reverse order

*00476860* clears byteA2 in the SAME control1DC records under critical section
418 (source lock0). *0059E3E0* clears DWORD3C under that same lock for its reserved
Human identity. Startup exposes these actual typed endpoints and lends the same
record state to PreparedSpawnPackets, whose accesses acquire lock0. Bounds
reject native invalid indices; exceptions release the lock. No record copy or
separate player array substitutes for the actual owner.

Original00698F80 calls presentation reverse module finish, dubbing close,
authority reverse module finish, THEN HUD close. Source reserved Humans can
therefore die before Script4 contexts exit. Derived ActorMissionScriptContext
now validates its borrowed actor10C against the retained frame's current EC
word0 before dereferencing it. Base-frame operations can retain their frame;
actor-dependent requests after retirement reject instead of using freed memory.
The old blanket instruction to finish/unlink VM before the original reverse
module pass cannot substitute for this actual order. Final aggregate destruction
still retires VM before deleting remaining scene/engine owners. Positive Exit
commands requiring already-retired Human state remain a required receiver, not
a reordered successful execution claim.

*0049EF70* now has its concrete initial HUD receiver: nonzero flags AND mode2,
kind1 clears matching6B8FC then6B8F8, kind2 leaves that pair, unknown kinds
return. Main roll2F00+88 matching invokes required target/reset consumers.
The final borrowed actor is **HUD10B3C = command10B20+1C**, not Signals60;
assembly proves Signals is2E80. Command bind004B4980 initializes1C=0; it now
has a typed actor borrower and matching clear. Main roll bind004B0750 sets88=0.
No populated roll/menu producer is offered; positive004B0920/004C11C0 rebuild
requests explicitly reject. Signals' existing scalar60/64/68 cold writes are
unchanged. There is no successful menu cleanup placeholder.

## Compass and actual spawn interfaces

Actual spawn→Players→HUD instantiation found a concrete mismatch in0049F1C0:
**005A78F0(9,nullptr) searches inventory DEFINITION9**, not a player action.
`hud_compass_policy.h` now calls the existing actual Human storage/Items lookup.
It keeps all original mode7/flags1A0/inventory visibility/crosshair14/command
visibility/current-Human gates, then calls the same compass owner with the result.
Definition9 is separate from runtime item identity. No new lookup credit.

OfflineSpawnPresentation now borrows Startup's actual signed local identity;
the native DWORD comparison preserves negative bit patterns. PreparedSpawnPackets
borrows the SAME SessionActivationState and rereads game1C for direct/broadcast
selection.00698510 initializes1C=0, now published in that scalar bank; status04
remains separate. Both APIs require lvalue owners, not temporary copied words.
Strict actual host composition now constructs the real offline presentation,
PreparedSpawnPackets and OfflineSpawnPacketDecoder, including actual camera
refresh, borrowing actual record state/locks, three broadcast streams, same A8 Human and roster.
The former declared Spawn decoder endpoint is removed from the fixture.

## Evidence and targeted verification

[Receipt](checkpoints/p2.2-world-actor-consumers-20261110.json) preserves hashes.
Original SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
WORK original exports: actor-cold-lifetime/0059E3E0,
scene-actor-pending-removal/00476860,
item-spawn-loading-boundary/0049EF70 and005B69B0,
dynamic-item-contact-dependencies/005B7570,
hud-containing-post-bind/0049F1C0,
soldier-bind-first-frame-required/005A78F0,
hud-loading-panels-lifetime/004B0750/004BF2E0/004B4980.
New HUD target/callee exports and exact child-offset assembly are retained.
Original00698510 and00698F80 were reused from existing owner research.

Focused native ASan/UBSan checks cover actual locked records/preservation/error
unlock, real Scene2 registry/context retirement and foreign rebinding, consumer
mode/config/context/proximity/SOUND/HUD identity routes, contact sound boundaries
and failed emission/retry, actual definition9/compass gates, signed local identity,
camera-before-roster and live game1C direct/broadcast with recursive locks.
Frame/SOUND/packet/actor boundaries are explicit doubles. Actual full typed host
router/WorldActors/VM/SceneActorRuntime/common Consumers and offline spawn chain
compile strictly for i386; Car factory and remaining containing aggregate/HUD
world data are still borrowed. Application links;27 tracker checks pass. No
Windows fixture or original EXE/DLL/Proton was run; no desktop verification.

One newly credited frozen partial:005B7570. Existing HUD/context/spawn coverage
is corrected/extended once, with no double credit or complete promotions. Current
totals34 complete/1,944 partial (1,939 client/five engine)/18,783 frozen.
