# Packet-to-world synchronization contract

This is the original client-side architecture, not a complete multiplayer wire
implementation. Reuse the actual process queues/broadcasts/input manager from
[revisioned source binding](evidence/source-review.json), at revision c09c22c
as recorded in [review](evidence/source-review.json). Function/evidence addresses
below refer to the fingerprint in [manifest](evidence/manifest.json).

## Three payload layers

1. Transport/connection queues copy bytes and carry channel/queue flags.
2. Process0072C3B0 selects input histories versus the ordinary5-bit stream grammar.
3. Ordinary grammar selects module or application envelope20. Module IDs select
   actual registry virtuals; actor packets then select actual actor/state virtuals.

A single generic packet handler cannot replace these layers. Payload lengths are
not universally self-describing after the module tag. Ignoring an unknown actor
payload can misalign every later message in that same packet.

## Construction, flushing and ownership

Process0097D070 owns208/244/280 broadcast streams (stride3C), request2BC and targeted
scratch2F8.0072BEF0(target,module,local,priority) clamps priority>2 to2; if !local
and authority, chooses request2BC; otherwise target0 chooses208+priority*3C and
nonzero chooses2F8. It records priority20, active1C, target24, inverse-local18,
module28 and writes5-bit module tag. Priority is not a reliability declaration.

0072BF90 clears active1C. Broadcast target0 remains pending. Targeted adds module
terminator19 in5 bits, maps **external transport identity** through0072EC90 to a
peer slot, then local slot→0072C230 or remote→0072D0E0(slot,bytes,ceil(bits/8),0x42).
Cursor resets after submission; ignored send returns do not prove success.

0072C5D0 flushes at process period5054 (initial50) using accumulator5058, one
subtraction and original large-overshoot reset. Adds terminator19 once to each
nonempty broadcast. Nonoffline authority selects nonlocal peers with transport00!=0; client selects
slot0.0072D0E0 separately gates actual connection stage74==2. Authority+presentation loopback reaches0072C230. Request2BC instead reaches
**0072C170 authority FIFO**, then resets; scratch resets separately. Actual input
manager14 virtual14 serializes histories; send flags20 then22 are separate from
ordinary42. Do not turn request production into an immediate authority call.

0072C170/0072C230 copy allocator-owned payload before entering process lock430,
record timestamp007E814F, then append an18-byte linked FIFO node under lock.
Authority FIFO1F0/count1F8 and presentation1FC/count204 are process-lifetime owners.
No caller buffer is retained. World borrowers bind only after actual module owners
exist and unbind before destruction; queue lifetime survives world exit.

0072C360 callback ABI: ECX ignored context, **EDX bytes**, stack+04 length,+08
logical channel,+0C queue flags; RET0C. It combines `(channel<<5)|flags`, resets
ECX to actual process0097D070, and chooses C170 if authority bit1, otherwise C230.
Do not use the decompiler's missing/context parameters as a C++ signature.
DirectPlay callback00722180 FFFF0011 separately copies receive bytes into C230
with flags80; FFFF0013 frees the return-buffer pointer. Neither bypasses queues.

## Drain and dispatch

0072C3B0(authorityPhase=true) requires bit1; optional domain174==1 service pump
precedes authority drain. Otherwise presentation FIFO is chosen. Under430, pop and
unlink current node, extract metadata, then unlock **before** any decoder/world
call. Flag20 selects actual input manager virtual18 with phase=true and
`(flags>>1)&1`; requires actual game/input/payload/length. Ordinary packets consume
5-bit tags until19 or false. Tag20 reads32-bit application command:

| Process phase | Application receiver | Module dispatcher | Actual module slot |
|---|---|---|---|
|Authority|00727800|0048DC50|virtual28|
|Presentation|00728450|0048DC70|virtual24|

Nonapplication packets require current game and loaded14. False stops **this
packet**, whose owned bytes are then freed007496C0; it does not imply the following
queued packets are retired. No universal unknown-message skip is established.
Native explicit failures may throw; payload/unlock handling must still preserve
the original ownership boundary, without reporting success.

