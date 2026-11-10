# HUD mission-loading reconstruction

This is source-only required0049A940 HUD loading work within full P2.2. Current
session HUD/artwork/world rendering remains cold. No successful HUD initialization,
rendering/readiness/desktop verification or function-ledger credit is claimed.

## Verified cursor and artwork borrowers

`hud_cursor_loading.h` supplies004A8C10/004A8CC0 at HUD+2E60 and0049DC60.
Bind through the ACTUAL HUD scene+7C: sniper(mask1),scope(mask1),arisaka/k98/
springfield/lee_enfield(mask20). Scope hides immediately when found, then root
and scope hide again, then optional variants4..1. Root/scope missing rejects
instead of native null calls. Missing variants are tolerated during bind; later
mode6 requires them and rejects at the corresponding missing-frame call. No
successful empty-artwork service or readiness flag exists.

Cursor kind0..3 shows sniper then corresponding variant1..4 without hiding other
variants. Kind4 shows scope only;6 hides sniper,scope,variants4..1. Other signed
values, including5, do nothing. HUD0049DC60 modes0/2 call kind6;1 calls kind4;
3 fetches actual control current player, actual005A2B10 hand weapon and second
payload/native4C+0C (serialized payload+08), then applies that kind. Missing
required player/weapon/payload rejects native invalid reads. Payload tag10 has
no0C field and rejects. This source composes the SAME FirstPersonHumanItems/
GameMissionItems path; it does not use selected equipment230 as a substitute.

`hud_artwork_loading.h` supplies0049B730 actions39 and0049B620 player-state
textures20/40. Names are action%02d/playerstate1_%02d/playerstate2_%02d and scene
lookup maskFFFF. Missing visual stores a null borrower. Present visual follows
actual1E0 mesh+04(LOD0),LOD5C first group,+04 material; lazy04000000 invokes
material44 before reading texture0C. No independent texture retain is added.
Actions/first states resolve texture THEN hide visual; second states hide BEFORE
texture resolution. Actual source order remains visible on endpoint failure.
004A8D70 speed root literal is `spee`(scene+7C/FFFF), then frame+48 speed01/02/03
(FFFF) and hide root. Missing children remain null; missing root rejects.

The actual HUD scene owns all these visual/texture borrowers. Clear them before
scene teardown/replacement. Host GameActorFrames supplies explicit HUD-scene
lookup and first-group texture traversal, so mission scene lookup is not silently
used in place of the separate HUD scene. These receivers do not create artwork,
textures or successful render output.

One focused ASan/UBSan check verifies all99 numbered entries, optional missing
lookups, visibility masks/order, switching without invented hiding, real payload
word/signed kinds, required-frame guards/partial failure order, speed names,
four endpoint failures and clearing borrowers. Engine/player/item endpoints
remain fixtures. Strict i386 composes actual host HUD calls, actual Human/items,
model/camera/GEffects and FirstPersonViewLoading enable/reset using this REAL
cursor receiver. Players world/retirement mappings remain declared; unrelated
playback endpoint is declared. Application compile/link passes, no execution.
Receipt hud-artwork-cursor-verification-wip.json fingerprints/removes four audits.

## Remaining full HUD startup boundary

0049A940 creates actual scene13 and loads gameinterface through model cache,
links model into its scene root, creates HUD camera3/light2/click sound4 and
applies original settings before0049B730/0049B620/0049B280/0049B5E0.0049B280
has REQUIRED panel binders, not optional no-ops:004BF2E0/004B0750/004B30E0×6,
00495F40/004A9740/004A9010/004AAAC0/00493F80/004AB860/004A4AE0/004A0EE0/
004AAF20/00496450/004AC440/004AD250/004AD4E0/004AF4A0/004AD8F0/004AE7C0/
004AF2E0/004AF630/004AFC00/004AFE50/004B0000/004AB230/004B4980. It uses actual
main-scene and texture tables, binds other actual HUD/text/inventory consumers,
then0049A940 sets flags9 and initializes debug Border text/0049F1C0. Recover all
needed bodies before publishing initialized HUD, not only cursor and textures.

Matching loaded teardown is0049B1B0 (starts at saved original disassembly),
including0049B900/0049B610/0049B490, debug text release, click/camera/model/scene
lifetime. Cold destructor0049A750 is separate and remains previously verified.
Successful first-person-hud-cursor-loading/,hud-loading-artwork-bindings/ and
hud-cursor-frame-binding/ exports support current contracts. Bulk hud-loading-panels-lifetime/ export completed successfully (all30 bodies,4266 decompiler lines).


## Shared HUD text geometry

`hud_text_loading.h` supplies00644A80/006449D0 against actual TextState. Null
visual does nothing. Otherwise update bounds only when visual1D0&40000 is clear,
copy all eight words at170, sign-extend native width/height WORDs, and truncate
width*FLOAT(1/1024)*BYTE inset90 to integer. Store geometry08=width*minX+inset,
0C=height*minY*FLOAT(-4/3)-1,10=width*maxX-inset,
14=1-height*maxY*FLOAT(4/3), then18=stored10-stored08 and1C=stored0C-stored14.
These are raw native geometry values, not a new claimed pixel-layout/render path.
Use x87 extended intermediates and explicit float stores; exact FLOAT constants
are retained in hud-text-geometry-constants.json.

Depth20 uses actual scene15C camera and visual world-Z. Read near124/far128
BEFORE camera then visual world-matrix updates. Compute
(abs(visualZ-cameraZ)-FLOAT.1-near)/(far-near). Scene-null returns0;
visual/camera-null or depth outside[0,1] returns-1; native unordered comparisons
retain NaN, including0/0. Compare the EXTENDED quotient before its float store:
a quotient just above1 that rounds to1 still rejects. Finally set native72/70
and TextObject dirty. If a depth endpoint fails, geometry08..1C has already been
stored while depth/flags remain unchanged, preserving source call order.

Focused ASan/UBSan composes actual TextState: seven geometry stores, signed WORD
and truncated inset behavior, null paths, exact source order, extended boundary,
Inf/NaN/division-zero and five partial-store endpoint failures. Engine endpoints
are fixtures. Strict actual host TextState/GameFonts/frame/camera/bounds calls
compile; application compile/link passes. Fixed TextState's missing standalone
<stdexcept> include found by this new consumer. Four temporary audits fingerprinted/
removed by hud-text-geometry-verification-wip.json. Full panel/render integration
and ledger/readiness/desktop remain unchanged.


## Small panel loading and matching text lifetime

`hud_small_panels_loading.h` now supplies seven additional required panel binders
with their real text owners/shared font and localization factory. Actual host
GameOwnerBackend::make_hud_text is the SAME factory formerly inline in make_hud;
its extracted method preserves the cold77-text constructor behavior and shares
the same weak font cache with later panel allocations. No second font catalogue
or successful text factory default is introduced. Actual localization uses the
SAME StartupSession text cache through existing006E4FB0 behavior.

-004AD4E0 hint finds Htext/FFFF, allocates normal TextState(inset10), binds geometry,
 then004AD880 clears records/hides required frame. Enabling clears then localizes
 id/style3F before showing. Localization/visibility failures keep prior stores.
-004AF2E0 voting finds voting/FFFF, allocates text, sets inset0 BEFORE geometry,
 then hides a nonnull visual; absent visual is genuinely tolerated by this binder.
-004AFC00 carnage finds carnage_stats/FFFF, binds text geometry with default inset10,
 THEN sets inset0 and appends actual empty shared text/style49. The timing differs
 from voting and must not be collapsed into one zero-inset binding helper.
-004AFE50 server text finds injury/FFFF for geometry, allocates/binds with inset10,
 then sets inset0, localizesA46/style4A and initializes enabled/time0. It owns text,
 not an extra visual reference or a separate artwork frame.
-004B0000 injury text follows the same geometry/inset/localization, THEN invokes
004B01A0(false,-1,-1), which clears the record, sets enabled/time0. Retain the
 localization call even though reset clears its result. Setter clears first;
 enabling caches time, appends localized style4A and sets enabled last.
-004AAAC0 status borrows scene and eight named frames:tabl01/pkneel/play/mwalk/
 mrun/msprint/mpaxis/mpallied, allFFFF. Hide order8,7,2,3,4,5,6; scalar28..34 are
0/0/-1/0. Allocate text, inset0 BEFORE binding tabl01 geometry.
-004AF4A0 camera icon finds camera/FFFF, invokes004AF600(false), resets time/flag0.
 Setter shows only when frame exists; enabling always caches time2000/flag1 even
 if artwork absent. Disabling leaves timer/flag intact.004AF4E0 is a confirmed RET;
 its empty close is original behavior, not an unimplemented successful service.

Matching text releases004AD640/004AF450/004AFDA0/004AFFC0/004B0180/004AAD10
are supplied by explicit close methods. Some original fields remain stale after
release; native unique ownership clears them as a defined safety guard. Borrowed
frames are explicitly discarded before containing HUD scene release. Whole-HUD
close order still must compose these methods with all remaining panels; implicit
C++ fallback destruction is not credited as original containing teardown. Live
rebind rejects instead of leaking prior native text; partial failed binding remains
available to explicit cleanup and exception propagation.

One native ASan/UBSan check composes actual seven-panel/TextState/geometry code:
six shared font references and matching releases, exact inset timing/styles,
localization-before-clear, status ordering/scalars, camera missing-frame/RET
behavior, duplicate guard, required artwork/factory/geometry/visibility and both
localization failures including native partial stores. Factory/engine/localizer
endpoints are fixtures. Strict i386 instantiates actual GameOwnerBackend shared
font/localization factory and all panel/host/geometry/close calls; application
compiles/links. Four temporary audits removed by hud-small-panels-verification-wip.json.
No renderer, active HUD/session/world/desktop/readiness or ledger credit.


## World pointer, info icons, progress and selector artwork

`hud_auxiliary_loading.h` supplies004AB860 world-pointer,004AB230 information
icons and004AAF20 progress with matching004AB950/004AB2D0 RET and004AB0F0 text
cleanup. Pointer borrows BOTH actual HUD/main-world scenes, then finds/hides16
HUD scene+7C/mask1 names from original0083AEC0 table (retained JSON). `k_move`
occurs at1/14/15; preserve three lookups/hides, not a deduplicated table. Set70/74=1,
00=0,0C=-1,78=0 and both position XYZ(.5,-.375,-10). Both native fourth stack
words are unwritten; preserve known lanes as explicit guards, no constructor credit.

