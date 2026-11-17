# H8 PACK command12 — required dependency boundary

Research continuation after inventory checkpoint44afdbc, 2026-11-12. Command12,
its dependencies below, and parent H8/PACK remain **open**. This note adds no body
credit or completed source checkbox. Do not implement a successful generic action
receiver around missing HeavyFire or nonoffline speech effects.

The research index was used before exports. WORK
`analysis/work/mission-world-20261103/human-h8-pack-action-index.txt`,
`human-h8-pack-action-place-index.txt`, `human-h8-pack-heavy-producer-index.txt`
and `human-h8-pack-action-speech-index.txt` retain lookup results. Two sequential
read-only/noanalysis Ghidra runs successfully exported0053D520/005467C0 into
`human-h8-pack-player-action` and00472E50/00472CE0 into
`human-h8-pack-heavy-producers`. No ERROR, project/type edits, builds running in
parallel, original or Windows execution. Original SHA remains
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.

## Receiver and mapped action

Original0053D5D0 authority,0053D870 Player presentation and00594110 Remote
presentation C are in WORK `human-h8-pack-effects`. The latter two are identical:
action53/56 do nothing; otherwise read the action's mapped inline command from
0083C348+action*8; unless4C, send Human005A45F0 a type1 command with mapped action
and null/default remaining arguments. Actual `HumanInlineLoading::action_command`
is the existing005A45F0→005ABD90(case1)→005ABB50/Look receiver; **this is not
common3/DC00586200**. No new generic common-command routing is appropriate.

Authority actions33/34 immediately call005467C0(action,command10 position,
command30 direction) and return that result, before the ordinary Player fields.
Other nonoffline actions skip special53/56, apply mapped inline command unless4C,
request005882F0(category=0083C34C+action*8,delay100,flags1,quiet0), then return1.
Offline7 instead calls0053D520, reads the actual Players process virtual30 byte
into PlayerC0, resets Player520, applies mapped inline command/speech, then if520
is zero computes native engine-RandFloat*0080F6F0+0080F708 followed by __ftol.
Finally it writes command24 to Player4E4 and copies all nine DWORDs command30..50
to Player4F0..510, including direction W and action/extra fields. Preserve order
and queued inline replay through existing Player frame; do not normalize/copy
only the visibly used XYZ fields.

Offline speech53/56 selects0083C4A0[command44] only for index0..38 and category!=8A;
action56 adds1. Other actions21..32 add bool(PlayerC0) to their ordinary category.
Other mapped speech uses the ordinary category. Native request's return sets520;
there is no invented successful timer if an effect throws.

0053D520 is the **Players process virtual2C setter**, not a DD0 action setter:
actions0/2/4/6/8/10/12/14/16/18/19/20/53 pass0;1/3/5/7/9/11/13/15/17/56 pass1;
others return without a setter call. Getter virtual30 is actual00459480, returning
raw BYTE43 (preserve neighboring bytes and avoid an unproven bool normalization).
Recover setter's complete body before assuming it only stores43.

`human-h8-pack-player-action-static.json` captures section-mapped raw action pairs,
39 speech categories and constants. Ordinary action table0..42 ends just before
speech table0083C4A0. The native unchecked action expression for43..56 aliases
that following data; special53/56 avoid the ordinary lookup. Do not infer valid
semantic enum cases solely from contiguous readable bytes. Packet action is six
bits; research actual producers/bounds and preserve required original lookups.

## Action33/34 HeavyFire support dependency

005467C0 looks for actual inventory **definition243/244**, not runtime identity,
using005A78F0. Missing definition instance returnsfalse. It scales command
XYZ direction by0080F6F0, performs real006F7C90(start,displacement,hit,NULL,1,0),
and returnsfalse on a genuine miss; unavailable collision ownership is a required
failure. This differs from the vertical completion-ray helper; reuse SAME World
loaded tree/buckets and general CollisionRay with the exact masks/normal policy.

