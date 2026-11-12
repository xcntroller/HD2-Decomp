# PACK action/speech dependency checkpoint — 2026-11-12

This continuation preserves the previous agent's two working-file speech edits
and completes the ordinary0053D5D0/0053D870/00594110 policy in
`human_player_packet_action.h`. **Command12 remains open** at support33/34;
common3 and PACK integration remain required. No successful support default or
application loading was added. Full delayed Players virtual0 frame execution
remains an H8 obligation, distinct from the packet effect's delayed publication.

Original fingerprint remains8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Reused original player-action C/raw assembly/static table captures named in
[the boundary](human-h8-pack-action-boundary.md). Raw00459470 proves virtual2C
is exactly one BYTE43 store and RET4; virtual30/00459480 returns the raw byte.
The frozen inventory omits00459470, so it receives no new credit.

The original0080FB28 vtable was inspected directly: virtual0=00456020,
virtual2C=00459470,virtual30=00459480. Research index preceded new read-only
Ghidra exports. Initial direct00456020 export failed because no function starts
there in the saved analysis. A temporary script created only in-memory functions
at00456020/00459470, exported them plus existing HeavyFire lifetime workers,
and read-only Ghidra discarded all changes. Successful exports/logs are in
WORK `human-pack-action-players-resume-v2`; the failed first log remains labelled
research failure. No original binary execution or saved project/type mutation.

## Implemented policy

Ordinary Player actions retain the native mode7/non7 split and special53/56
lookup, mapped inline command suppression4C, Players speech mode setter/getter,
speech return into520, RandFloat*500+1000 signed64 truncation fallback and the
complete command24 + nine DWORDs30..50 delayed publication. Player/Remote
presentation only applies the mapped inline command. Authority publishes the
successful command12 to SAME second90 even offline, as00540DB0 shows. Inline,
speech and random callbacks are followed by physical Human/B0 validation before
further borrower use; source throws on retirement without replaying prefixes.

`HumanSpeechLoading::request` now admits the required flags1 nonoffline path:
source kind1 queues directly, other sources scan all physical slots31..0 with
native live/suppressed/player-controlled and distance gates. It does not stop
after finding a listener. Phrase selection/cooldown/B8 prefixes and FIRST phrase
length remain; queue is actual command19, not action-pool13. Source flags1-clear
immediate SOUND4C remains a reached requirement. Received00588170 playback has
no native mode gate; kind19 uses the SAME speech/dubbing/Players notification
endpoints. Mode7 still directly plays its second phrase.

Action table aliases after42 are captured through56; values outside that
recovered range reject instead of reading unrelated native storage. Specials53/56
skip table lookup. Original unchecked arbitrary-index equivalence is not claimed.

## Verification

Temporary native ASan/UBSan with leak detection passes100 speech lifetimes:
nonoffline direct and listener-gated command19, all32 slot observations, dead/
suppressed/distant listeners, cooldown, queue throw prefixes, suppression,
offline playback, received notification/quiet behavior and real codec roundtrip.
Another100 policy lifetimes covers raw BYTE preservation, mapping/special speech,
full nine-DWORD copy, mode gates, random fallback, exception prefixes and deleting
the state during inline callback. Human/B0/Scene/engine/SOUND endpoints are doubles;
actual packet codec and PlayersProcess scalar owner are production source.
Support endpoint is explicitly doubled and rejection-tested, not reconstructed.
Concrete i386 host full packet dispatch link and launcher/probe builds are checked
separately in the receipt. Windows executables are never run.

## Next dependencies

Complete HeavyFire16 actual request/configuration/list ownership and native
constructor/presentation/retirement policies for actions33/34, then common3's
remaining subtype effects, consolidated positive packet verification and PACK
integration. Existing HeavyFire cold update must not silently succeed on newly
populatable lists. Delayed Players00456020's exported body requires recipient/
order/formation dispatch when H8 enables its frame caller; do not replace it
with an idle return. No command12/full H8/PACK checkbox is closed here.