Info binds10 info%02d/FFFF visuals, calls actual first LOD0 group+18 BEFORE hiding
each, then00=0 and borrows SAME artwork first-state20/action39 array addresses.
Find/hide infosel/FFFF last. No texture/material replacement or added reference
is invented for group18. HUD scene and artwork arrays must outlive the receiver.
Pointer/info close are confirmed original RETs; explicit borrower discard is a
containing-lifetime guard before destroying scene/tables.

Progress finds prgbrbk/prgbr01(mask1), clears08/0C, sets background actual frame08
position(.5,FLOAT-.58,-10), updates it, finds/hides progress_name(FFFF), sets10=-1,
allocates SAME font/TextState (default inset10), binds geometry and hides background.
An absent bar is genuinely retained as null by this binder; background/name are
required. Close hides background BEFORE releasing text. On a receiver error,
owned text still releases and original first error propagates. Rebind with live
text rejects instead of native leaking overwrite. No progress updates/rendering
or successful initialized containing HUD is claimed.

`hud_weapon_selector_loading.h` supplies004AC440 artwork: select_weapon(mask20),
selector_weapon(mask1), children drkr1/select_weapon00(mask1) hide in order, scene
tabl03/drkr2(mask1) hide, children select_weapon01/02(mask1) hide, hide root,28=0.
004ACDC0's proven cold-record branch is supplied separately. Populated records
require actual004AC370 frame/text destruction and real record producers before
exposure; no successful populated cleanup is supplied or credited. Existing cold
vector is a defined initialization guard, not new constructor recovery credit.
Discard frame borrowers explicitly before scene release.

One focused ASan/UBSan check composes actual source and TextState/geometry:
16 exact/repeated pointer names, known vector lanes, ten group18/hide icons and
SAME array borrowers, progress position/order/scalars/font lifetime, genuine
missing-bar/RET paths, factory/position/geometry/visibility/missing cursor/info/
group failures, text release after close failure, selector masks/order/cold close
and two selector failures. Native engine/factory endpoints are fixtures. Actual
host i386 Frames/GameOwnerBackend/font/geometry/group/selector calls and application
compile/link pass. Four temporary audits fingerprinted/removed by
hud-auxiliary-verification-wip.json. No renderer/active HUD/world/readiness/
desktop/ledger credit. Remaining movie, inventory/weapon-record and larger panel
startup/cleanup dependencies remain mandatory.


## Seven injury movies

`hud_injury_movies_loading.h` supplies004AD250/004AD310 through actual
GameMeshMovieServices IShow ownership. Allocate/open seven streams in ascending
order with exact `GameMovie\AnimHit1.bik` through7, flags0. Nonzero open status
(positive OR negative) releases that stream through IShow+04 and leaves its slot
null, without stop/close. This is native optional stream failure behavior, not a
successful fabricated movie or claim that runtime files were opened here. After
the attempts, find actual injury/FFFF visual, invoke itsA0, first LOD0 group18,
hide and update. No decode/play operation occurs in this loading body.

Clear visits successful slots1..7 in ascending order: IShow18 stop,10 close,
then04 release, clearing each owner. This differs from frame/texture+00 and the
full-screen opening-video lifecycle. Added checked actual stream-close endpoint
10071B80; existing movie factory/open/stop/release endpoints remain shared and
unchanged. Cleanup continues and releases all owners after either endpoint error;
partial bind failures use the same cleanup and preserve the first error. Duplicate
live bind rejects native leaking overwrite. Artwork remains borrowed until explicit
discard before HUD scene release. No texture-restoration/playback producer is
invented in this loading-only receiver.

Focused ASan/UBSan covers exact paths/flags, both nonzero-open signs/release-only,
injury call order, ascending cleanup with simultaneous stop/close failures,
null/throwing factory/open/missing-artwork/prepare failures and all-streams-failed
native path. Movie/frame endpoints are fixtures; actual host GameMeshMovieServices
and all frame/group/IShow calls compile in strict i386. Application builds; no
movie decode, original/Windows/desktop execution, readiness or ledger credit.
Receipt hud-injury-movies-verification-wip.json fingerprints/removes four audits.


## Roll menu and process signals

`hud_dialogs_loading.h` supplies004B0750/004B0900 and004BF2E0/004BF480.
Roll's actual RET12 includes a third BYTE argument copied to31; decompiler
omitted it. Borrow actual HUD identity/scene, find rollmenu/FFFF, copy all FOUR
raw local B0 position words without matrix update, find kurzor/FFFF, hide root,
allocate shared text/default inset10 and bind geometry. Then88=0,31=argument,
30=0. Cursor may be absent; root is required. No roll population is invented.

Signals borrows HUD/scene, finds signals/FFFF, hides only if present, sets0C=50,
allocates/binds text (null visual does nothing), clears60/64/68 and sets6C=1.
LAST publishes original process table0089BBB8: editor0083B610 if mode&7==7,
ordinary0083B980 otherwise. Missing root still allocates text/selects table but
returns false. Both closes CLEAR text records before releasing text/font owners;
active process table survives close. Borrowers discard before HUD/scene release.

`HudSignalTables` requires one shared process owner across HUD lifetimes. Exact
immutable11x20 DWORD tables each occupy0x370 bytes;004BFFE0 copies rows at50-byte
strides. All440 DWORDs recovered from fingerprint-guarded PE section bytes, with
binary/JSON evidence.0089BBB8 starts null in verified PE virtual zero-fill tail,
not file-backed bytes. Retained raw values carry no invented command semantics.

Focused native ASan/UBSan compares all440 source DWORDs with original binary,
verifies table selection/persistence/publication-last, real TextState/geometry,
shared fonts and clear-before-font-release, roll third BYTE/four position lanes,
missing signals false and required-artwork/visibility/factory/geometry failures.
Host Frames/GameOwnerBackend/Hud/text composition compiles in strict i386 and
application compile/link passes; no Windows execution. Four temporary audits
fingerprinted/removed by hud-dialogs-verification-wip.json. Full startup/render,
world/readiness/desktop and ledger remain unchanged.


## Hands/shoulder and sixteen objective icon rows

`hud_material_lookup.h` supplies004A89E0/004A8A90 (visualECX/nameEDX), reverse
LOD0 group iteration and original007E88F0 ASCII case equality. Texture lookup
has required material/texture reads and no visual prepare; replacement lookup
prepares visualA0 and skips null material/texture. Preserve repeated LOD/group
and lazy texture reads, invoke matching indexed GROUP18, then return the material
from a repeated lookup AFTER replacement. No substring match or independent
material owner is substituted. Exact X_SQUARE.BMP recovered at0083AC9C.

`hud_hands_loading.h` supplies004A9010/004A92F0: find tabl02/FFFF then child
hands/shoulder literals i_hands/i_shoulder/FFFF. Find original hands texture,
increment its32-bit14 reference (not frame16-bitE4), then replace/borrow matching
hands and shoulder materials. Allocate SAME font/text, inset0 BEFORE root
geometry, append four separate empty shared records/styles36..39, five words28
through38=-1 and item pointers1C/20=null. Close releases retained texture FIRST,
then CLEAR text records before deleting text/font. Borrows discard before scene
release. Runtime item/text/material producers004A9320+ remain required.

`hud_objective_icons_loading.h` supplies004AD8F0/004AD9B0 and004AD9A0 RET.
Root mpobjectives mask20 is optional in this binder. All16 rows find
mpallied_obj%02d/mpaxis_obj%02d/mpneutral_obj%02d through SCENE7C/mask1.
Selection-1 hides allied/axis/neutral THEN updates allthree in that order. Later
selection1/0/2 shows allied/axis/neutral respectively, others hide all; preserve
updates even when visibility unchanged. Missing required icons reject native
null call; out-of-range indices are guarded. This unconditional HUD artwork
loading does not invent multiplayer objectives or network state.

Focused native ASan/UBSan composes actual TextState/geometry, reverse lookups,
null skips/case equality/repeated lookup/group replacement, texture retain/release
and text-clear/font order, four records/scalars, five endpoint and factory/artwork
failures; all48 icons/lookup masks/hide-then-update/selection/RET/missing root,
missing icon/update failure. Host actual ResourceOwner/Frames/shared font/text
composition and application compile/link pass. Native engine/font are fixtures;
Windows fixture is compile-only. Four audits removed by
hud-hands-objectives-verification-wip.json. Full HUD/world/render/readiness/
desktop/ledger remains unchanged.


## Six soldier panel source receivers

`hud_soldier_loading.h` supplies004B30E0/004B3840 and selection004B3E70.
Containing0049B280 binds indices5..0 and must compose six owners, sharing SAME
first-state20 and second-state40 texture tables (native60/64 borrowers). Store
scene/index/Human74=null, find Soldier%02d/FFFF, tagEC=index+100. Child lookups
use10001. OR visual1D0 with1000 for player0*/ear*/select* (hide each), photo*
(prepareA0, first GROUP18, repeat LOD0/first-material read), healthbar*/healthcr*/
health0*, stamina0* THEN staminabar* both into24, disguise0* THEN disguisebar*
both into28. Preserve both overwritten visuals' flag updates. Ordinary mode
(mode&7!=7) sets disguisebar XYZ scale1/update; editor skips this operation.
Fourth scale lane comes from an unwritten stack word; preserve known padding.

command*/mind* each flag, GROUP18 then repeat LOD/first-material read. Scene
univerz/FFFF hides before borrowing first lazy texture. Load actual graphics
x_fc_death.bmp/x_fc_a.bmp flags4; native ignores status, so actual returned
texture determines owner, with a defined missing-owner guard instead of eventual
null close. No synthetic texture or claimed actual runtime acquisition. Allocate
primary text, inset0 before root geometry; visibility* flag then second text,
inset0 before geometry, hide visibility, selection0 and hide root. Optional
AImode* gets flags only if present; set98=-1,A0 XYZ1, preserve unwritten fourth
lane. silence*/medic* remain nullable borrowers; mp_vip* hides if present;8C=0.

