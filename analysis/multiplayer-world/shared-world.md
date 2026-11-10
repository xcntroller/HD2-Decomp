# Shared game owner and entry contract

The original has one game-state constructor00698510, one initialization body
006989B0 and the same world/module owners. Status7/8's connection state0068A710
is a different temporary owner. It must not be mistaken for a multiplayer world
subclass. Reuse [world ownership](../world-scene-ownership.md),
[activation](../session-activation-loading.md) and the source review linked below.
New evidence is indexed by original address in [manifest](evidence/manifest.json).

## Controller entry:006C20F0 status9

1. Configuration008AE920+04 must be nonzero. It means client1/host2 submission,
   separate from process flags00982100. If current state is retained menu type11,
   restore its game5B8 as application current **before** menu shutdown08/delete00.
2. Shutdown08/delete00 the old current state. For nonoffline `(flags&7)!=7`,
   perform00730A40 →0072E890 →00731610(true), in that order.
3. Allocate0xEC aligned16, construct00698510, publish application+14. There is no
   safe allocation-null rollback in the original caller.
4.006B8700 imports selected multiplayer map metadata into application GameData's
   inline mission at+14.006B9190 returns that descriptor when configuration+04!=0
   (unless the separate standalone-selection discriminator takes precedence).
   Named C4 player records supply resource/display names through006CA5D0, side64
   and soldier54. This is not campaign candidate eligibility.
5. Invoke the SAME game virtual04=006989B0 with that descriptor. Failure invokes
   game shutdown08/delete00 and builds Main type2. Success saves game64 in app20;
   nondedicated configuration170==0 sets camera command2(1,1), then opens retained
   player/menu selector-2 through0069E620. Dedicated omits that presentation route.

The three preprocessors have concrete mutations:

| Receiver | Original effect | Required live owner |
|---|---|---|
|00730A40|Authority: under418 lock, named stage74==2 peers toggle AC and copy it to peer wrapper08. Client: toggle local-slot AC and copy to connection334+08.|Actual peer vector/local slot/connection wrappers; no new roster|
|0072E890|Under418 clear every peer3C Human borrower and nine words40..60; reset actual input manager0073F5A0 when present; clear three process statistic blocks.|Existing process peers/input owner|
|00731610(true)|Reset each input record twice using0073E660(1,0,1)/(1,1,1), then free the selected FIFO: authority1F0 if bit1, otherwise presentation1FC, under430.|Actual input record collection and copied packet payloads; it does not clear both queues|

## Packet-triggered entry:application command0

0072C0B0 decodes envelope20 and dispatches command0 to00728450 on presentation.
This is a second real entry path, not a status9 alias:

1.00724110 sets process5044=1 and rebuilds actual HumanInput manager through
   00723000/00723220 before old-world retirement. Optional profile-gated service
   remains conditional; see [input lifetime and sync](initial-sync.md). Read32
   into application1C and read32 into0097D4B8.
2.006AD780 reads received rules/configuration;006AD5B0 destroys the previous map
   vector, reads a single selected map and sets selection290=0.
3. Shutdown08/delete00 old application current; allocate/construct/publish SAME
   game00698510. Import006B8700. Read display-player count8, each name capped50,
   side2, soldier5, then006CA5D0.
4. Initialize virtual04 with006B9190. On success construct00725A40 peer record,
   append00726F30 under0072EC70/0072ED30, then **enqueue** request9 through
   0072B1C0(9,0,0,false). This asks authority to compare Scene2 population.
5. Failure deletes the newly initialized game and constructs Main. Unlike status9,
   this branch has no explicit second shutdown08 before that deletion. Preserve
   this distinction; no atomic restoration of old config/state is established.

00728450's ECX is the application receiver; its switch argument and packet are
stack values. Decompiler labels and apparent extra parameters on00698500 are not
usable prototypes.0072C360 callback independently uses EDX payload and three
stack values; see [packet ABI](packet-synchronization.md).

## Selected-map and mission metadata