The hit builds a native support request using X/Z, actual Human borrower and
005A8820(false) side. Nonoffline side uses actual configuration+A8+50/54;
offline action33 uses bool team, whereas action34 still reads configuration50/54.
Action33 includes negated directionX/Z, original RandFloat→__ftol support type,
and hitY+00810928. Inspect raw for its omitted random scaling and exact slots;
C types conflate borrowed pointers with floats. No source Human/side may be
truncated into a DWORD in a native adaptation.

00471FB0 returns process0089B1B0, **HeavyFireSupport module16**, not Mine/runtime
module12. Its base constructor00472190 first installs00810058 with pure virtual
entries; containing00471FC0 **replaces it** at00472059 with00810050. Actual vtable
virtual0→00472E50, virtual4→00472CE0. Do not treat the base purecall table as the
loaded receiver or substitute Mine placement. Current actual host owner is
`HeavyFireModuleLoading`, bound by `GameWorldLoading`; its Startup
`HeavyFireLoadingState` persists across World lifetimes.

00472E50 authority producer: mode1 clear returns2; support flag1B0+side set returns1;
request support type outside0..15 returns3. Otherwise it constructs00470530 with
actual type-specific module configuration10+type*18 and request fields, inserts
into the SAME owned190/198 list, then sets support flag. It publishes actual
module16 packet (subtype0, full request fields and Human identity) through
0072BEB0/0073CF50/0072BF90 and returns0. The native packet is still produced after
allocation failure; source exceptions must not become fabricated success.

00472CE0 similarly uses flag1B2+side, constructs004719F0 with positionY=300.f,
inserts into SAME1A0/1A8 list and sets flag, publishes subtype1 and full position/
Human fields, then returns0. Both callers005467C0 ignore module result and return1
after a successful item/ray prefix. Preserve that distinction from actually
creating a support object when a busy flag rejects it.

Current module16 cold update/finish comments assume no source producer. They
must change when these actual owners become producible: constructor/configuration,
list ownership, request publication, populated frame/finish/close paths and flag
lifetimes require actual receivers or exact required guards. Never retain a
successful empty update on a now-populatable list. Pending source lookups:
00470530,004719F0,004732B0 config initialization, matching retirement/update
00472270/00472510/00472C60 and native insertion00473070/004729C0. Use index first.
No implementation or credit for these dependencies has been added yet.

## Nonoffline speech dependency

Existing actual `HumanSpeechLoading::request`005882F0 only implements offline7.
Original full C is already indexed at `human-feedback-consumers-required/005882f0.c`;
raw `human-speech-common-abi.asm` resolves the decompiler's incorrect float/int
expressions. It first selects/rotates guarded phrase, tests B8 and suppression,
stores B8=1000 and postpones category. Nonoffline command12 needs its flags1
branch: conditionally append **frame command0x13 (=decimal19), not an AI action
pool13**, with category8/quiet-byteC to the SAME B0 second90 FIFO. Actual queue
and kind19 serializer already exist. Source kind1 appends directly; other Human
kinds scan reserved Humans31..0, actual alive/player-ready/distance gates and
keep scanning after a match. Return FIRST phrase's actual speech length whether
or not a task is appended. Actual raw source eligibility must be recovered.

Flags1 skips the original mode2-clear/flags1-clear immediate SOUND prefix; broader
speech callers must retain required failure for that prefix until supplied.
Existing World received kind19 still rejects nonoffline00588170. This playback/
packet effect is a reached dependency of the newly produced FIFO and must be
completed on SAME speech/Players/HUD/dubbing owners rather than merely queueing
an unconsumable command.00588170's nonoffline producer/quiet and phrase lifetime
policies require the original full body; do not reuse its offline subset by
removing a guard without evidence.

## Verification and continuation

Finish the above actual owners and receiver policy, then temporary native
positive/failure/reentry/FIFO/borrower teardown audits, strict concrete-host i386
full link, latest launcher/probe rebuild, consolidated ledger/contract/handoff
and meaningful source commit. Required unsupported paths remain explicit; none
of this research closes command12, full H8 or application/desktop acceptance.