004B3E70 compares cached BYTE89, equal returns, otherwise stores BYTE BEFORE
visibility call. Cold/unwritten selected and colour lanes have explicit zero
guards without constructor credit.004B3840 releases death texture FIRST, then
primary text THEN visibility text WITHOUT preceding record clear, nulls both
and root. This differs from hands/dialog explicit-clear retirement. Failure
cleanup uses these owned releases; scene/material/table borrowers discard before
containing scene/artwork destruction. Human binding/photo/health/runtime text
updates004B3890+ remain mandatory, unimplemented consumers.

Focused ASan/UBSan checks allsix source owners/shared arrays/index/tag/masks,
photo order, flags and overwritten slots, ordinary/editor scale/known lanes,
actual TextState geometry/font ownership and record-sensitive close order,
selection equality/store-before-failing-endpoint, optional late frames, six
endpoint failures, first/second allocation failure, missing required texture/
artwork and index guard. Engine/texture/font endpoints fixtures. Actual host
ResourceOwner/graphics/frame/group/font/text composition compiles in strict i386;
application compiles/links. Four temporary audits removed by
hud-soldier-verification-wip.json. No containing HUD/world/frame/readiness/
desktop or ledger credit.


## Compass artwork and cold ring owner

`hud_compass_loading.h` supplies00493F80/00494140 with required00493B30 ring
construction and source reverse00493BC0/00493B10 owned retirement. Find kompas,
kmpscvr,strelka/FFFF; ring finds kul_us/kul_en/FFFF and hides each if present.
Native ring BYTE08 reads low BYTE of the scene stack argument (dummy allocator
storage), preserved as raw byte without invented flag semantics. Sequence0C/
10/14 starts empty. Constructor exposes no fabricated marker producer.

Zero16 words60..9C, then eight alternating child kmps_bl%02d/kmps_rd%02d/FFFF
lookups/hides. Root and children are required; cover/arrow and ring art may null.
Zero four scalar lanes00..0C, publish shared PROCESS0089BB4C=1 then48=0 BEFORE
final root update. Keep one shared owner across HUD lifetimes. Close deletes
ring (records reverse, each actual frame owner releases before record deletion)
then clears compass root only; globals persist. Explicit remaining borrower
clear precedes scene release. Runtime ring population004936C0/00493D10 and
compass simulation remain required/unexposed; populated-ring execution untested.

Focused ASan/UBSan checks exact names/masks/order/16markers/four scalars/16zero
words/allocator stack alias, optional art, shared publication-before-update and
persistence, duplicate guard, five endpoint and three missing-artwork/null-scene
failures with owned ring cleanup. Host actual frame-owner/frame calls compile
in strict i386; application compiles/links. Four temporary audits fingerprinted/
removed by hud-compass-verification-wip.json. No live HUD/world/render/readiness/
desktop/ledger credit.


## Message font/artwork and screen bounds

`hud_messages_loading.h` supplies00495F40/00495900/004960F0 and source
004950C0 queue cleanup. Font acquisition uses SAME GameOwnerBackend weak font
cache; extracted make_hud_fonts retains existing text allocation behavior. Find
literal Text/mask1, store2C argument(50 at0049B280), zeroD37C/D36C/68/168/30/
34/38,3C=150,D384=1. Read signed32 graphics HEIGHT then WIDTH, float-store44/40.
Save actual visual world matrix70 four words, use SAME saved X/Z for both local
XYZ writes(Y=FLOAT.18 thenFLOAT-.3), preserving native positionBC. Position skips
when frameE0&80; each write followed by update then00495900. Hide/update after
second bounds, finally10/D385/D387/D386=0. Decompiler lost saved X/Z registers;
assembly is authoritative. No return-to-original-position is invented.

00495900 null visual leaves all outputs unchanged. E0&100 clear invokes visual58,
copy12 raw DWORDs at10 (distinct from00644A80 world bounds170), add/subtract
FLOAT.005 on minXY/maxXY, then multiply X by storedwidth and Y by storedheight
andFLOAT-4/3, preserving intermediate float stores. Negative right edge wraps
to WIDTH; no upper-right clamp. Bottom clamps only aboveheight/below0, keeping
NaN/zero. Ordered x87 comparisons preserve NaN; do not apply <= tests.

004960F0 releases font FIRST, then004950C0 frees queued raw strings in reverse
while retaining vector capacity, resetsD37C and conditional remaining font,
then nulls visual. Current source queue remains cold/private until actual message
producers and glyph/layout consumers are recovered; populated retirement is
unverified. No fabricated queued messages/rendered text are supplied.

Focused ASan/UBSan checks signed32/float dimensions/height-first calls, both saved
position layouts, exact bounds/update/hide order, asymmetric clamps/zero/NaN/Inf,
null output preservation/font cleanup/duplicate guard and eight endpoint/font/
artwork/scene failures. Native engine/font endpoints fixtures; actual host font
factory/graphics/frame/geometry composition compiles strict i386; application
compiles/links. Four audits removed by hud-messages-verification-wip.json. No
full HUD/world/render/readiness/desktop or ledger credit.


## Crosshair source loading, visibility and colour

`hud_crosshair_loading.h` supplies004A9740/004A9F00/004A9F10/004AA760.
Store30/34=0, find cross/FFFF, native direct XYZ(.5,-.375,FLOAT-.14) with80
position guard, update, then children cross_L/R/T/B mask1. Each arm:first GROUP18,
repeated indexed LOD0/group0 material flag08|08000000, material0C zero,10 zero,
08 one,14 FLOAT0, visualA4 FLOAT.6. Repeat LOD/group lookup per operation.
Allocate SAME shared text/default inset10, child cross_name/mask1; if present,
scaleXYZ2/native quat+flags, bind geometry then hide name; else bind root geometry.
Native material/scale fourth stack lanes are unwritten; explicit zero/known-lane
preservation guards carry no constructor credit. Set14=0/update root,show1,
zero cached colourXYZ then request white. No runtime target/name/distance producer.

004A9F10 stores requested BYTE35 before querying REQUIRED actual006B66D0 only
when nonzero, forces0 if preference false, then shows/updates root with no equality
shortcut. Native getter reads current ordinary application24 manager274!=0 or
editor mode7 application20 profile73; real manager mapping remains mandatory,
not a default enabled preference. Strict i386 uses a declared unresolved settings
endpoint, actual host frame/material/font/geometry calls. This explicit source
boundary does not establish integrated preference lookup.

004AA760 gates on rootE0 visibility bit1, compares cached XYZ only (fourth input
ignored for equality), NaN differs, caches allfour input words BEFORE T/B/R/L
first-group material0C calls, then root update. Partial receiver failure leaves
new cached colour; do not add rollback/equality retry. Hidden root leaves cache
unchanged.004A9F00 deletes text WITHOUT preceding explicit record clear; borrowed
frames discard before scene teardown.

Focused native ASan/UBSan checks exact four-arm/material call order, native
TextState/geometry/name fallback/default inset, preference/visibility and colour
gates, equality/NaN/cache-before-failure/fourth-word behavior, record-sensitive
text/font destruction, eight endpoint/factory/artwork failures. Engine/font/
settings endpoints fixtures. Actual host i386 and application compile/link pass;
Windows not executed. Four temporary audits removed by
hud-crosshair-verification-wip.json. No actual006B66D0 manager mapping/full HUD/
world/render/readiness/desktop/ledger credit.


## Scoreboard and explosive timer initialization

`hud_scoreboard_loading.h` supplies004AE7C0/004AED00. Find mp_scoretable/FFFF,
allocate SAME text/inset0 BEFORE geometry. Mode7 skips manager224 and stores-1;
otherwise current manager224 style0 maps-1. Style1 appends0/style43,0/44,00/45,
00:00/46 then shows root. Style2 appends0/43,0/44, optional00/45 ONLY when
manager240!=0,00:00/46, with NO root visibility call. A missing root is genuinely
tolerated by style2 binder geometry. Style3 appends99/99/44,00/45,00:00/46,
finds children mpallied_score/mpaxis_score/FFFF, copies axis raw local XYZ into
allied with native80 write guard/preservingBC, hides/updates axis then updates
allied. Other styles hide root. Close deletes text without prior record clear.
Current actual manager mappings are supplied by the configuration data below;
earlier isolated strict fixture declared these endpoints before that recovery.

`hud_explosive_timers_loading.h` supplies004AF630/004AF8B0 against actual
GameActorServices::Actor type18 getter and explicit borrowed0089B3B0 registry.
Find explosivetimer/FFFF/store scene60, append every nonnull actor type22 in
REVERSE registry order, preserving duplicates and accepting more than10 actors.
004B04A0 is vector insertion, not an explosive virtual service. Native actor
owners must outlive this borrower list. Find10 root children timer_explosive%02d/
mask1, allocate SAME text, bind geometry with DEFAULT inset10 THEN set inset0,
append empty/style48,hide. This ordering differs from scoreboard. Reserve at
least10 separate8-byte layout-record capacity after allslots; no fabricated
records/active countdowns are created. Close clears actor list retaining capacity,
then slots0..9: clear frame borrower/delete text, then5C=0. Layout capacity and
root/scene remain until explicit borrower discard/native containing destruction.
Partial failure closes all owned text; rebind requires prior cleanup.

Focused ASan/UBSan checks every scoreboard style/team branch/editor bypass,
literal records/missing-root style2/visibility and axis positioning/update order;
timers reverse type selection/duplicates/null/>10 actors, exact ten names/masks,
inset timing/geometry/style/capacity, partial late-slot cleanup, six endpoint/
configuration/allocation/artwork failures. Native font/frame/config/actor-type
endpoints fixtures; actual host i386 uses real ActorBase::type/Frames/font/text,
with initially declared config resolved by subsequent actual composition below.
Application compile/link pass. Four audits removed by
hud-score-timers-verification-wip.json. No active world actor registry/countdown/
HUD rendering/readiness/desktop/ledger credit.

## Shared application configuration data and actual HUD getters