Configuration284..288 owns selected-map pointers;290 indexes them.006AD580
dereferences selection without validating bounds.006AD590 serializes that selected
record via006AD820.006AD5B0/006AD990 reads its pair. This packed0x48 descriptor has
unaligned string members+1/+5; it is **not** the ordinary GameData node ABI.

Wire order (bit-packed, no chunk envelope):

| Order | Writer/reader | Representation |
|---|---|---|
|Config|006AD6E0/006AD780|domain174:8; style224:8;22 fields228..27C each16; globals9820D8,9820D4,9820DC/1000 each16. Reader sign-extends16 and multiplies final seconds by1000; domain/style zero-extend.416bits|
|Map strings|006AD820/006AD990|name+1 NUL, resource+5 NUL; reader cap4096 per string|
|Scalars|same|C,10,14,18,1C each8;20 signed16; string44 NUL|
|Objectives|006ADB70/006ADBD0|count8, then each24-byte memory row in wire order+04:16,+00:16,+08:16,+0C:8,+10:2,+14:16 =74bits. Reader sign-extends the16-bit fields|
|Table|006ADC30/006ADC70|count8, then each12-byte memory row+00:16 signed on read,+04:8,+08:8 =32bits|

Neither count8 writer proves a safe maximum in its vector owner; do not silently
truncate oversized source collections. A native bounds rejection would be an
explicit safety deviation. Readers mutate strings/collections incrementally;
no checksum, transaction or general short-read recovery is established here.

006B8700 clears GameData inline+14 through virtual28, writes caption2002 at1C,
copies selected name into GameData80 and resource into38 (instruction receivers
006B8745/006B8753; resource is mission-relative24). For each objective row it
allocates0x30, final vtable00815AE8, **kind12**, with destination/source fields
1C←04,20←00,24←08,28←0C,18←10,2C←14; appends through inline virtual3C. Copies
three-word table rows into GameData8C. Maps GameData64/68/70/6C/74/78 from
descriptor C/10/14/18/1C/20, retains new string44 before releasing old GameData7C,
then inline virtual24 finalization. Success returns1 despite unchecked allocations.
GameA8 borrows the application-owned inline mission; temporary packet/map strings
must not become dangling mission storage. Full selected-map metadata is absent
from current front-end catalogue source.

## Common loading, update, retirement

006989B0 calls006CA780 and uses the same Scene/Scene2, quantizers, Cars, actor
factories, Items, module registration and live services. Authority activation
0048DB00(true) is an ascending pass when bit1; HUD lies between passes;
presentation0048DB00(false) and FPV require bit2. Offline7 alone spawns application
live squad through00611C90. Nonoffline sets match88 (authority3, client0), inspects
mission objectives and publishes world-state command00699290(0x14) from authority. Null-descriptor
loading obtains selected MP resource via006AD580+5; ordinary nonnull loading uses
mission resource24. Do not run a parallel scene or copy registry.

Common fixed0069AF10, variable0069B230 and render0069B870 contain actual mode gates;
see [matrix](authority-modes.md). Their current source rejects unrecovered
nonoffline tails. Same C++ owner types are reusable, not already integrated.

00698F80 offline snapshots Humans into application live squad. Nonoffline instead
retires that squad from its tail. Then FPV close, lights, reverse presentation
module20, dubbing close, reverse authority20, HUD and shared transient/collision/
material/face cleanup follow the existing contract. File unload18 and final
destruction are distinct later stages; their module passes are not interchangeable.
Unbind process→world packet borrowers before world destruction. Process queues
outlive worlds; a missing loaded world returns false, never a successful decoder.
Connection wrappers have an independent process/worker lifetime.

## Actual source gap and scope

[Source review](evidence/source-review.json) pins c09c22c. `SessionOwner` and
`copy_mission_request` require a nonempty prepared squad; `SessionSelection` borrows
the campaign route; `GameDataNode` lacks kind12. These guards cannot simply be
removed. Add the original mode-specific metadata/request contract while retaining
the same game/world owners and their validation. Controller9 is currently an
unavailable throw; connection host/join/game-entry callbacks are explicitly
unavailable. Phase2 can complete this source receiver with truthful prerequisites;
actual ready transport and synchronized sessions are Phase3 acceptance.
