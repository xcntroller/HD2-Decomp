# P22-PACK implementation — 2026-11-11

**PACK is source-closed as of2026-11-13.** [Final integration/limits](p22-pack-source-closure.md),
[receipt](checkpoints/p22-pack-source-20261113.json), [plan](p22-pack-plan.md).
The detailed text below records the earlier wire/notification checkpoints and
historical blockers. Human source through51082a2 resolves the selected effect
blockers; application/desktop delivery remains separate.

Worktree `HD2-Decomp-p22-pack`, branch `feature/p22-pack`, clean starting HEAD and
read-only fetched master `f9d5f03183900044ce191530a13f34690b7e8059`.
Existing worktrees/branches/history were inspected before writes; no reset,
checkout change, merge, or change to master/LIFE/UI during implementation. Research/p22 was read
through Git at `ca3b314`; its PACK row reviewed older `799a793`. Current fetched
source is implementation truth. UI contracts were read through Git at `a2d61c6`
(the retained implementation contract names `d01face`), without copying source or
ledger. No resource/mission observation is inferred from static command support.

Resume started from `287c6fc2107a1e60d1b44d0eb93a1badb7c57049`, after the
user-authorized non-force conflict-resolution push. Fresh read-only fetch observed
`origin/master=494cbee40566248084aaf9c8f01e7f75724d13c5`; no master import.
UI message contracts were re-read through Git at
`a3de5e14549e1bf98cb0324fc1097a52960d4267`. They retain the same existing
`append_message`/owned message queue interface used here. No UI source/ledger copied.

## Evidence and ABI

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
The research index was consulted before new exports. Fingerprint-qualified
exports are locators; the decoder wrappers omit ECX/stack parameters in several
Ghidra signatures. Native instructions and vtable slots determine the contract.
Read-only/noanalysis Ghidra exports went to temporary directories; retained
copies, fingerprints and assembly are under ignored `analysis/work/p22-pack-20261111`.
The independent research project was not saved. No original code was executed.

Original evidence reused from the main checkout's ignored WORK:
`process-packet-receivers/005418e0/005a5fb0`,
`human-contact-static-missing/00540db0`,
`human-loading-inventory-commands/00587bf0`,
`human-inventory-swap-dependencies/00587e00`, and Human inventory wrappers.
New exports cover the exact reader/helper entries below, their original paired
writer vtables, B0 presentation `005406C0/00592FE0`, discard `00541CB0`,
`005AC540`, `007866F0`, and the Player/Remote producer slots. Metadata SHA and
native instruction fingerprints are recorded in the receipt. New type storage
is portable source, not the original allocator/vtable/SEH ABI.

## Grammar and codecs

Each command tag is five bits. Command24 ends a Human list; module19 ends the
outer process packet. A command19 inside a Human list has its own payload.
Original switch holes (5 and25..31, plus per-receiver holes) consume their tag
only and continue. There is no invented universal payload skip.

Authority reserved `005A5F80` admits only Human+B0 kind1. Presentation reserved
`005A5FB0` admits kinds1 and7. Null, non-Human, missing B0 and other kinds use
`00541CB0` or `005418E0`, not an empty payload. Positive authority ignores
1/13/14/18/19/20 tags; authority discard consumes1/13/14 but lacks18/19/20.
Presentation lacks17/21; Player presentation also lacks18. Remote presentation
admits18. These are distinct original grammars, including their unusual holes.