`frontend/game_configuration.h` recovers application+24 configuration DATA from
006B4A60's actual0x298 allocation/006AD350 reset. This supplies data ownership,
not successful network/transport/manager activation. Existing MultiplayerServer
00724EE0/00724EF0 subobject17C is reused: resets preserve host/address/version.
Store04=0, clear218, copy TWELVE literal ASCII bytes `\x01\x02\x03` at00842ADC
to21C (not three control characters),170/174/178=0; zero360 bytes08..16F.
Port220=11001,style224=0 and exact22 rules228..27C are:
32,20,0,3,30,10,1,1,1,0,1,100,1,1,1,1,0,3,-1,1,0,1.
These differ from00618A00's multiplayer MENU presets and cannot use those defaults.
Rotation290=0, reverse descriptor deletion/retain capacity.0061B440 releases
string44, raw table38,raw table28,string05,string01. Typed descriptor lifetime
exists; unknown metadata/activation producers are not exposed as recovered data.
Data close reverse-clears rotations/index-1 then C++ destroys strings21C/218;
294 runtime producer and transport lifecycle remain absent, without fake success.

ApplicationStartupData owns ONE configuration data instance after HumanStore,
releases it after its previously owned four components per006B6010. StartupSession
and GameOwnerBackend resolve the SAME current instance. HUD style224 and teams240
now read actual style/rules[6].006B66D0 actual getter resolves current manager
274=rules[19]!=0 in ordinary mode; mode&7==7 returns current profile native73
BYTE unchanged (ProfileSettings start38→bytes3B). No menu-local copy/default
preference, stale profile pointer or declared successful getter remains. Strict
i386 now composes crosshair and scoreboard through actual GameOwnerBackend/
StartupSession/ApplicationStartupData/configuration/profile source, replacing
the earlier unresolved fixture endpoints. Overall HUD/world remains uninitialized.
Multiplayer menu/connection→manager configuration producers are still separate
required integration before active multiplayer/world use; no transport claim.

Focused native ASan/UBSan compares ALL22 rules with original assembly-derived
binary, distinguishes menu presets, checks literal sentinel/port/styles/identity,
server reset preservation, reverse descriptor/string release/owned table cleanup/
capacity, reset/clear/repeat and all16 flag modes/current preference changes.
Actual i386 HUD/crosshair/scoreboard/config/profile/host composition and application
compile/link pass. Four audits removed by hud-configuration-verification-wip.json.
No original/Windows/desktop execution, loading readiness or ledger credit.


## Inventory scrollbar loading and geometry

`hud_scroll_loading.h` supplies004AE2E0 with004ADA70/004ADA20/004ADBA0,
004ADC90/004ADC50/004ADDD0/004AE0F0/004AE290/004AE790 and geometry close004AE2B0.
Main RET12 args are inventory identity,root,tag. Tag rootEC, store callback,
lookup barrow up*/barrow down*/slider* mask10020; nested callbacks point to SAME
scroll receiver and tags OR1/2/3. Button child normal*/actived*/inactived* and
slider normal*/actived*/sliderbase* use10001. Copy normal local XYZ into other
states with80 guard/preserveBC. Button tags allthree state frames; slider tags
normal/active, hides/updates base. State helpers perform ALL visibility calls
then ALL frame updates, preserving partial failure order.

Initial004AE790(1) is DISABLED: button normal/active hide,inactive shows; slider
normal/active both hide. Do not call this an enabled widget. Flags stored before
receivers. Missing child states returnfalse from inner binders, ignored by outer;
missing required state later rejects during disabled-state calls. Missing ONLY
sliderbase genuinely leaves normal/active bound with no geometry and outer bind
can finish; preserve that original ignored-false path without claiming runtime
slider readiness. No click/drag callbacks are fabricated.

Successful slider allocates seven zero floats(native20), reads normal then base
E0&100/virtual58 frame10 eight-word bounds. Half extents float-store, then four
bounds float-store, subtract normal localX from X and localZ (NOT Y) from Y,
storeheight14 thenwidth10. Vertical iff storedheight>width; NaN/tie selects
horizontal. Setpercent50 via004AE0F0. That helper reads root WORLD position FIRST,
then compares cached float equality. Changed value caches/clamps0..100 preserving
NaN, computes native FLOAT.01/extended interpolation and clips before float-store
into world-position copy; calls actual frame08/update. Horizontal modifiesX,
verticalY, preserves other world lanes. Equality still queries world but makes no
position/update calls. Native constants retained in hud-scroll-constants.json.
Main sets80/84/88=0,8C=-1,90=1 after state calls. Geometry close frees only table;
frames/callback remain borrowers until explicit discard before scene release.

Focused ASan/UBSan covers exact identities/tags/masks, copy/visibility/update
order, disabled states, both geometry axes/float offsets/localZ, cache/clamps/
NaN/store-before-failure, missing-base genuine path, missing states and five
endpoint cleanup failures. Actual host i386 frame/bounds/tag/position calls and
application compile/link pass. Four temporary audits removed by
hud-scroll-verification-wip.json. These typed source owners are composed by
inventory below; raw HudOwner tracks remain cold until containing HUD runtime
composition. No runtime input/initialized whole HUD/world/render/desktop/ledger.

## Inventory artwork, same inline texts and scene drag ownership

`hud_inventory_loading.h` supplies complete loading body004A0EE0 and matching
004A1DA0 RET. Borrow scene/control-context2DF0; context is opaque here, no invented
texture-table semantics. Scene inventory/mask1 and items00/items01/mask20; root
frame08 XYZ(FLOAT.178,FLOAT-.333,0),hide/update. Bind SAME67 original inline
TextState owners from HudOwner's77, locating native HUD+30 offsets; never allocate
replacement texts/fonts. Ten other cold text owners are untouched by this body.

Six rows use exact bck*/head*/drop*/hand*/uniform*/shoulder*, caption/selection
names retained in original-string JSON. For EACH side: replacement material
004A8A90 X_SQUARE.BMP, value text geometry(defaultinset10), mesh18, caption hide,
captioninset0 BEFORE geometry/localizedA96+row/style0B/index0, then hide selection.
OffsetsF0+row*290+side*148 primary and+A8 value. Tags200+row/210+row occur after
right caption and before right selection. Preserve side/order/native masks10001.

Four capacity blocks1050+section*3E4 (left/right holsters,left/right backpack):
captioninset0/geometry/localizedA9C orA9D/style0B/index0/hide, value+344 inset0/
geometry/hide, four item visuals and capacity/selection artwork. Asymmetric
hlcpct00 mask10001,hlcpct02/01/03 mask1 is retained. Per itemindex0..3, visit all
four blocks: mesh18,inline default-inset text at+C0+index*9C, replacement material;
THEN tags220/230/240/250+index. Borrow original texture via004A89E0 from first
left body visual without retain. Bind namexx00/01 and text_allcpct00/01 to inline
2B80/2C1C/2CB8/2D54(defaultinset),hide capacity texts; find/hide description.

Create real driver50 subtypeOBJ_(5F4A424F), duplicate first body visual via4C,
normalize actual source WORLD forward60 four words, set clone0C direction/flags0,
visual1000/nameDraggingFrame/prepareA0/mesh18/link actual HUD scene1F8/hide/update,
replacement material. Scene80 retains, THEN local clone reference releases at
004A1AD8; object/material remain scene borrowers. Normalization preserves native
extended sum versus stored float, tiny-vector ONE-component substitution, NaN
arithmetic and fourth lane. No invented finite/zero-direction success default.

StoreC4/2DF4/2DF8=0; scene skills/mask20 remains nullable. Native1FE4 text binds
text_soldier stats/1 at DEFAULT inset10, localizedA6B/style0B/index0/hide. Seven
exact skill triples (retain original typo strenght and spaces first aid) bind
caption2080+i*9C inset0/geometry/localizedA6E+i/style0B/index0/hide, bar borrower,
value24E0+i*9C inset0/geometry/hide. Find root scroll13..16/mask1, present roots
bind actual scroll source above with SAME inventory identity/tagsF10/F20/F30/F40;
2E10=0 last. Original004A1DA0 does NOTHING; it does not clear texts, destroy
scrolls or unlink drag. Typed inline scroll owners retire with containing cold
HUD lifetime; explicit helper is ownership composition, no new destructor-address
credit. Scene owns drag; discard all borrowers before its release. Runtime
inventory/item/selection/drag handlers remain separate unimplemented consumers.

Focused ASan/UBSan composes actual HudOwner77/TextState/material/scroll source,
verifies67 existing geometry-bound texts/no extra font allocation,24 localized
records/inset distinctions,12+16 frame tags, four geometry owners, scene-retain/
local-release order, real RET close preserving ownership, direction tiny/zero/
NaN/fourth lane and eight early/late endpoint/null-factory cleanup failures.
Factory/frame/localizer endpoints fixtures. Strict actual host i386 composes
GameOwnerBackend/Hud/Frames/engine factory, shared texts/materials/scrolls and
application compile/link pass. Four temporary audits removed by
hud-inventory-verification-wip.json. Full HUD scene/renderer/session/world,
readiness/desktop and ledger remain unchanged.

## Info model and shared process style registry — 2026-11-08

`hud_info_loading.h` recovers00496450/00496DC0 loading/retirement. The initial
00499F10 is generic pointer-copy with equal begin/end, followed by RET8
004207B0; this clears the cold borrowed list, not a font factory. Loading saves
**boolean** driver15/19, disables both, creates kind9, uses the SAME actual
category1 LoadedResources cache008AEC90 with `ingame_info`, flags0. Successful
model gets that name, links actual HUDscene1F8 and Scene80 retains it. Find
`game_info` mask1 and set(.5,-.375,-.5), then restore both driver booleans.
Native guards restore on factory/load/missing-root exceptions too, refusing
success after a required cache failure rather than reproducing native invalid
root access. Local model reference releases **last**, after geometry/styles/hide;
scene owns publication and must outlive borrowers.

Mode7 gives kind0; ordinary config224=0..3 gives1..4; unknown values retain
previous kind. ScaleXYZ(.7,1,1) for1, (.7,.7,1) for4, otherwise(1,1,1), retaining
known native DC guard for the unwritten fourth stack lane. Actual quaternion/
E0 invalidation/update, optional model children `header`/`text` mask1 hide,
shared TextState default inset10 and actual00644A80 geometry follow. Register
all73 exact52-byte descriptors008399E8..0083A8BC, then00496E40(false) hides
root without reaching00496E70 population. Word140 is zero before model work.
00496DC0 unregisters all73 in ascending order, writes each handle-1, THEN
releases text without an explicit record-clear. Borrowers discard separately;
there is no model-unlink call in this close.

