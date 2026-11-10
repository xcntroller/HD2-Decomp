# Player identity, Human control and retirement

Evidence is indexed by address in [manifest](evidence/manifest.json); source
observations are separately pinned in [review](evidence/source-review.json).
Reuse [actor identity persistence](../future-dependencies/p2.6-actor-persistence.md)
without equating its saved identities to packet tokens.

## Identity domains

| Identity | Owner/lookup | Representation and lifetime |
|---|---|---|
|Transport peer identity|process0097D070 peer records;0072EC90|record+00 DWORD; lookup scans **named** records under418 lock and returns slot orFFFFFFFF|
|Peer slot|same vector1E0..1E4, strideC4|Positional index;00982104 local slot. DedicatedFFFFFFFF has no local Human. Not a stable transport ID|
|Peer Human borrower|record+3C;0072EC20(slot),00730460(local)|Borrowed actual Human pointer; factory writes it, disconnect/reset clears it. Vector can reallocate; don't cache a record pointer across unlock/mutation|
|Scene2 actor token|module13 positional vector;00475D50/00475DD0|0..31 reserved player slots, other identities>=32; spawn decode reads8-bit Human identity; ordinary actor packets read10 bits|
|Dynamic actor token|module12;004772F0|Reverse lookup compares actor+10; not proof of equality to actor+14 runtime persistence identity/generation|
|HumanStore identity|Human170 / soldier3C|Campaign/profile candidate identity; distinct from peer and Scene2 identity|
|Prepared record identity|soldier04, default1023|Spawn-selection/network field. Do not substitute HumanStore index or squad ordinal|
|Local player roster|PlayersProcess0089AC30 / HumanRoster|Borrowed Human pointers; current2C, selected28, previous30 are roster indices; override58 separate|
|Input record|HumanInput manager0073EC50|Owns history/record lifecycle; callbacks borrow actual Human B0. Must detach before B0/Human retirement|

Peer fields additionally observed:08 name/eligibility,40..60 nine stat words,
64 current side,68 pending/secondary side,6C connection wrapper,74 connection stage,
AC load toggle, A1 administration, A2 retiring marker, A8 timestamp. Named does not
prove connection stage2.00730140 writes64 or68 under418 according to its boolean.
Offset54 also lies inside the nine-word40..60 reset range: its entry/display use
is lifecycle-sensitive, not an additional disjoint field.0072EC20 and00730460 return null on out-of-range slots; they do not manufacture a
Human or retain the borrowed result.

## Actual Human creation and writeback

00612A60's entry ABI matters: ECX is peer slot, EDX is actor identity, followed by
six stack values (position, direction, prepared soldier, stats, clothing, alternate
state); RET18. Entry stores EDX at EBP-2C and ECX at EBP-24. EDX is used for model
name `_PLAYER_%2d` and004759C0 scene registration. ECX is later used as the C4
peer-vector index. They may coincide at an individual call site; they are not one
identity by definition. This fixes the decompiler's apparently uninitialized
`local_28` and lost register parameters.

The shared factory creates/loads kind9 model, links the actual world root, sets
pose/name, updates frame, constructs the actual kind3 actor004759C0, releases the
factory frame reference, then calls0059EDC0. Nonoffline then writes that SAME Human
into peer3C under418, timestamps A8 with timeGetTime, and conditionally registers
with the actual TeamMgr auxiliary owner when authority and006B66B0 require it.
Existing model/factory failure returns null; some later initialization failures
leave registered actors for world cleanup. Do not create an independently owned
network Human.

0059EDC0 uses Human19C kind,1A0 personality,170 HumanStore identity, actual B0,
speech/profiles/Items storage. Nonoffline also applies B0+60 from soldier08 and
side00730140(peer,soldier40,false); soldier09 can select game78. Current source
implements the ordinary offline equipment/profile path but explicitly rejects
nonoffline identity writeback. Real inventory, clothing and item registration are
reused; a copied display-player row is not a full prepared soldier.

## Authority spawn → presentation

0069E340 rejects an already-existing peer-slot actor in nonoffline mode, validates
style-specific spawn/side requirements, then uses actual SpawnPoint00475940(1)
and00611C90. Missing/invalid spawn publishes application0F, not generic factory
success.00611C90 returns immediately without authority bit1. It selects pose,
clothing and identity through actual rule/spawn owners, splits inventory stacks,
creates through00612A60 and stores SpawnPoint+A8 borrower.

If game1C==1 it calls006123A0 directly. Otherwise it publishes module13 spawn-point
packet through00475530, subtype0 in2 bits. The established offline writer/reader
pair is reused with nonoffline tails:

1. player8, Human identity8, world position quantizer and direction;
2. generation8, name NUL, transport identity32, prepared soldier00727480;
3. nonoffline stats: one10-bit word then eleven7-bit words, destination indices
   `0,2,1,3,4,5,6,7,8,9,11,10`.

006125A0 reads that same order; client-only performs actual generation adjustment
00475510(identity,generation-1), binds name/transport to player0072E940, then calls
006123A0. Requires initialized world quantizers, actual actor registry, SpawnPoint
frame and full catalogue/Items/B0/profile/input/presentation services.

006123A0 reuses SpawnPoint+A8 offline, or lookup00475D50(decoded identity) on
nonoffline authority; client-only creates through the SAME00612A60. Thus a listen
host must not create a second presentation Human. Only decoded player==local slot
and SpawnPointA0==0 binds local camera/roster; remote Humans remain remote. B0kind1
and7 have separate side/clothing/flags branches.0069E290 applies local match/HUD
effects; it is not the peer3C pointer writer. No second Players/Camera/HUD per
remote actor exists in this process architecture.

## Disconnect and repeated world lifetime

Application presentation command2 carries transport identity32. Client-only logs
departure then calls0069E720. That receiver reverse-finds peer00 under418, clears
transport/name; if Human3C exists, sets A2, clears game78 when equal slot, detaches
presentation HUD/Players override58/spectator B4 references when they equal that
Human, conditionally calls authority TeamMgr auxiliary virtual24, then00479700
and00475C50(Human,0), clears peer3C and nine stats. It does **not** erase the peer
slot by shifting the vector. Detailed actor retirement leaves remain the existing
registry contract; do not replace them with `delete peerHuman`.

Connection termination callback00722180 FFFF0016 unregisters wrapper334 through
00746D20, deletes its actual virtual00(1), nulls334 and writes application pending
status11. It does not itself delete a borrowed world pointer. Status11/retained
unwrap belongs to the P2.5 controller lifetime contract. World entry0072E890
clears all peer Human borrowers before a new session; input record reset and
selected FIFO draining follow, as [entry order](shared-world.md) specifies.

Still conditional/unresolved: complete B0 runtime packet effects/input-history
timeline, nested late-join actor state and spectator transfer.
The reached0069D780/0069D9C0 existing-Human and queued allocation boundary is now
mapped in [initial sync](initial-sync.md), including limit-return cleanup asymmetry;
its queued completion and0059F050 remain conditional follow-ups. These are named required receivers
before those paths are admitted, not evidence of a campaign reinforcement policy.
Phase2 can keep their honest unavailability without blocking the bounded entry
contract. Phase3 must test join/disconnect against real authority and transport.