Widths below exclude the five-bit tag. `s16` means read16 then sign-extend;
writers preserve the existing unmasked bit-buffer behavior. Scalar offsets refer
to original command fields (command18's88/8C/90 use typed8/C/10 plus separate text).

| Kind | Reader; paired writer | Payload in wire order |
| --- | --- | --- |
| 0 | 005B10C0/1150;1100 | none:005AC760/005AC650 do not read/write inputs; both256-byte inputs zero |
| 1 | 005B1510/16E0;1540 | actor10, event4, subtype4, amount s16, optional1; present: primary2 XYZ, direction3, length8 |
| 2 | 005B1980/19F0;19A0 | item s16 |
| 3 | 005B1180/1230;11A0 | control6 (constant39), value s16, argument8 |
| 4 | 005B1000/1090;1030 | primary-default XYZ twice, secondary-default direction, flags9 |
| 6 | 005B0F70/0FE0;0F90 | Scene2 identity10 |
| 7 | 005B1A10;1AD0 | item s16, source3, destination3, index4, optional1 + actor10 |
| 8 | 005B1BB0;1C40 | current s16, selected s16, current-slot3, selected-slot3, optional1 + actor10 |
| 9 | 005B1E40;1E90 | item s16, slot3 |
| 10 | 005B1F10;2020 | item s16, default direction, scale4, optional1 + s16 |
| 11 | 005B1D10;1D80 | actor10, action4, optional1 + item s16 + slot3 |
| 12 | 005B1290;13F0 | XYZ raw32, type6 (constant43), XYZ raw32; offline7: four more raw32 in50/48/44/4C order |
| 13 | 005B2340;2700 | repeated slot3 + definition32 + identity32; slot7 terminates |
| 14 | 005B2100;2150 | Human10, item s16, argument10, bool1 |
| 15 | 005B0EE0/0F50;0F00 | action4 |
| 16 | 005B0E50/0EC0;0E70 | rank4 |
| 17 | 005B1800/18F0;1820 | amount10, type4, subtype4, actor10; writer clamps amount0..1023 |
| 18 | 005B21D0;2230 | native128-byte-capacity NUL string, s16, s16, bool1 |
| 19 | 005B22B0;22E0 | category8, quiet1 |
| 20 | 005B2A90;2D20 | referenced Human s16, existing00727660/00727480 equipment-only soldier |
| 21 | 005B2F80;2FA0 | Human s16 |
| 22 | 005B2FF0;3010 | bool1 |
| 23 | 005B3060;3080 | bool1 |

Quantizers are the SAME Startup HumanInputShared and existing SessionPacketStream
profiles; no new quantizer, clock, queue or packet-world owner. Helper005AC660
zeros velocity only if all components are strictly below native DOUBLE0.01.
Event1 writes the raw direction and truncates length from stored FLOAT length²,
using its IEEE-word epsilon test then the FLOAT near-one comparison. Scale10
reader maps raw4/15*4+1 and clamps1..5. **The initial zero-multiplier interpretation
was incorrect:** after PUSH005B20AB, FIMUL005B20B2 reads[S+08]=mask15. The merged
writer preserves `(speed-1)*.25*15`; [original-qualified correction](human-ragdoll-source.md)
and the final closure contract supersede that earlier claim.

Command11's absent optional fields were indeterminate in the native reader.
Source gives them the paired writer's -1/slot7 sentinel instead of exposing
uninitialized memory. Command17 source leaves the input value unchanged while
writing the same clamped bits; native overwrites its command field before writing.
Native command18 has a fixed char array; the source owns a string and retains
read_string(128)'s exact bounded consumption. These source adaptations do not
justify complete-function credit or malformed-data native equivalence.

## Concrete owners and producers

HumanPacketCommand owns decoded scalars, strings, inventory values and prepared
records. B0's existing first80/second90 vectors own these values; no packet or actor
pointer is stored in commands. Existing Ladder production and all native stream
selection/framing/order remain. Serializers append in one transaction, restoring
cursor and selected metadata on a source exception. B0 retires commands only
after a successful append. Flush failure removes its attempted19; earlier streams
already enqueued remain retired, so a retry does not duplicate broadcasts.

Player0053E610/0053E7F0 and Remote00592320/00592500 now queue actual move/swap
requests through the existing inventory owner. Same-Human actor becomes-1.
Logical command kind follows the writer vtable: the native swap constructor
stores7 at+4 but its00810CAC writer emits8. No constructor tag extrapolation.
Those producers report the completed queue append, not a completed item transfer.
Actual item identities come from SAME Items. The existing immediate AI loading
move/swap remains restricted to its verified slots. Cross-Human clothing-slot4
swap still requires original Human alignment/eligibility gates; it rejects before
queue mutation. No weapon simulation or item-transfer body was invented.

Command13 snapshots SAME current sending Human storage at serialization, preserving
fixed slots0/4/5/2/3 then forward holsters1/backpack6 and actual item identities.
Authority21 queues20 on SAME second90. Serializer20 resolves its actual referenced
Human then calls the existing005A8B50 equipment snapshot; it does not serialize a
seeded/default soldier or retain a Human until a later flush. Missing required
referenced Human fails the append transaction, leaving the queued command for retry.

Module13 now binds reserved presentation and nonreserved Human presentation to
actual Human services; other actors still reach the existing WorldActors decoder
(including original SpawnPoint/SAME authority-created Human/Players/camera path).
Absent registry slots and original default00405B20/30 module methods return false;
the existing Script module4 SetOn receiver is now bound through its checked
registration. Other Script opcodes and unavailable Versions/Runtime/Shot/HeavyFire
packet receivers still reject. No incoming bytes are silently consumed as a successful unknown module.

Available positive binding includes authority0 nonlocal real Player buffers and
second0, consume-only4/16/22/23, existing Ladder6 and offline15 plus queueing,
21→20; presentation Player consume-only0/3/4/13/15/22/23 and Ladder6; Remote0
actual buffers,4→00583120 actual B0+10 direction copy,6,13/15,19 existing speech,
22 existing HumanScriptLoadingControls suspension,23 clears actual2C1 then calls
existing inline action76. Command14 now binds the ordinary offline notification;
command16 binds six Human stats and actual process rank/message publication below.

The receiver resolves the SAME physical Scene2 slot and published B0 before every
command; an earlier callback can replace B0 or retire the actor. It does not keep
a raw Human/state borrower across that callback. A different actor installed in
the slot is not retargeted. Initial nonreserved receiver selection is checked even
for an empty list. These are source lifetime guards against native stale-pointer
behavior, not claims of the original callback exception/SEH policy.

Script SetOn00424540/00424590 reuses the actual Script module's existing
MissionScriptVisibility and the SAME process broadcast. Its append now restores
cursor/metadata if serialization or00416CF0's runtime predicates throw before PC
mutation. Light dedup/update order and a genuinely false native advance predicate
are unchanged. A focused additional ASan/UBSan/LSan audit verifies the actual
SetOn producer→stream→flush/FIFO→registered module→existing decoder, no-scene
consumption, unsupported/default/receiver failures, predicate failure/retry and100
cycles. Engine/model endpoints are doubles; no other VM opcode was credited.

## Offline notification and rank receivers — resumed checkpoint

`human_packet_notifications.h` completes00588870's ordinary offline branch of
command14 over the SAME Scene2 Human identities, B0 kind, HumanStore profile,
model name, localized text cache and existing HUD message owner. It preserves
the absent/non-Human target return. Authority14 remains its original tag-only
hole; no presentation layout is imposed on authority. Offline7 bypasses the
native client-only medical-item/health mutation, so it needs no combat substitute.

| Source/target classification by005A0890 | Caption | Colour FLOAT RGB | String arguments |
| --- | --- | --- | --- |
| Same Human, Player B0 | 2277 | 0,160,0 | none |
| Same Human, other B0 | 2278 | 255,255,255 | source name |
| Different, source Player | 2274 | 0,160,0 | target name |
| Different, target Player | 2275 | 0,160,0 | source name |
| Different, both other | 2276 | 255,255,255 | source then target name |

005A8320 takes `user_name14`, falling back to **full_name0C**, unlike the existing
005A82C0 objective helper's short name. Without a profile it takes the real model
frame's name. Missing model is an explicit source guard. In the last case native
instructions obtain the target name before the source name, with source,target
varargs order; source preserves both. No Human/state/frame borrower is used after
the callback-visible HUD append.

005A4600 copies its localized caption through a512-byte buffer and formats into
1024 bytes;0069BF70 then formats that result AGAIN, with no arguments, into512.
The typed formatter preserves both passes, NUL termination semantics, `%s`, `%%`
and bounded string width/precision/left alignment. For example an inserted name
containing `%%` loses one percent on the second pass. Unavailable arguments,
other conversions and buffer overflow throw instead of performing native
varargs/stack UB. These are source exception guards, not native error semantics.
Publication uses existing HudSceneLoading0049D2E0's flags gate and owned
HudMessagesLoading00494A40 insertion, without another queue or UI orchestration.

`human_packet_rank.h` implements command16's actual stats/rank path.
005A96A0 copies exactly indices2,1,4,6,7,8 into both currentEC and maximum11C
arrays. Health0, indices3/5 and all other fields remain intact.006A1CB0 starts
with caller defaults800,80,80,80,80,80,50,30,20. Missing HumanStore profile leaves
these intact. Profile stats1/2 are unscaled; stats3/5/6/7/8/9 scale by
`signed(rank)*FLOAT0080FD38+1`, exact factor0.029999999329447746, with native x87
signed64 truncation/low DWORD and signed upper-only100 clamps. Output3 remains
its caller default. Decompiler signatures omit the rank argument; the retained
native disassembly verifies the factor, stack ABI and source fields.

Player005406C0 writes the incoming rank to the SAME process record54 **before**
stats and0072FAC0, so the latter sees an equal rank and selects promotion/green.
Remote00592FE0 scales stats with the **old** process rank, then publishes the
new one. Authority16 still only consumes its payload. No replacement roster or
fixture-derived application rank is introduced.

0072FAC0 holds actual process lock418 (Startup lock0) through rank write,
caption/name lookup and HUD publication. Local downgrade uses22C4/red255,0,0;
equal/upgrade22C5/green0,255,0. Other identities use22C6/22C7 and prepend the
actual process record name. Rank-name00725B00 uses family64==1 ?2792:279C, plus
the new rank with DWORD wrap. All display values are copied before HUD may
replace/clear process records or retire Human/B0. Source RAII releases the lock
on throws; already-applied stats/rank prefixes are not rolled back or replayed.

There is one exact conditional owner gap: when actual configuration224==0 and
record3C is nonzero,00725B00 must read **that record's typed Human/clothing4**
and its item definition44. Current process records have no typed association
writer/view for3C. The actual00725A50 initializer sets3C=0 and family64=2; the
existing spawn publication does not establish3C. Source uses those real fields,
but explicitly throws if the missing association is reached. A raw32 pointer
cast or lookup through a different Players roster would invent ownership.
Configuration224!=0 short-circuits that lookup as native does. This branch is
still mandatory before claiming the whole rank/effect boundary source-closed.

Remote18 was incorrectly labelled speech in the first checkpoint. Verified
005A9C20→005AC4A0→0077CE10 loads a **named special animation**, checks actual
DD0 kind1, uses cache0041FC10, setup007757E0, process request groups/track update
007724E0, and reset00775970 for empty/invalid animation. Existing DD0 lacks this
setup/reset effect interface. Its exact reached failure now names that owner;
no sound/subtitle substitute or research-only implementation credit is claimed.

## Historical H8 dependency review — 2026-11-12

[Reassessment at14d59d3](p22-pack-hum-h8-review.md) supersedes the missing-owner
status in the historical table below. Human now supplies1/10/17, conditional
rank association and selected command3 subtypes; required2/7/8/9/11/12/18/20
positive effects remain absent. No HUM source was imported into PACK in this
review. PACK remains source-open.

## Precise remaining blockers (historical 2026-11-11 snapshot)

These effects are reached failures, not successful empty callbacks. PACK parsing,
serialization and all independent implemented-owner bindings continue around them.
The names identify the required Human/state effect interface, not new packet owners.

| Payload/phase | Required existing-owner effect still missing at fetched master |
| --- | --- |
| 1 presentation | Human event/death005A15D0 over its actual B0/DD0, not another event queue |
| 2 authority/presentation | active005A6930/005A6A00 clothing; existing PreparedHumanClothing supplies cold preparation and explicitly rejects active DD0/Sight/Players refresh |
| 3 authority/Remote | B0+DC00586200 typed control validation and positive action/attachment effects |
| 7/8 authority/presentation | Human005A2D50/005A2E90 validation and005A2E10/005A2F50 apply; existing immediate loading move/swap is narrower and rejects Player/Remote, general/cross-Human transfers |
| 9 authority/presentation | Human item005A39D0/005A3A40 |
| 10 authority/presentation | Human item throw005A3490/005A3750 |
| 11 authority/presentation | Human item005A37C0/005A38F0 |
| 12 authority/Player/Remote | B0 action0053D5D0/0053D870/00594110 and their real control/effect owners |
| 16 presentation, configuration224==0 and record3C nonzero | typed process record3C Human/clothing4 association for00725B00 rank name; stat copy and other rank/message branches now implemented |
| 17 authority | Human virtualD4=005A1C90 damage; existing Car Human impact endpoint already explicitly rejects the same missing receiver |
| 18 Remote | special animation005A9C20/005AC4A0/0077CE10, actual DD0 setup007757E0/reset00775970 and track requests |
| 20 presentation, including discard | referenced Remote Human0059EB60 equipment replacement, SAME Items/B0/DD0; absent/non-Remote target genuinely has no effect |

Packets for these families are fully decoded then report the exact reached effect.
An exception retires this packet's owned bytes; later FIFO packets remain owned
and unlocked. It is not safe to replay an entire failed decoder packet after
partial earlier effects. Stream append/enqueue retry is distinct and audited.

Other event/clothing/combat/control producers require the corresponding Human/UI
owners; their wire serializers exist here, but seeded commands do not establish
those gameplay producers. No missing Human simulation, weapons, combat, AI, UI
orchestration, FRAME/RENDER/GUARD/OWN or multiplayer transport is supplied to make
an audit succeed. No selected mission has been observed to require any conditional
branch. The full PACK effect boundary stays unchecked until its owners are real.

## Verification and limits

Focused temporary native ASan/UBSan/LSan uses the production codecs,
SessionLocalPackets, GameLoadingModules, HumanWorldLoadingServices and actual
Base/Player/Remote command/buffer owners. Scene/Items, DD0 Ladder, graphics,
input/platform, sound and registry endpoints are explicit doubles. It covers all23
payloads, independent original widths and literal optional/signed goldens, every
byte truncation, mixed lists/terminators/holes, all discard destinations,
Player/Remote producers, actual snapshots, supported receiver paths, every
explicit Player/authority effect failure, callback replacement/retirement,
serializer/receiver/enqueue/partial-flush failures, storage/node allocation
failure under lock, loaded gates and100 World unbind/rebind cycles. No engine
outcome is inferred. Sanitizer runtimes are the existing local tools; LSan ran
outside ptrace restrictions. Temporary sources/binaries are fingerprinted and removed.

The resumed notification/rank audit separately runs serialization→shared stream→
flush/FIFO→production module13/Human services→actual HudMessagesLoading queue,
using native Human/state/Scene/platform endpoint doubles and initialized fixture
records/captions. It covers the five notifications, Player/Remote rank order,
stat preservation, conditional record3C failure/config bypass, formatting/overflow,
HUD throw/later FIFO recovery, callback-driven process-record replacement and
Human+B0 destruction, pre-payload-complete truncation and100 mixed-command
unbind/rebind cycles. It does not supply real gameplay producers or mission data.
Its receipt records separate source/binary fingerprints and retained logs. The
strict i386 composition below instantiates the real application Human/state owners.

Strict i386 compile/link instantiates the actual application controller,
GameSessionLifecycle, World/module/Human/packet graph and concrete producers.
Existing application-launch link also passes; neither executable was run.
Frozen-ledger checks and existing tracker tests pass. Only implemented bodies
present in the frozen inventory receive partial claims. No complete promotion,
engine delegation credit, absent-function credit or desktop acceptance.