006438E0/006438D0 are wrappers around **process008AE2E8**00641D90/00641CC0,
not per-TextState palettes. The existing StartupSession OpeningTextStyles now
owns that same full registry: all90 builtins from independently anchored
00641E30/00643790 contracts, raw13DWORD descriptors, first-null-hole insertion,
signed negative/out-of-range removal false, already-null in-range removal true,
trailing-null trim retaining slot0. Movie's14 initial zero handles remove style0
once; info insertion subsequently reuses0 before appended90..161; no second HUD
style owner. Native allocation failures throw instead of publishing null styles.
Lazy builtin initialization rolls back on allocation failure. Builtin padding2E/
2F has a defined zero native guard (original unwritten); static HUD descriptors
preserve every original bit. Correct original builtin0 height=.04, whereas the
previous bounded movie source supplied .03 for unused style0; reached movie8
height remains .035. No glyph/layout/rendering completeness is claimed.

Evidence: hud-info-bind.asm; hud-style-registry.asm;
hud-style-descriptor-constructor.asm; successful hud-info-factory-style-close/
(no export ERROR); hud-info-style-data-evidence.json and all3796 original bytes
hud-info-styles.bin. Temporary focused ASan/UBSan verifies90/73 descriptors,
movie-hole reuse, removal/trim bounds, all five kinds, actual scene/local
ownership distinction, default text geometry, repeated cleanup, five early/late
endpoint failures plus partial style allocation failure. Strict i386 instantiates
actual GameActorFrames/GameOwnerBackend/StartupSession registry/config receivers;
application compiles/links. See hud-info-verification-wip.json for fingerprints
and removed audits. No original/Windows execution, initialized containing HUD,
info population/rendering, active world/readiness/first frame/desktop or function
ledger credit. Full P2.2 remains open; map/command and containing HUD follow.

## Command-panel loading and retirement — 2026-11-08

`hud_command_loading.h` implements004B4980/004B4DE0/004B5790 and matching
004B4D40/RET004B5780/004B59C0 loading-source contracts. It borrows actual
HUDscene/native04, worldscene08 and collision5A7B4; five explicit opaque arguments
preserve native0C/60/64/40/44 identities. BorderFFFF is required for update,
hidden before world capture; native10/14/18/1C zero. All18 normal/alternate
button pairs are recovered in original call order, including aggresive's typo,
15 before14 and wait4/5 after14. Each has identical EC tag1..18, alternate hide,
normal WORLD70 four-word position copied through realframe08 to alternate,
then004A8BD0(show,false) shows normal/inverse alternate without global-cursor
call or extra updates. Initially enabled1..5/17, other normals hidden.740/744zero.
Five TMcamera1x/2x/4x/8x/16x icons bind;90=1 then004BF130 advances to2,
shows icon2 only and sets speed1. Assembly resolves decompiler's lost stack:
0089AEA0 vtable0080FDD0+88→004650A0 returns SAME camera+0C detached controller;
FLOAT stays on stack across RET0;00462750 writes detached40 only. MissionCamera
now preserves this actual detached speed, constructor1, without activation or
simulation. No engine-camera virtual88 call is fabricated.

Reuse SAME eight HudOwner inline TextStates10BC8+i9C (commandA8+i9C), all
inset0 BEFORE actualgeometry; captions1C84/1C8E/1C98/1CA2/1CAC/1CB6/1CC2/
1CC0, style47/index0, corresponding text_* hide. This adds **no replacement
inline font/text owners**. Ten info%02d tags20..29 follow. Kind9/cache1 loads
l_aicomm withflags0; original intentionally tolerates cache failure, releases
local attemptedmodel and storesnull748. Successfulmodel retained by COMMAND,
not added to scene. Nested004B0750 uses explicitNULL HUD and flag1; shared
HudDialogsLoading now allows that actual null identity. It still makes its one
additional real shared-font text with defaultinset10. Zero5C; six routebuffer
count pairsC75C+iF008/+3004, flags5A7B1/BD/B8/B9/BB/BC. Keep unwritten
large route contents absent, no successful producer. Ten texture borrowers use
lie/kneel/stay/slow/walk/run/sprint/passive/defensive/aggresiveFFFF, oneactual
LOD0 each, firstGROUPmaterial, lazytexture resolution as needed. No retains.
Finally54/5A7B0zero.

004B4D40 FIRST selects actual input mode2 via006B7400→006A8690. New shared
StartupSession endpoint uses SAME current profile BindingConfiguration/input
manager and original reverse installation before mode store; no private mode
field or fake success. RET004B5780, then004B59C0 clears inline indices
**0/1/2/3/4/5/7**. Index6(camera-motion) is genuinely retained, not options7;
saved assembly preserves exact seven ECX offsets. Release748, reverse-release
cold route-model vector750/754 preservingcapacity, close nestedroll(clear then
text/font release). Explicit discard of borrowers occurs before scene retirement.
No original runtime command producer, route simulation, click or rendering is
exposed. Unwinding reclaims local ownership; explicitclose uses CleanupErrors
so an input installer exception cannot bypass resource/roll cleanup.

Evidence: hud-map-command-dependencies/,hud-map-command-records/,
hud-command-camera-close/ (successful exports/noERROR), saved
hud-command-bind-close/buttons/button-visibility/text-retirement/detached-speed/
input-mode.asm, actual strings hud-command-names.json, and vtable bytes
hud-command-camera-evidence.json. Focused temporary native ASan/UBSan uses
actual HudOwner77/TextState/HudDialogs/MissionCamera sources with engine/font/
localization/input endpoints fixtures:18 pairs, five speed icons, sharedcamera,
eight texts, ten textures, optionalmodelerror, exact retainedcaption, repeat
and seven partial-failure stages. Strictactual i386 and application compile/link
pass. Removed audit fingerprints in hud-command-verification-wip.json. Source
only, uncomposed wholeHUD, no readiness/desktop/new function-ledger credit.

## Map loading/record source — 2026-11-08

`hud_map_loading.h` recovers004A4AE0/004A4F60 and loading-only record helpers.
This remains **uncomposed**: actual missionA8 objective-vector and type37 zoneA4
producers are required world integration, not successful empty services. Native
fixtures provide their declared data endpoints; i386 verifies actual ActorBase/
frame/cache/text/auxiliary source with explicitly **declaration-only** World
endpoints (not linked or run). No actual zone/objective producer claim.

Create kind9 before querying boolean driver15/19/11; set0/1/0, **DIRECT model74**
`missions\\<selected mission>\\map` flags0/null/null, then restore all3 booleans.
Native guards also restore on load exception. Nonzero result clearscenter/map,
releasesmodel and returnsfalse.0049B280 ignores this actualfalse return, so an
absent mission map is legitimately tolerated; it is not an initialized map.
No category1 cache call/fallback. On success findcenterFFFF, map*10001; present
center links HUDscene1F8, frame08(.5,-.375,-1), update. Scale20starts0; optional
mapF0 firstsubstring **`scale`**, atof starting immediately after5characters,
floatstore. `scale:...` therefore yields0; no invented colon parser. Explicit
native path-length guard prevents original fixed-buffer overflow.

Find/borrow allied_active,vip,mapcamera,enemy,objectiveFFFF from HUDscene,
reparent each to actualmap(flags0),hide/update. Mode7 kind0/count6; ordinary
config2240/1/2/3 giveskind1/2/3/4 count0/32/32/6; unknown leavesprior values.
004A4FD0 ALWAYS looks upalliedFFFF, even count0.32slots get0flag; count active
slots eachcreate OBJ_5F4A424F,duplicateallied,linkmap,hide/update,allocate actual
shared-font TextState/defaultinset10 WITHOUT geometry. Own clone/text separately;
noScene80/add or extra clone retain. Inactive slots remaincold.

Kind2/3/4:004A72C0 optional selectzone1; ifpresent link center preservingworld
(flags1). Scan **actual registry starting32**, type37/groupA4==-1 only, retain
borrowed pointers in reserve20 temporary; **>=16 matches creates NO zones**.
For each matchingactor's actual frame shortname findmap childmask20. Present
zone creates24-byte record; root/actor/tag1000+candidateindex; zone_disabled*
10001required/tag, allied*/axis*10001optional/tag/hide/update;state10=2.
Kind3/4 also004A7520 iterates actualobjectivevector, seeksobjective%d starting1
mask20, appends record BEFORE004A7A90. Findtext_obj*/allied_obj*/axis_obj*10001;
missingfaction returnsfalse ignored, retains partialrecord. Ifbothpresent,
optionalcaption gets sharedtext/default10/geometry/HUDscene andhide; hideboth
factions/root;0C=4/10=2. Unwritten failed-record fields get definedzero guards,
no initialization-credit. Kind0 instead004A7680/004A7ED0: same actualobjectives,
requiresbothfactions, hidesboth/root,18-byte successfulrecord only,state0C=-1;
missingfaction frees attempted record/noappend, no text.

Mapdepth28 differs from006449D0: nullmap gives1, actualHUDcamera required,
clipsread first, **map WORLD update BEFORE camera WORLD update**; sameextended
quotient/range/NaN andFLOAT.1, no reordered helper use.004A5630(false,false):
only ifcenter exists,24=0,hidecenter/map; kindnot0/1 additionally actual
HudAuxiliary004ABAF0(-1). Its coldselectedindex-1 returnswithoutcalls; changed
selection hidescurrentfield00frame ifnonnull,stores-1,retainsfield00/no update.
Pointerfield00 is now a typed frame borrower, matching originalread/use.

004A4F60:004A5190 forwardalliedslots releaseVISUAL thenTEXT/flag0;
004A51D0(0)/004A5400(0) shrink privatecold dynamicmodels inreverse, retaining
capacity; kind2 zonefreeforward/selectzoneclear,kind3/4 then goalobjectiveclear/
textdelete/freeforward,kind0 editorobjectiveclear/freeforward. **Mapmodel LAST**.
Discardborrowers separately before scene. No dynamic enemy/objective producer,
map clicks/rendering or objective-state updates are exposed.

