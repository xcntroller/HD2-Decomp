# Initial synchronization and respawn boundaries

This closes the architecture of the received-start path. It does not claim all
nested snapshot payloads are recovered. Evidence:00724110/00723000/00723220,
0072A0B0 command0x15,00728450 command0x15,0048DB30/60, the actual five module
receivers and0069D780/0069D9C0, indexed in [manifest](evidence/manifest.json).

## Input manager replacement before old-world retirement

00724110 sets process5044=1. If process4E88 and current profile body38 are nonzero,
it invokes optional00724370(false); that optional service is not established as a
required ordinary gameplay receiver here. It then calls actual00723000:

1.00723220 invokes existing process14 manager virtual10(true) and nulls14.
2. AllocateE8 aligned16 and construct0073EC50; publish process14. Install actual
   identity-dependent getter callbacks0072D1C0 atE4 and0072D1F0 atE0.
3. Configure real session quantizers20/120/A0, bind both manager profile virtuals,
   flags3B, local peer4C, interval float bits3D0F5C29 and shared00982F94=flags&1.
   These profile/default contracts already exist in main and are reused.

This occurs **before** received-start0 destroys the old application game. It is
not a stop/wait transport call. No safe allocation-null branch or atomic rollback
exists at the publish/configure writes. Do not move it after world construction
and call the changed order original behavior.

At reviewed main, `host/game_world_loading.h` constructs `Inputs inputs_` in the
world's `Owners`; only `HumanInputShared` is in StartupSession. The native B0/input
record lifetimes already require ordered detachment. P2.8 must adapt that ownership
explicitly: reuse the actual input type/profiles/getters and one canonical manager,
with checked borrower retirement/publication. Do not leave a process manager and
an unrelated world manager both claiming Human history. A native policy that
defers old-manager destruction until borrowers detach is a **safety deviation**
requiring focused verification, not recovered original transactionality.

## Count gate and full peer state

After shared world initialize, client requests9: local peer5 and signed16 count of
nonnull Scene2 slots>=32. Authority compares its own count, not identity/content.
A0B0 command0x15 writes1-bit agreement. False has no success payload; presentation
requests status11. True writes vector size8 and **all** C4 peer records in order:

- transport00:32, name08 NUL;
- Human actor10:32 orFFFFFFFF when peer3C null;
- peer state00749A70/00749AE0:46bits from actual peer+40 receiver, described below;
- only if Human ID!=-1: position three raw32 float words, normalized direction
  three raw32 float words, actual Human→prepared record005A9040 then full
  00727480/00727660. Normalization preserves native threshold/fallback arithmetic.

Peer-state order is relative+0C:11 bits,+04:10,+08:10,+10:10,+14:5.
Writer encodes negative0C by adding2048 and mutates source10 to1023 when outside
0..1023. Reader subtracts2048 only if encoded0C is **strictly greater than1024**
(encoded1024 remains positive1024); other fields zero-extend. Absolute peer fields
are4C,44,48,50,54 respectively. Preserve this boundary quirk and source mutation.

Reader client-only clears/repopulates actual peer records and creates Humans via
00612A60; authority does not create duplicates from its own snapshot. Additional
Human/state actions, local roster and B0 flags use the actual recovered path. This
is a different route from SpawnPoint's incremental quantized spawn packet.

Then writer0048DB30 traverses19 registered slots **ascending**, skipping null,
virtual38(packet). Reader0048DB60 does the same via virtual3C(packet). Afterward
006991B0 writes4 bits per existing missionA8 child status10 (no separate count);
00699220 reads4 per same actual child count and invokes existing0069BC40 with
**1-based index** and decoded value. Preserve that setter’s real side effects
rather than directly replacing child fields. Authority then sets target peer74=2
if transport lookup succeeds and process0097D078=1 **before**0072BF90 submission.
Ignored send/finalization returns do not prove successful synchronization.
Presentation sets0097D078=1 after its module pass, reads objective state, changes
match phase0069AA50(2), opens retained selector-2 and handles the original status11
gates. None of these writes prove transport or content agreement.

## Real module snapshot pass

Ordinary packet virtual24/28, persistence30/34 and initial-sync38/3C are different
interfaces. This pass has no generic per-module length envelope. Registration,
loaded collection cardinality and earlier module mutations constrain parsing.

