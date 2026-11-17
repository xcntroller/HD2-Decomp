# Reached AddDiaryText and following game-type predicate — 2026-11-17

Source-only checkpoint after422d793. Actual1361c82f user run passes the alarm/
Run continuation and activation, Scene78 render, fixed-update, input/full-update
prefix, then handles missing command137 AddDiaryText with result1. Exact
executable/logs:WORK/first-frame-script137-return. Stable scene/control/game entry
and all visual fade/diary presentation remain unaccepted.
[Receipt](checkpoints/p22-frame-script-diary-20261117.json).

00416E10 opcode89 translates byte30 at0041CE44 to00419D64. Original reads integerA0
operand0 BEFORE nullable application20 getter0041D760. Present profile repeats
that getter, chooses006AE510's profile94/B8 record using selected root (exact1
choosesB8, all other values94), then006A7820 appends the raw DWORD to pending18.
It preserves order and duplicates, and skips append when the record's first byte
is nonzero. Missing profile skips mutation; both paths guarded-advance00416CF0
and returntrue. There is no text localization/render or packet at this receiver.

ProfileProgress now owns suppression, stored and pending arrays inside the actual
ProfileState application20 owner.006B5D20 assembly proves cold flag0 and both
arrays empty;006A77E0 clears stored then pending without clearing suppression or
freeing capacity. Existing preparation discoveries now borrow these same records
and append through that gate, instead of owning a separate array invisible to
mission scripts. Existing preparation reload/cancel/route behavior is retained;
no new save-file chunks, commit-to-stored operation or successful save/load service
is supplied. Profile persistence/debrief/loaded-save suppression producers remain
separate required work. The flag has real storage for its original future writer.

Startup exposes its existing nullable profile pointer without allocating a flow.
World services select the same root adaptation already used by SessionSelection:
actual offline request.preparation.root (checked against the live route), or actual
application GameData E8 for network. This avoids a second root/default or stale
startup E8 in the typed offline selection bridge. Profile and preparation outlive
World/Script; no retained profile loan or extra profile is created. Review's OWN
changes in human_world_loading_services.h were preserved by matching in-place
insertions, never a full-file copy from main.

Captured TUT_Objectives adds4000,45,4001, then immediately evaluates _SPGetGameType
in two If statements. Added original0041E3F0 predicate1A/table0041FA80 ->0041E489:
read actual mode04 then08, DWORD wrapping mode08+4*mode04, accept signed0..10,
otherwise returnFFFFFFFF. Actual existing startup game_mode_field4 and selected
route game mode supply those distinct native fields; GameConfiguration.field04
and process mode flags are not substitutes. No fabricated predicate truth value.

127 native ASan/UBSan/leak checks use actual context, profile/record, preparation
borrower and command/predicate policies with explicit frame/application endpoints.
They cover current-profile getter count/order, missing profile versus integer
lookup, duplicate/raw IDs, exact root1/other selection, same pending identity as
preparation, suppression/reset/capacity, changed-owner/root failure before PC
advance, authority/PC gates, valid/invalid/wrapping game types. Actual Startup/
World/VM/preparation/application/probe i386 O2/Werror build and stage-only hashes
are recorded in receipt. Fixtures are fingerprinted then removed; no permanent
tests or agent execution of original game/DLLs/Proton/Windows fixtures.

Research index preceded lookup. Existing00416E10/0041E3F0/006AE510/006A7820 and
006B4A60 exports were reused; bounded raw0041D760/006B5D20/006A77E0, translation
and predicate assembly remain under WORK=analysis/work/p22-frame-closure-20261117.
PE SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Full recoverable review patch includes tracked/untracked headers. OWN dirty source
and prior artifact remain unchanged. Interactive review overlay stays uncommitted.
No count/promotions/matrix closure: FRAME/HUM/OWN and wider VM remain partial.
Next: actual revised desktop continuation and its first concrete failure.