Evidence successful hud-map-command-dependencies/records/,hud-command-camera-close/;
hud-map-bind-clones.asm,hud-map-objective-record.asm,hud-map-pointer-selection.asm.
Focused ASan/UBSan fivekinds/0,6,32clones/texts, driverrestore/directpath/scale,
map-before-camera depth, ownerorder, reservedactor32/16zonecutoff, factionfailure
asymmetry, actual coldpointer, toleratedabsentmap/threeexceptionprefixes pass.
Stricti386/app compile/link pass; hud-map-verification-wip.json removes four
temporary audits. Source only; wholeHUD/world/readiness/desktop/ledger unchanged.

## Containing HUD loading source and shared panel composition — 2026-11-08

`hud_panels_loading.h` composes actual0049B730/0049B620/0049B280/0049B490
source classes, not successful callback placeholders. One HudOwner77, Factory/
font cache, processstyles/signaltables/compass, camera, Players/Items and World
are borrowed for the whole lifetime. Actions→playerstates→speed/cursor→signals/
roll→soldiers5..0→messages50→crosshair→hands→status→compass→worldpointer→map→
inventory→progress→info→selector→movies→hint→camera→objectives→scoreboard→voting→
timers→carnage→server/injurymessage→carnagevisibility→infoicons→command. Native
6B6B0 is the **just-bound carnage_stats root**, not a soldier mp_vip lookup:
show only(mode&7)==7 and editor kind3. Native command arguments retain SAME HUD,
first20state/action39 arrays, auxiliaryInfo owner and inventorycontext identities.
No separate copies of shared text/texture/global tables are made. Actual map
false return remains intentionally ignored; required missing artwork still fails.

Close follows0049B490: signals/roll, soldiers0..5, messages/crosshair/hands/status/
compass/map/infoicons, command, progress,pointer, **info styles/text**, selector,
movies,hint/camera/objectiveicons/scoreboard/voting/timers/carnage/server/injury.
004A1DA0/004A8DE0/004A8D50 are RETs, not releases of inline inventory text or
scroll geometry. Separate explicit containing retirement discards all borrowers
and reclaims inline typed scroll geometry before source reuse; no native cleanup
address is credited for that guard. CleanupErrors preserves required release
sweep after command input/progress/movie endpoint failure. A combined new-boundary
check found/fixed a missing info-close call in this new composition before it was
accepted; previous individual-panel contracts remain unchanged.

`hud_scene_loading.h` recovers0049A940/0049B5E0/0049B1B0 around these concrete
sources. Existing flags&1 returns1 before any allocation. Create actualScene13,
fit physical viewport iff scene244/248/24C/250 differ. Model9 uses SAMEcache1
`gameinterface`, flags0; boolean driver15/19 save→0/1→restore. Negative cache
result rejects/cleans required artwork; no fallback. Model links HUDscene1F8,
frame08zeroXYZ, Scene80retains while HUD retains its original model reference.
Camera3 setactiveScene90; frame1C direction(0,0,1),flags0; direct position
(.5,-.375,-15.5)/80guard; range(.2,500); virtual88 integer1/84float2/update.
Light2: GI_Light/type4/colour84(1,1,1)/power90float1/flags9411; Scene80 AND root
sector74, THEN localrelease. Native unwritten fourth vectors have definedguards.

One explicitly passed process HudClickSound owner represents0089BB7C (must
outlive the HUD); overlapping owners reject rather than overwrite/leak. Create4
before sound-available check. If actual ISndDriver0C true, sharedcache2 loads
button.wav flags0 and native ignores status. Set7C3,visiblefalse,volume1,name
sound_click,link HUDroot. Actual sound-unavailable branch retains the owner and
still configures it, without inventing audio-file success. Bind all panels,
find down_panelFFFF,zero6B8F8/FC. flags9,10/14/18zero,20/21zero. SAME existing
inline debug TextState6B85C/defaultinset10 binds optional Border geometry and
appends exact `Camera pos: 0,0,0`,style4E; no replacement text/fonts.

0049F1C0 finalcompass policy uses actual panel borrowers: ordinarily showfalse;
mode7 only, flags&1A0zero, inventoryroot hidden, crosshairfield14!=1 and command
Border hidden, then SAME current Players receiver and005A78F0 action9/0 nonzero.
Player/action endpoint remains declaration-only in i386, actual production mapping
not invented.00494170 toleratesnullcompass and neverupdates;004B6180 tests actual
BorderE0visibility. Native fixture tests ordinary branch skips player lookup and
editor action path shares the provided receiver.0049B900 currently covers the
proven cold Human74/ring-record path (six soldier roots5..0 hide, flags clearbits
2/4); populated Human/compass producer/removal integration remains required.

Loaded close zero-flags returnsfalse. Otherwise hide soldiers,clearflagsbits2/4,
zero6B8F8/FC,clear SAMEdebugtext,0049B610 repeatszeroes,closepanels,releaseprocess
click,camera,model,Scene90null THENscene. Borrowers/99artwork table refs drop at
retirement, flags0. Typed destructor additionally reclaims any incomplete
acquisition prefix even before native flags9; no native success credit for guards.
Strict source ownership requires frames/factory/Hud/panels/process owners to
outlive the containing scene wrapper.

New composition evidence resolves0089B3B0/B4 to Scene2global0089B3A0+10/+14
**identity** slots, not active-vector semantics:007D8AB3/00475DD0 plus saved
hud-scene-identity-registry.asm. `HudActorIdentityView` reads actual
GameSceneActors→SceneActorRegistry count/identity on each access, maps Instance
back to SAME ActorBase, retains reserved32/holes/compacted tail and never copies
or pads a second registry. Map World now explicitly requests
hud_actor_identities(); timers accept this view. Focused native check uses actual
SceneActorRegistry to distinguish34identities/oneactive, reservedslot removal and
tail compaction. Stricti386 instantiates actual GameSceneActors view inside every
panel/containing-source consumer. Earlier map receipt's declaration named
active_actors was a source contract naming error; no such active-list binding was
ever integrated. Corrected in this composition receipt, with new direct evidence.

Native ASan/UBSan composes ALL production panel/Hud77/TextState/MissionCamera/
style source bodies; verifies actualsharedfont/global/source identities, movies,
scene-retained models, postbind and repeat cleanup. Separate containing audit
uses the SAME composed panels to check idempotent/full load and twiceclose,
requiredmodelerror, optionalnegativeclicksounderror/unavailable-sound branch,
inline debug, earlyinventory/latecommand unwind. Engine/resource/localization/
World/player endpoints remain fixtures. Strictactuali386 frame/cache/font/input/
config/camera/identity source compile and application link pass. Worldscene/
missionA8 objectives/type37zoneA4/currentplayer/action9 mappings remain explicitly
declared; no runtime implementation supplied. Receipts hud-panels-verification-
wip.json and hud-scene-verification-wip.json fingerprint/remove four audits each.

This completes a **source-only containing HUD startup/lifetime composition**,
not an activated session or renderer. It is not called by GameApplicationState;
loading_ready staysfalse; session Human98/A0/A8/B0/Overlay.words40 staycold.
Actual initialized world, populated HUD/compass/message/selector producers,
0049B9B0 prepass/0049B9F0 rendering and general text/glyph consumers, full006989B0
activation and firstworldframe remain P2.2 work. No desktop/original/Windows
execution or new frozen-function-ledger credit.

### Same actor/movie/frame backend for production composition

`GameSceneActors` now derives from existing GameMeshMovieServices instead of
parallel GameActorServices. That movie-services class already derives from
GameActorServices, so the actual scene backend has ONE entry/context/frame-data
owner and actual HUD movie endpoints together. This supplies the concrete
production type for HudPanels/HudScene without constructing a second actor
services registry. Existing ActorBase/Car/Human references still bind the SAME
GameActorServices subobject. Panel Actor is deduced from the actual identity
view's base pointer; timers accepts an explicit ActorType, because
GameSceneActors::Actor means Instance* rather than its ActorBase type. No new
actor factory or gameplay method is supplied by this inheritance change.

Focused stricti386 instantiates ALL containing/panel/camera/cache/font/input/
identity calls with **GameSceneActors itself** as frames, asserts single
convertible ActorServices base and correct ActorBase inference. World/player/
objective/zone action endpoints still declaration-only. Application compile/link
passes. No repeated unchanged native-panel audits: this final follow-up only
changes endpoint availability/type inference. Receipt hud-shared-actor-
verification-wip.json fingerprints/removes two compile fixtures. No execution,
production-world startup or readiness claim.

## Actual host process-owner binding and HUD console gate

`host/game_hud_loading.h` now binds actual GameSceneActors, GameOwnerBackend,
HudOwner and StartupSession's SAME process style/signal/compass/click owners.
Panel members outlive scene cleanup. This concrete host source wrapper requires
real Players/Items/Camera/World inputs; it neither creates a new actor services
owner nor activates a world. Stricti386 instantiates all bind/close bodies with
actual host/font/frame/cache/input/current-configuration/proxy receivers. World/
Player producers remain declarations. The wrapper is not installed into active
GameApplicationState. Compile-only verification/fingerprinted two removed fixtures
in game-hud-verification-wip.json; application link passes.

0049B9F0's second draw gate is **00720B80(10), integer console variable no_gi**,
not network/cinematic/effects state. Direct asm0049BA12 setsECX10. The table is
ELEVEN24-byte records starting **00844468**, not00844470: name00/word04/type08/
variableBYTE0C/domain10/currentvalue14. Getter compares actualvariableBYTE and
integer type1, returns currentvalue else-1. All11 exact original records/strings
and zero initial values are in hud-console-variables.json/.bin. Actualvariables
5hydroshock/6sniper/10no_gi have type1,flag1,literal domain`0,1`; type2 entries
say/sayteam/kill/giveitem are COMMANDS, not stringvariables.0071FE70's stringvar
allocation loop is therefore genuinely empty for this frozen table. Native
console dictionary/command parsing/dispatch/effects remain absent, not credited.

`game/console_variables.h` owns that process DATA, full00720B80 getter and the
three literal0/1 variable-store branches of00720A80→00721B90. Wrong domains return5
without mutation; a public nonvariable write rejects instead of exposing generic
command dispatch. The wider domain lexer and RawSet command branches are not
claimed. One StartupSession ConsoleVariables persists across missions; backend
hud_no_gi reads currentindex10 from SAMEowner (no constant-false visibility stub).
MissionCamera.hud_suppressed reads SAME state2B4!=0, actualvirtual80→00465080.
No console mutation or HUD drawing occurs merely from constructing these owners.