Application producers0072A0B0 and0072B1C0 enqueue under their real process lists
when their final drain flag is false in nonoffline mode. True drains, constructs
envelope20/32-bit command, serializes per-command payload, finalizes and retires
owned parameters/nodes. A0B0 flushes pending streams with1000 before each message.
They are not direct world receivers. Their many unrelated admin/chat branches are
not prerequisites for bounded game entry.

## Concrete module dispatch

Final registry is19 slots0..18;18 may be null. Packet-relevant final vtables:

| Kind | Vtable | Authority28 | Presentation24 | Boundary |
|---|---|---|---|---|
|4 Script|0080F750|00405B30 false|00424590|read8 opcode; ECX opcode/EDX packet into0042B650. Only5F visibility source currently exists|
|7 Versions|0080F908|00405B30 false|0043F1E0|Conditional model-part/selection publication; actual body remains required when reached|
|12 TempActors|008100E8|004788A0|004788E0|Dynamic token lookup/physical creation below|
|13 Scene2 Actors|008100A8|004755D0|00475630|Positional actor slot then actor virtual20/1C; reserved wrapper below|
|14 ShotMgr|0080FA18|0044DC00 false|00452BD0|Presentation cosmetic branch, not authority damage|
|16 HeavyFireSupport|00810010|00405B30 false|00472590|Conditional actual fire-support receiver, not a mandatory cold route|
|Others|see catalog|default false|default false|00405B20/30 are `xor al,al;ret4`; no payload. Not successful no-ops|

004788A0 reads10-bit token, reverse-lookup004772F0 against actor10, dispatches
actor20, false if absent.004788E0 reads **2-bit** selector (x87 instruction check,
constant2 and added1). Selector0 reads10-bit token and actor1C if present; missing
returns true without a general payload discard. Selector1 reads another2-bit
subkind;0 reads three signed16 values, quantized position/direction and calls
actual00478D30. No generic factory/deferred-reference protocol is proven.

004755D0 reads10-bit Scene2 slot.>31 immediately dispatches actual actor20 with
unchecked registry indexing. Reserved0..31 uses005A5F80: only nonnull Human with
nonnull B0 **kind1** dispatches20, otherwise00541CB0 consumes the authority
grammar. Presentation00475630 uses actor1C or005A5FB0: B0kind1 **or7** admitted,
otherwise005418E0 presentation discard. Null is not a zero-length message.

Human vtable008111E0 slot1C=005A5F60 →Human+B0 virtualC0;
slot20=005A5F70 →B0 virtualC4. Both are direct receiver-adjusting tail dispatches.
B0kind1 vtable00811848: C0=005406C0 (presentation apply), C4=00540DB0
(authority request validation/queue). B0kind7 vtable008112D8: C0=00592FE0,
C4=00583220 (actual empty body). Other B0 kinds have different targets and are
not admitted by the reserved wrappers. Do not promote an empty kind7 authority
body into acceptance of arbitrary reserved-player payloads.

## Human command grammar and authority publication

Commands are5 bits, terminator24. Verified helper dispatch:

| Command | Payload reader | Presentation/discard | Authority/discard |
|---|---|---|---|
|0|005B10C0|yes|yes|
|1|005B1510|yes|authority kind1 ignores the tag; authority discard consumes it|
|2/3/4|005B1980/005B1180/005B1000|yes|yes|
|6/7/8/9|005B0F70/005B1A10/005B1BB0/005B1E40|yes|yes|
|10/11/12|005B1F10/005B1D10/005B1290|yes|yes|
|13/14/15/16|005B2340/005B2100/005B0EE0/005B0E50|yes|13/14 are not kind1 authority actions; authority discard does consume them|
|17|005B1800|not presentation grammar|yes|
|18/19/20|005B21D0/005B22B0/005B2A90|yes (kind1 lacks18)|not authority grammar|
|21|005B2F80|not presentation grammar|yes|
|22/23|005B2FF0/005B3060|yes|yes|

The discard receivers differ: presentation005418E0 lacks17/21; authority00541CB0
lacks18/19/20. Both have holes5 and return on24. This table describes actual
switch coverage, not permission to send undefined/hole tags. Exact payload bit
fields remain with their named helper bodies; full live action compatibility is
not claimed by this architectural checkpoint.