| Kind | Writer38 | Reader3C | Actual reader contract / remaining leaf |
|---|---|---|---|
|2 Physics|00410EF0|00410F70|Only when real body vector30..34 nonempty: signed16 indices untilFFFF; decode0073D7A0 record; for valid index call actual physics virtual4C(body150,1,record,zeroVector). Requires existing bodies; unknown solver record is not a generic transform|
|4 Script|0042AFF0|0042B330|bit2 and nonnull packet:1-bit scene state; if set13 raw32 values plus1-bit value into scene virtual11C selectors1..14; signed16 frame count, each name(cap255)+1bit OnOff via actual scene name lookup/virtual30. Requires world scene. This is presentation/frame state, not saved VM/global variables|
|7 Versions|0043F090|0043F120|bit2 and nonnull: signed16 index sequence, terminal index `< -16-recordCount`; negative index maps slot `-index-1`, client-only purges actual parts backwards through0043C040; nonnegative existing record calls0043DB40. Nested version/model fields not closed|
|12 TempActors|004782C0|00478520|Specific type records below; this creates certain dynamic actors before Scene2 state replay|
|13 Actors|004769D0|00476A10|Capture actual slot count, ascending nonnull actors virtual50(writer)/54(reader). Requires same existing positional actor population, including Humans created by peer stage|
|Other registered kinds|0048D940|0048D940|Verified RET4 no-I/O shared default; no invented module state|

All concrete table entries and byte-span digests are in the manifest. Writer
addresses are verified virtual targets; unexported writer bodies are not claimed
paired-format-ready. Missing real collections can alter whether bytes are read;
skip-success cannot replace a required receiver.

Module12 writer/reader pair is now bounded at the outer format: leading module70
word16 (reader sign-extends), then **6-bit** type (constant39), actor10 signed16,
type payload, terminating type0 with no following identity. Types emitted receive
these actual branches:

| Type | Payload before actual actor50/54 | Creation/restore |
|---|---|---|
|2|actor9C→10:16, actor90 raw:16, world quantized position and direction|00478D30(identity,signed16,signed16,pose,direction), then actual actor54|
|11|actor90→0C:16, actor90→10:16, same quantized pose/direction|004790C0(identity,-1,signed16,signed16,pose,derived impulse(0,-.1,0),direction), then actor54|
|35|Only actor10 identity after type|Lookup existing actual004772F0(identity), then actor54; no factory/deferred reference in this branch|
|Other nonzero kinds|Writer emits type+identity only; reader has no type-specific apply|Not proof these can be populated safely in a supported route|

Writer type11's first field is `actor+90→0C`, second `actor+90→10`; preserve these
actual expressions rather than guessing modern item/owner IDs. Reader has no null
guard before returned/existing actor virtual54. It does not provide atomic rollback
or a general existing-actor purge. Exact factory behavior controls duplicate/repeat
semantics. Type35 existing identity is a hard data prerequisite.

Known nested actor pairs (packet state, **not** save48/4C): type2 PlacedItem
005B8880/005B8890 are verified RET4 no-I/O; Human005AA770→005A9C40 and
005AA780→005AA1B0; Car004DDF90/004DE270; type11 RuntimeWeapon005B70F0/005B7100;
type35 physical005E2930/005E29A0. Their reached nonempty bodies are precise remaining
format blockers for a complete populated snapshot. Do not reuse P2.6 actor bytes
merely because the same containing object owns both interfaces.

## Respawn ownership and failure

Authority application request1 constructs owned prepared soldier/stats allocations
and transfers them into0069D9C0. That receiver chooses actual style-dependent spawn
pose (style0 bounded attempts via00475840/0069D6A0; team styles validate existing
SpawnPoint/side and005ED700, falling back0069D940). Missing spawn emits event0F,
retires prepared contents005AA0F0 and frees both allocations.

If006B66B0 applies, config270 limits per-peerB0 or game84 usage according to27C;
-1 means unlimited. The reached limit-exhausted return releases the lock but does
not free the two transferred allocations in this body. Preserve that original
partial ownership quirk in research; native cleanup would be an explicit fix.
Objective style can update game78/70 and soldier09/model54. Team style1 adjusts
direction with0069E0A0. Nonoffline calls0069D780 to update the existing Human, then
allocates0x30 pending record: two4-word pose vectors, owned soldier/stats pointers
at20/24 and peer slot28, appended to game9C..A0. Consumer/completion/destructor
ownership of that pending vector remains the exact follow-up boundary.

0069D780 client-only style3 updates peerB0/game84 counters according to27C. Under418
it validates peer slot, named record and Human3C, calls0059F050 with pose/soldier/
stats, applies objective-kind1 B0+550 orkind7+2C0 byte, unlocks, conditionally calls
00724570(profile body38 gate), and clears local game90/HUD when local. It updates an
**existing** Human; no second actor creation is established by this receiver.
Nested0059F050, spawn pose validation, queued completion and optional service remain
conditional implementation dependencies, with their original failure behavior
unproven until recovered. This is not an offline campaign reinforcement contract.