Evidence: successful hud-console-variable-lifetime/,hud-console-variable-type/,
hud-render-message-gates/,hud-render-entry/ and hud-render-variable-refs.log;
saved accessor/set/render-entry assembly. Ignore partial failed exports
hud-render-initial-state/(invalid0049B7F0 overlaps0049B7E0) and
hud-render-variable-owner/(invalid00720B10 overlaps00720AD0). Successful fresh
exports above replace them; no invalid function was implemented or credited.
0071F9B0 is a MATERIAL/EFFECT TABLE loader, not console initialization; no such
side-effect claim was implemented. Getter's actual record base/name offset is
confirmed through00720940/0071FE70/00720C00, not guessed from neighboring strings.

Focused native ASan/UBSan checks all11 original type/flag/word/value defaults,
command getter-1, allthree current variables/valid and invaliddomainwrites,
no_gi index10 and native invalidindex guard. Strict actual StartupSession/
GameOwnerBackend/MissionCamera source and application compile/link pass.
hud-console-verification-wip.json removes four audits. Source-only; no active
console command side effects, whole HUD draw, world/readiness/desktop/ledger credit.

## Shared byte text cache, glyph emission and panel draw consumers

`text_glyphs.h`, `text_layout.h`, `text_render.h` and the SAME `TextState`
font owner now supply the ordinary BYTE/atlas path of00645270/00647EE0.
Evidence: successful hud-text-layout-glyph-dependencies/,hud-text-layout-wrapping/,
hud-byte-text-layout-completion/,hud-byte-text-alignment/,
hud-byte-text-justification/ and hud-byte-text-word-split/ exports under
[the working evidence directory](work/mission-world-20261103/). Saved assembly
hud-byte-glyph-atlas.asm,hud-byte-text-wrapping.asm,hud-text-line-horizontal.asm,
hud-text-line-positioned.asm,hud-text-absolute-position.asm,hud-text-scroll.asm,
hud-text-viewport.asm,hud-text-caret.asm and hud-shared-render.asm settle x87
operands omitted by the decompiler. No wide/GDI/direct-font success is supplied.
00648860 dispatches its BYTE body006484C0 or wide body00648880; the wrapper is
not itself the BYTE body address. Font virtual10=006402A0 only binds its actual
atlas through Graph44. GameTextRenderer shares the SAME GameFonts five atlases,
not another allocation.0063F450 changes managerBYTE1C for a temporary separate
mode; its two direct callers0066E057/0066E0D9 are saved in
hud-direct-font-mode-callers.asm. This API explicitly implements atlas drawing.

007E5E00 truncates through FISTP QWORD and returns its LOW32, so NaN/overflow
integer-indefinite yields0; retained signed32/16 wrap uses defined unsigned
arithmetic.00644EB0 height uses signed displayHEIGHT*styleFLOAT08+FLOAT.5.
00644E10 ordinary width stores integerheight toFLOAT before nonzero aspect
multiplication, zero product uses raw height*DOUBLE.4+DOUBLE.5. Passwordflag4
uses '*' without that zero fallback.00644D60 counts stored bytes includingNUL;
00644EF0 keeps the ORIGINAL text when the first glyph exceeds the crop width.

00645270 keeps flow and absolute fragments separately, shares record strings,
and leaves dirty72 set on empty/invalid input.00645F20 uses styleBYTE2C joining,
BYTE2D wrapping, flags8 new paragraph, normalized width and height-based margins.
No-wrap cropping updates its fragment width but retains the PRE-crop remaining
line width. Wrapping starts with last-space0, then -1 on subsequent lines:
first overwide word can create an empty line; a subsequent overrun without a
break emits a prefix using the saved space width and drops the remaining word.
Whitespace recognition is007E6636 CRT bit8 for BYTE<80; tail assignment after a
break uses native C-string length and thus stops at embeddedNUL.00647830 splits
justified lines at spaces, excludes each space's width, preserves empty space
fragments, and drops a nonpositive-width final tail.00647270 distributes actual
remaining geometry width between the resulting fragments.

00646820/006468D0/00646940/006469C0 retain paragraph margins, baselineSHORT
wrap, and first ORIGINAL record's vertical alignment. Baselines start at
geometry14+verticaloffset.00646DB0 positions flags10 fragments independently;
00646A10 uses float stepping for left/justified lines, extended stepping for
center/right and caret overflow. Center remaining/2 is INTEGER division.
00647040 stores normalized absolute anchors asFLOAT before alignment; vertical3
adds the full height (not a conventional bottom-anchor rewrite).006467B0
unordered extent comparisons do not request clipping.00643E80 computes actual
maximum scroll from lastbaseline-geometry0C+1, clamps position and marks clipping;
00648EB0 separately updates/clamps signed offsetWORD76.

00647D00 clips to signed display dimensions; its top lane includes FLOAT5 and
width/height use +FLOAT1 before truncation.00647EE0 saves/restores the actual
six-word viewport, emits optional background, updates WORD76, renders flow then
absolute fragments and optional caret/debug quad. Native background driver3
restores BOOL via647FDA SETNE, whereas debug driver9 restores its RAW state.
Glyphs retain depth20/rhw=1/depth, original UV/triangle order, SHORT x wrap,
192-vertex batches and zero guard for unwritten specular.00648C80 caret uses
storedFLOATheight, baseline=ftol(y-height*FLOAT.1), raw integer height for top.
TextObject remains the sole record/disabled/color/selection/caret owner; native
offset views synchronize before layout/drawing, including existing edit setters.

GameOwnerBackend.hud_draw_text binds the actual StartupSession styles and
process TextRenderControls(008AE4F0/4), fonts, driver and graphics. Source panel
consumers now call this same bridge:004A9720 hands;004B4760 soldier (ROOT visible
reread before each text);004AFBD0 ten timers ascending;004AF2B0 scoreboard
(style08!=-1/root optional);004AD8D0 hint;004AF480 voting(optional root);
004AFE30 carnage;004AAD00 status(field34);004AFFF0/004B0210 enabled messages;
004AB220 progress(unconditional);00496E00 info(Graphstate4 save→4→restore).
Graph74/78 are distinct from I3D driver18/1C. Engine vtable1009D840+74/78
points1006ABC0/1006B220; hud-graphics-state-abi.asm confirms argument cleanup
and getter, while setter has NO defined status on its cached no-op return.
Delegated engine calls earn no new body credit.

004A41E0/004A49A0 inventory draw now uses the SAME HudOwner inline texts,
original side order/2B74 bit2 gates, four capacity/slot sequences, names/totals,
skills-root gate and description-root gate. Preserve the repeated1394 draw in
the second-side branch. Constructor0049FEF0 final word offsets are explicit in
HudOwner.inventory_word_offsets;2B74 borrows that SAME constructed word instead
of a second flags copy. Evidence hud-inventory-text-draw.asm and
hud-inventory-inline-word-map.asm. This does not implement runtime inventory
population, input or visibility producers.

Focused native ASan/UBSan checks byte metrics/crop/wrap/justification/absolute
alignment/16-bit positions/batching/caret/viewport/failure restoration and panel
gates. Strict i386 instantiates actual GameFonts/Graphics/TextState and panel
consumers with GameSceneActors/GameOwnerBackend; application compile/link passes.
Receipts hud-text-verification-wip.json/hud-panel-draw-verification-wip.json
fingerprint and remove temporary fixtures. No original/Windows execution,
production world activation, firstframe/desktop acceptance or ledger credit.
Map draw004A6FE0/004A7910/004A7EB0 now draws goal text ascending when
text18 exists and state0C!=4, then active allied labels in reverse. The recovered
004A7EB0 body is a text draw gate, NOT an objective-state update. Evidence
hud-map-goal-draw-update/ and hud-map-text-draw.asm. Populated selector/messages,
player bindings and containing0049B9F0 remain required.

## Message queue and containing HUD rendering

`HudMessageState` now owns the SAME HUD+2F90 metadata created by00494980 and
bound by00495F40. HudOwner.message_state() is passed by reference into
HudMessagesLoading; earlier duplicated constructor-only colour metadata is gone.
The input string68..167 and its first-byte flag68 are ONE array. Fonts remain
the actual shared GameFonts reference; record payloads and vertex buffer remain
owned by that same loaded message receiver. No console lexer, keyboard/chat
send service, expiry tick or runtime message producer is fabricated.

Successful hud-message-byte-render-queue/ contains00494A40/00494A60/00494A70,
00495150/00495190/004952C0/00495360. Do not use failed partial exports
hud-message-render-producers/ or hud-message-queue-completion/ (invalid interior
00494E60 and00495240 starts). hud-message-render-and-producers.asm preserves
all originals; fmt bytes8399D4='Say: %s',8399C8='SayTeam: %s',832CFC='%s'.
00494A70 ignores empty strings, increments all older record DWORD10 indices
FIRST, converts actual RGB floats with007E5E00, allocates/copies NUL-terminated
bytes, uses font3, evicts/releases oldest at capacity and appends. This is actual
queue insertion, not a replacement message formatter. Invalid zero capacity is
a defined guard for native underflow; normal00495F40 supplies50.

00495F00 saves/restores RAW driver3 around setting1 and00495A00. The BYTE draw
uses first48 bounds except mode10==1 selectssecond58. Height truncates actual
D370FLOATheightfraction*displayheight44 without +.5. StartY subtracts stored
FLOATheight, optional input line00495360, then another raw integer height. Draw
newest records in reverse, respect scroll30, original per-record line10 gaps in
mode0, bottom clipping and actual fontID14; absent font does not consume a row.
Glyph width=ftol(FLOAT(height)*aspect) without rounding; zero product advances
ftol(rawheight*DOUBLE.4). Queue colour packs original DWORDcomponents with
wrapping arithmetic. Emit top-left/right-bottom/left-bottom/top-left/right-top/
right-bottom, fixedFLOAT.02depth/1-depth rhw. Draw the glyph crossing the right
edge BEFORE testing it. One atlas bind/Graph88 triangle submission per nonempty
record, resetD36C after flush. Native16C..D36B holds1680vertices; explicit guard
refuses overflow instead of native memory corruption. Shared host submission
now accepts that larger native message buffer (ordinary text still batches192).