005406C0 applies state/actions through existing actual Human receivers:1→005A15D0,
2→005A6A00,6→actor lookup/007861A0,7/8/9→005A2E10/005A2F50/005A3A40,
10→005A3750,11→005A38F0,12→0053D870,14→B0 virtual10. It consumes0 but does not
reapply a generic input record.00540DB0 allocates typed command objects, invokes
their actual virtual00 payload reader, validates through actual clothing/item/
state receivers, and conditionally queues accepted records through00547600 in the
SAME B0 queue. Command0 allocates208 bytes; only nonlocal Human and005AC540 admits
two40-DWORD copies into B0 and insertion00547460; otherwise frees. Command17 calls
Human virtualD4 with decoded actor reference;21 synthesizes command20 in its queue.
Some branches queue even in offline mode; do not replace all with one predicate.
Accepted nonoffline authority actions refresh peerA8 via actual Human actor10.
00592FE0 has its own kind7 application/animation/death behavior. These differences
explain why authority requests cannot be replayed as presentation commands.

00452BD0 reads1-bit subtype;0 reads quantized position and8-bit value, takes exactly
one `rand()%3` texture draw, then00451A80(...,upVector,texture,value,30). This is a
presentation effect. Other subtype has no such effect. Preserve actual PRNG draw
order if implemented; do not turn it into authoritative hit/damage.

## Initial synchronization and later state

Presentation command0 loads shared world then enqueues request9. Request writer
publishes local slot5 and **only** count16 of nonnull Scene2 identities>=32.
Authority00727800 command9 sign-extends that16 count, compares its own same-range
count, then enqueues A0B0 command0x15 to external peer identity with equality flag.
This is a count comparison, **not identity/hash/content agreement**.
The [initial-sync contract](initial-sync.md) maps the later ordered module38/3C
pass, dynamic outer records and existing-Human respawn. Authority publishes peer
stage74=2/process readiness before final send submission; ignored send returns
do not prove synchronization. These interfaces differ from persistence30/34.

Presentation0x15 false requests status11 on current state; true reads full peer
records and optional Human pose/prepared data, client-only clears/rebuilds roster
and creates actual Humans, then0048DB60,00699220,match0069AA50(2), retained
menu selector-2 and conditional status11. This is a second synchronization stage;
do not expose control merely because world initialize returned true. The outer0x15 and module12 formats are now mapped; nested actual actor50/54,
writer pairing for Physics/Script/Versions and input/runtime readiness remain
conditional follow-up contracts. Ordinary module13 spawn above is separately paired.
Application0x14 uses004782C0/00478520 dynamic state; application7 additionally feeds
0069C390 snapshot. Reuse P2.6 formats where actually shared, not a new serializer.

## Send classes and Phase3 boundary

0072D0E0 sends through actual connection334 on client; authority selects peer6C
under418 with stage74==2. It splits flags into `logicalChannel=flags>>5` and
`queueIndex=flags&3`.00745EA0 copies payload and enqueues a retained record under
connection20, into queue14+4*index.00745D20 constructs queue0/1/2 with final vtables
008165F0/00816614/00816638. Ordinary42 therefore uses channel2,queue2; input20/22
uses channel1,queue0/2. These are **custom queue choices**, not DirectPlay flag names.

Queue0 enqueue00745130 stamps monotonic record14 and appends. Queue1/2 enqueue
00745520 additionally retains timed pending records.007456D0 retires acknowledged
sequence<=28, schedules at most7 retries per update, doubles retry interval capped
5000.007457C0 processes completion only for next contiguous sequence24+1.
Queue2 overrides take00745920: it releases only next contiguous sequence48+1,
clears record flag4 and returns it. This establishes resend/ordered bookkeeping;
the complete ACK/socket/wrap/loss protocol is not reconstructed here. In
particular queue0's lack of this resend list is not proof of all transport behavior.

Worker00746B50 registers a **borrowed** connection pointer underworkerB4;00746D20
swap-last unregisters without deleting. Disconnect must unregister before its
virtual deleting destructor. Packet/process/world owners are separate lifetimes.
Real worker scheduling, sockets, ACK/wrap behavior and transport success stay
Phase3. No captured/native packet fixture can establish a live connection.
