# Human speech and shared dubbing consumers — source checkpoint, 2026-11-10

Continue the full first P2.2 checkbox. Application loading remains disabled;
no loaded world, first rendered world frame or desktop acceptance. This extends
[shared world consumers](shared-world-loading-consumers.md), the existing
[Human/dubbing source](human-control-loading.md) and [subtitles](sound-world-loading.md).
[Receipt](checkpoints/p2.2-human-speech-loading-20261110.json).

## Phrase windows and scalar decoding

`human_speech_loading.h` implements005AF7D0/F7F0/F810/F850/F8E0/FA60/FD10
against the SAME SpeechDefinition categories and shared Startup CRT stream.
Deadline field14 is unsigned game28+delay with DWORD wrapping; unsigned
now<deadline blocks guarded selection. Equality permits it. Enabled/list gates
follow the time test. FD10 updates enabled categories even for zero/negative
wrapped delay; F8E0 only sets its optional delay when signed delay>0.

Rotation removes the first phrase, adjusts the actual cursor (zero→remaining
end, otherwise decrement), and consumes ONE CRT draw. Cursor nowzero inserts at
rand%(remaining+1)+1; other positions use cursor+rand%(remaining-cursor+2).
Out-of-vector choices append. Cursor remains the adjusted window index across
insertion. This preserves the original no-repeat window, including singleton
random consumption. FA60 ignores the deadline but preserves enabled/list gates.

005B0440 maps unsigned phrase%10000+90000000.005B0460 uses unsigned QWORD FILD,
exact DOUBLE00811978=-1e-6, signed64 truncate/low DWORD then -90 subtraction.
005B0490 uses exact positive DOUBLE00811980, retains the x87 fractional part
following the first truncate, multiplies FLOAT100, then truncates again.
Decimal digit extraction is not equivalent: FLOAT/DOUBLE rounding near integral
million boundaries changes the result. Native checks include independent exact
rational golden values for the captured constants and UINT32 extremes.

## Actual B0 callers, mutation and output order

005882F0 returns0 for absent Human2C0 speech definition. Otherwise it selects
and rotates the FIRST phrase before checking signed B0B8>0 or flags50 bit40000.
A blocked Human can therefore consume a phrase/random draw. An accepted request
stores B8=1000 before category delay publication. Offline00588170 then rotates
AGAIN with FA60, ignoring that newly published deadline. Player-kind1 receives
its real subtitle notification before the quiet gate. Quiet suppresses only
camera/head lookup/playback, not the second rotation or notification.

Nonquiet uses actual scene camera15C and Human head marker260; XYZ differences
spill to FLOAT, then native (Z²+Y²)+X². Radius is2*005B0490(second phrase), with
signed integer→FLOAT conversion. Equal radius plays; native unordered comparison
also reaches SOUND. Real Startup.play_game_speech receives actual Human,
null relative model, SECOND sample identity, decoded near parameter, doubled
range and subtitlesfalse. The request returns catalogue sample-size metadata
for the FIRST sample even when quiet/distance prevents playback. No WAV is
opened merely to obtain this metadata.

005383D0 checks bit40000 before selecting any phrase, then increments category
with DWORD wrapping when variant and either Active18A/188 or actual process43
is nonzero.00542060 offline uses actual Human16C FISTP/low DWORD clamped0..100
(005A6280), process group0 presence and PlayerC0 to choose variant and quiet.
The shared world service now supplies these actual source callers/process
queries, replacing the required speech declarations. Other nonoffline queued
kind13 task/peer/scanning branches reject after the appropriate mutation prefix;
the absent-bit2 SOUND4C path is not required by offline7. B8's later simulation
countdown is separate; it is not fabricated by a speech request.

## Shared SOUND, subtitles and player messages

SOUND virtual94/004326C0 jumps to00439A10: SAME SoundCatalog signed-key map
operator[] inserts absent keys with0. Signed mapped value>=100 returns it;
otherwise emit the actual unknown-length diagnostic and return2000 without
replacing the retained map value. Both entries are absent from the frozen
inventory; the getter gets no credit.00436470 is also absent, and the already
recovered channel implementation is not counted again.

`HumanDubbingLoading` supplies actual Human model, typed2B8 then2BC attachment
borrowers, SAME Players current2C/override58, and MissionSubtitlesLoading.speech.
Human's attachment fields now hold Frame types rather than native pointers in
DWORDs; the constructor's zero values are unchanged. Startup owns the same16
channels. Channels must retire before models/attachments/session disappear;
ordered containing session cleanup remains unconstructed, not implicitly solved.

004900E0 caption lookup does not select/draw anything: signed ID>=90000000,
first matching entry, first text. Missing returns actual process0089BB44 fallback;
PE confirms BSSzero and only a direct reader, so Startup retains its cold null
borrower with no fabricated populated writer. Empty records return exact
"NO TEXT WITH THIS ID !!!"; null first text returns an empty string.

005A46A0 offline only for property19C1 looks up/rereads that caption and formats
005A82C0 actual profile user14/short10 or Actor name plus": "+caption. Its
0069BF70 zero-argument formatting path interprets %% as a single percent;
undefined conversions and native buffer overflow reject. Actual colour50/150/50
reaches0049D2E0: any nonzero HUD flags forward into SAME HUD message queue;
zero flags return.005A9B30 is a genuine offline RET and gets no new body credit;
the wider general varargs0069BF70 formatter also gets none here.

## Evidence and verification

Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored WORK=analysis/work/mission-world-20261103 holds successful read-only
human-loading-speech-required/, human-speech-category-required/,
human-speech-presentation-required/, human-speech-notification-consumers/,
earlier human-feedback-consumers-required/005882F0 and
human-shared-player-feedback-required/005383D0, plus original00542060.
Static human-speech-*.asm and scalar/process/virtual/fallback JSON captures
preserve correct register/stack prototypes and exact constants. Export at
004326C0 reported no function; static jump/map body and absent inventory were
used instead. No original game/DLL/Windows fixture was executed.

Focused native ASan/UBSan/LSan passes actual Human/Actor/storage/B0 bind,
category window/cursor/deadline/wrap, scalar goldens, two rotations and first vs
second identity, suppression/quiet/variant/process/distance/unordered/failure
prefixes, metadata map insertion/fallback, parsed duplicate captions and actual
HUD message queue, real Players control/dubbing borrower/subtitle routing.
Frame/engine/dubbing playback/clock/process endpoint values are doubles. Full
Dubbing channel bodies were verified previously; they are not re-audited here.
Strict i386 composes actual Human Active/Player types, common world service,
Startup process/catalogue/dubbing, real HUD/messages/subtitles and typed frame
attachments. Containing frame/aggregate/presentation/item/style/world owners
remain declarations. App links and27 tracker checks pass. Temporary audit
sources/include/object/executable are hashed/removed in the receipt.

18 eligible partials added, one existing0071DF30 extended. Ledger34 complete /
1,889 partial (1,884 client/five engine)/18,783 frozen, zero complete promotions.
Remaining Human factory consumers include actual selection feedback, stance/
area/Sight bindings. Continue those and containing Human/VM/FPV/HUD/SOUND/session
ownership, required integrity observed publication, ordered loader and first
camera/update/render/failure/repeat, then user desktop acceptance.