00495360 optional input line uses actual raw input, Say/SayTeam prefix precedence,
field168 skip, yellowFFFFFF00, raw integer height/aspect, native right-edge test,
and underscore caret at ticks150..300;301+ resets ticks. It does not invent
console command execution or expose keyboard input success. Clear/destruction
retain original font-before-payload order. Focused ASan/UBSan verifies owned
queue capacity/index/colour, empty insertion, repeated bind/clear, glyph order/
height/bounds/input/caret, scroll and RAW driver restoration on submission failure.
Actual i386 uses GameSceneActors/GameOwnerBackend/GameFonts and SAME HudOwner
state; app link passes. hud-message-draw-verification-wip.json removes four audits.

`hud_render.h` now reconstructs0049B9B0 prepass and0049B9F0 containing draw
CONTROL FLOW. Camera80 is queried first; no_gi10 then suppresses. Non-player
flags&4false ordinary mode draws scene only on20/21 or hint visibility, then
messages. Mode7 cold path returns. Playerflags4 true draw selection overlay,
enabled messages, optional command before/scene/after, crosshair/signals/roll,
messages only ifinventoryhidden, hands/soldiers5..0 ifflags18, timers/carnage/
inventory/map/info/selector/status/progress/scoreboard/voting/repeatedmap+info/
hint in native order. Required roots remain guarded; mapcenter/score/voting
retain native optionality and duplicate visibility reads. No successful fake
missing frame or required renderer is supplied.

HudSceneLoading exposes render/prepass using its SAME scene,flags20,Hud,panels
and actual camera passed by caller. Actual engine Scene78(scene,0) goes through
GameSceneActors→GameActorFrames, with checked engine virtual target. Camera80/
no_gi/source text/message/panel calls compose through actual host types. Command
prepass/before/after, selection overlay, signals and populated selector remain
MANDATORY unresolved interfaces in the i386 fixture, not successful defaults.
This is source orchestration, not completed active-world integration or complete
body credit. Native focused flow check covers suppression, both flag branches,
mode7/ordinary, reverse soldiers, repeated map/info and inventory message policy.
Strict i386 and app link pass; hud-render-flow-verification-wip.json removes four
audits. No original/Windows execution or desktop/firstframe claim.

Successful hud-command-signals-draw/ resolves three of those draw dependencies:
004C05F0 reads signals rootE0&1,004C0880 rereads SAME root then draws actual
text70.004B7830 tests nested roll+590's flag30, calls004B2FA0 (root0C visibility),
then draws SAME8HudOwner inline texts at command+A8+i9C ascending, without each
caption-frame visibility test. HudDialogsLoading/HudCommandLoading now supply
these bodies; containing hud_render calls them directly.004AAAB0 crosshair uses
field30,004B2FA0 roll uses root; neither gains a fabricated population producer.
Focused native sanitizer verifies signals visibility changes between active and
draw, roll gate, eight-inline-owner identities/order and nested roll before them.
Actual i386 again composes entire HudSceneLoading render with remaining FOUR
mandatory declared consumers: command prepass004B7780/before004B6720,
selection rectangle0049E260 and populated selector004AD140. Application link
passes; hud-dialog-draw-verification-wip.json removes four fixtures. No unchanged
subsystem re-audit, production activation, desktop or ledger credit.


Populated selector source now verified: hud_weapon_selector_loading.h supplies
004AC190/004AC530/004AC990/004ACB70/004AC370/004ACDC0/004AD140 with actual
Human storage, runtime item definitions, explicit shared icon cache and SAME text
factory. Main holder3 (or genuine empty-hand slot7), shoulder2 then holster1
stacks ascending; include runtime family0 or definition kind13. Clone main and
caption, replace i_frehnd.bmp with actual icon, link to template parent, bind text
geometry THEN zero inset. Selected scale1.1/caption hidden, others1/visible;
vertical spacing FLOAT.045; selector links selected main. Native close releases
records in REVERSE, each main frame then caption frame then inline text; retains
list capacity. This corrects the earlier handoff's ascending-close description.

human_inventory_counts.h reconstructs005A6AA0/005A6C50: definition counter
checks holders0/2/3/4/5 then holster1/pack6 and returns the first matching STACK
size; ammunition counter optionally visits pack6 in reverse overwriting matches,
then first reverse holster1 match adds and returns. No summed replacement count.
004ACB70 queries ACTUAL current player for quantities. Kind9/10/11 uses %d;
kind13 uses %.0f/%d with virtual20 auxiliary FLOAT amount (007D41A0/007D3280)
and denominator serialized payload0 bytes+8 (native payload+0C). Other kinds use
amountvirtual0C and ammunitionvirtual18. The pushed boolean belongs to005A6C50
(include pack only kind6); definition virtual18 RET does not consume it.
Kind0/1 skip labels. Source reads actual item families and definition IDs.

Successful exports hud-selector-record-draw-close/, hud-selector-byte-list-
completion/ and hud-selector-human-counts/ plus saved assembler/constant receipt
preserve original evidence. Ignore failed hud-selector-list-producers/ (interior
004ACE90 is part of004ACE20). Focused native ASan/UBSan covers real storage,
search order/labels, exact frame/text retirement, geometry, root gate and clone
failure prefixes. Strict i386 composes actual GameSceneActors/GameOwnerBackend/
GameMissionItems/PreMissionIcons; app link passes. hud-selector-verification-
wip.json fingerprints/removes four temporary fixtures. Remaining mandatory draw
bodies are command004B7780/004B6720 and selection0049E260. Outer selector show/
player eligibility/input timing/transfer and active containing-world binding remain
separate consumers. No desktop/firstframe/function-ledger credit.


Command route prepass004B7780 and path writer004B9D50 now share actual
HudCommandRoutes in HudCommandLoading. Six native F008-byte pairs each own2048
24-byte world vertices/count atC000 and512 auxiliary vertices/count atF004.
Writer uses actual16-byte positions, XYZ with FLOAT Y+.1, FISTP DWORD colour
rounding (including storedFLOAT*255), UV0. Appending begins at primitive count,
so a subsequent path overwrites the shared endpoint. Count adds positioncount-1;
empty/oversized paths reject native underflow/overflow. No route generation or
Human command queue is fabricated.004B9E80's actual Human route/model/selector
producers are researched in hud-command-route-buffer-write/ but remain required.

Prepass visits six pairs ascending, auxiliary FIRST then path. Each nonzero strip
clears texture, sets00810550 identity via Graph84→1006B360 and submits Graph88
kind2/count/format0. Engine1006B5A0 proves kind2 is LINESTRIP, format0 stride24,
versus text's format1/32. GraphicsVtable+84 is now typed, Graph88 vertex pointer
is const void* because both original vertex formats use that slot. No aliasing
of24-byte route records as32-byte CursorVertex. Saved hud-command-graphics-abi.asm,
hud-command-route-write.asm and hud-route-constants-abi.json preserve evidence.

Containing prepass now calls SAME command.draw_prepass directly; only command
before004B6720 and selection0049E260 remain required declared draw bodies.
Native sanitizer checks endpoint overwrite, buffer layout/capacity, rounding mode,
zero/camera/Border gates, six-pair draw order/borrowed pointer identities and failure
propagation. Strict actual i386 containing draw/prepass and app build pass;
hud-route-verification-wip.json removes four temporary fixtures. Source-only;
application/world/desktop/function-ledger state unchanged.


Selection0049E260 now source-composes actual actor bounds and world projection.
HudSceneLoading's SAME original6B8F8/FC pair is typed ActorBase borrowers rather
than unused DWORDs; original load/retire clears that SAME pair (including repeated
0049B610 clear). Draw selects first only when nonnull AND signals004C05F0 active,
else second; null returns without querying command visibility. Command visible
returns before bounds/projection. These are cold borrowers until real selection
producers publish actors; no selection/input/gameplay activation is fabricated.

actor_world_corners.h supplies virtualDC for the14 confirmed loading vtables.
Human005A85F0 uses EXACT visual name "a"/mask1, cached visual bounds then THAT
visual's world matrix. Common006115B0 and StaticWeapon00605E50 update root,
aggregate LOCAL bounds006111F0/006112B0 or00602B20/00602BC0, emit8corners Xmax
first/Ymax first/Zmin first, then root-world S_vector::operator*=. Common visuals
OBJ_/LMAP/SNGM/SMMR contribute; StaticWeapon only OBJ_, excluding descendant
visual flag1000 (not the root). Recurse through excluded nodes; local*parent
matrices retain excluded-parent transforms. Flat descendant bounds minX==maxX
ARE eligible: original FCOMP/test41/JP rejects greater/unordered, not equality.
The child fold uses native eight-corner face order. Root has original +/-1e16
sentinel when not visual; unknown actor families reject. Native unwritten fourth
lanes remain defined0, with no invented semantic or full layout credit.

Original00698500+18 is the WORLD SCENE, NOT camera. Scene vtable1009B798+98
is100476A0/RET16 (scene,input,output,count). Host projects eight actual world
corners through that scene's active camera/cached projection; missing camera's
80000009 fails instead of consuming native unwritten output. Brackets clamp
extents to10pixels, expand to2.5%display minimum AFTER clamp (no final reclamp),
use quarter-length corner arms and exact16vertex/8line order. Native z=.5,rhw2,
colour80FFFFFF,UV0; unwritten specular has defined0. Actual factory binds no
texture and Graph88(kind1,count8,format1); no world-transform reset invented.
Containing hud_render calls this source directly; only command-before004B6720
remains declared. Native ASan/UBSan verifies root/descendant transforms, Human
visual identity, common/static exclusions and flat bounds, pair/command gates,
project failure, vertex order/clamps/minimum size. Actual i386 composes SAME
GameSceneActors/GameOwnerBackend/ActorBase and app links. Successful exports
actor-world-corners/,actor-local-bounds/, saved asm and hud-selection-actor-
projection-map.json preserve evidence; hud-selection-verification-wip.json removes
four temporary fixtures. No active world/desktop/ledger credit.

2026-11-10: [HUD Human/player presentation checkpoint](hud-player-presentation-loading.md) implements0049B7E0/0049B870/populated0049B900,004B3C60/name/photo/caption/mode textures and actual compass marker construction/removal over SAME HUD owners. Source Players/presentation bridge and failure/duplicate/lifetime checks pass. Later HUD simulation and actual world/session remain; application still unloaded.
