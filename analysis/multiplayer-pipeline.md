# Multiplayer reconstruction — 2026-11-01

Status: integrated front-end source awaiting desktop review; session transport and game entry unavailable. No multiplayer desktop acceptance.
The user selected this family after accepting team selection in commit13863a7.
Loadout screens remain implemented but **completely untested until accessible**.

## Completion boundary and blockers

The full boundary is the six multiplayer units in
[menu-family-delivery.md](menu-family-delivery.md#multiplayer): browser, create,
filters/information, Add Server, ordinary Join, and standalone connection.
Ordinary main slots8–14 and connection-owner slots1/2 are distinct products.
Existing [route census](menu-route-census.md), control programs and expressions
already establish their registration/builders; do not repeat that census.

The actual receiving network game/session owner is required for Phase 2 game
entry; Phase 1 multiplayer menus and available service paths can proceed.
Controller006C20F0 status9 preprocesses00730A40/0072E890/00731610 and constructs
00698510. The latter creates real game objects0049A420/00490740, scene/camera,
and controller state. It is not an independent menu lobby and is absent from
the reconstructed application. Successful transport alone does not establish
game entry. Status7/8 construct the separate connection owner0068A710.

Internet browsing00723250 uses a separate GameSpy SDK path0073A7A0/0073AB30,
secret `sK8pQ9`, query keys1/11/6/5/8/10/4/51/3/19/52/53, pumped00723240.
Its availability has not been tested. LAN must not substitute for Internet.
Dedicated-server007226B0 launches original hd2ds.exe;
the project prohibits executing original executables. That branch needs a
reconstructed service or an explicit unavailable outcome.

The browser server-record/discovery dependency is a source-only checkpoint. It
does not close a family checkbox or enable an unfinished menu action. Phase 1
LAN/session acceptance still requires a controlled local endpoint and a user
desktop test. No successful placeholder.

## Original evidence

Client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Static read-only Ghidra exports under ignored `analysis/work/`:
`multiplayer-services-20261101`, `multiplayer-discovery-20261101`,
`multiplayer-client-20261101`, `multiplayer-wire-20261101`,
`multiplayer-callbacks-20261101`, `multiplayer-bitstream-20261101`.
The client batch stops at undefined00636570; later requested entries are not
implicitly exported. ExportFunctions decompiled=true checked for used bodies.
Only reconstructed bodies present in the frozen inventory can receive credit.

### Services and ownership

00723C70 cancels old work0072E4E0, installs enum callback and dispatches00723C40:
config+174==1 Internet, otherwise LAN00723AB0. Restore00723CC0 enumerates stored
records with query DWORD2; selected-row query00723CA0 uses DWORD0.
00723A30 creates/initializes the real DirectPlay8Client. 00723330 builds real
DirectPlay8Address TCP/IP host/device addresses. Host port defaults31567
(0x7B4F); a supplied record overrides IP/port. The Add Server UI's literal11001
is separate and must not silently replace the LAN discovery port.

Application GUID bytes00816350:
`82b0e52aa0cfd4119e1d0050fc213de6`, GUID
`{2ae5b082-cfa0-11d4-9e1d-0050fc213de6}`.
00723AB0 calls Client::EnumHosts with this GUID, 4-byte query data, LAN count
and timeout DWORD_MAX, targeted count1/timeout1000, retry0, flags0. Original
accepted results: S_OK, 0x80158040 (addressing), 0x80158320 (invalid device
address), 0x0015800E (pending). Preserve HRESULTs; accepted submission is not a
server response. 0072E4E0 cancels all, Close(0), releases actual client/server.

00722150 bridges DirectPlay callbacks to00722180 unless global mode7 idle.
ENUM_HOSTS_RESPONSE0xFFFF000B uses pApplicationDescription session name,
pAddressSender hostname, round-trip latency and the *response data*, not the
application reserved-data field. In the original it may update a borrowed
record through pvUserContext. Native reconstruction should copy callback data
before returning and queue owned values; no queued COM pointers/UI callbacks.
00635AA0/006358C0 enqueue under a critical section;00635640 drains on the menu
thread. LAN duplicate suppression is by IP alone. Selected pointer+21C is
cleared on refresh, and Join copies39 DWORDs before subsequent screen changes.

### Record and persistence

00724EE0 constructs0x9C bytes and calls reset00724EF0. Reset writes map ` ??? `,
style4, ping9999, players/capacity0 and bytes80/81/82/88/98 zero; host, IP, port,
versions and other words are preserved. Native fresh storage initializes the
otherwise uninitialized original bytes for memory safety.

| Offset | Data |
| --- | --- |
|00/33|51-byte host/map fields|
|68/6C|IPv4 bytes in network order / DWORD port|
|70/74/78/7C|style / ping / players / capacity|
|80/81/82|dedicated / manual flag / password|
|84/88|unknown DWORD / byte|
|8C/90|minor/major version (original globals1.12)|
|94/98|edition word / listed byte|

00725020 writes chunk2000 (decimal), two WORD lengths including NUL, then fields
68/6C/70/74/78/7C/80/81/82/84/88/8C/90/**98/94**. Tail order differs from
memory order. 00725290 scans children for2000, ignores unknown children, and
retains field defaults on bounded reads that cannot fit. Its overlong-string
discard loop has unsafe decreasing-size stack writes. Do not reproduce UB;
consume the full declared string safely, retaining original truncation to49
characters when length>=51. Reject malformed chunk bounds/unterminated strings.

00619D30 persists global `PlayersProfiles\\ServerList.bin`: root0x28A, groups
0x291 ordinary and0x295 favorites, count0x28D, record wrapper0x296 containing
chunk2000. 0061A120 clears both lists before opening, reads these groups, and
ignores legacy0x294 (also0x28F in ordinary). Counts are reserve hints, not a
source of records. No user file is modified by source/native fixtures.

### Discovery bitstream

0073D000 reads little-endian least-significant bits first, crossing32-bit words;
native bounds checks must avoid its unchecked read past advertised data.
00722180 reads2-bit response kind, then map bytes through0073D0A0(limit50,
always terminates index49). Record starts immediately after that bit string.
Legacy layout: style2, dedicated1, players6, capacity6, password1, minor8,
major8, port16 **sign extended**, flag88:1. If major!=1 or minor>20, rewind to
record start and read minor8, major8, style**3**, dedicated1, players6,
capacity6, password1, port16 sign extended, flag88:1. Edition defaults1;
extended host names containing `DEMO` force2 (007E5A40 is strstr), others read2 more bits. Kind0
delivers browser record; kind1 continues into a separate game-info payload
00725FB0 and must not be presented as a simple browser response.

Widths are confirmed by disassembly00722275–0072248C: log2(3)+1 truncates2;
log2(4)+1 gives3. Constants0080F748=2,008108A8=3,0080F548=4,
0080FB00=1. Host conversion00724760 uses ACP and00724800 UTF-8, capped50
(original output ends49). Fresh manual and response records are distinct.

### Filters and browser state

00635E40 applies filters only when profile-options+4==1. Max ping+394 (enabled
only if signed>0); style+398(-1 any); bytes39C responding-only(ping<=9998),
39D dedicated,39E not-empty,39F not-full,3A0 no-password,3A1 current-version.
Preserve the actual not-empty predicate: **capacity!=0**, despite its label.
Not-full compares capacity!=players; both version words must match.

00634EC0 Restore state4/9 and00635290 Refresh state5 toggle into cancellation
on a second click. 00635AD0 drains pending results, refreshes rows each1000ms,
and stops discovery after15000ms inactivity. Internet entry state6 displays
stored records first. State9 displays responding/unlisted records. These UI
states and filters do not prove that a network request succeeded.

## Integrated front-end source checkpoint

Ordinary type2 slots8–14 are now registered in the real application menu;
Main enables LAN and Internet entry. This is **unaccepted interactive source**,
not completed multiplayer delivery. All six family gates remain open until their
actual desktop/service requirements pass.

- Browser: eleven original row products, five text columns, password/dedicated/
  voice icons, sort direction artwork, scrolling, selection, favorites, Refresh,
  Restore and Back. LAN calls the real OS DirectPlay adapter. Internet retains
  stored records and reports unavailable GameSpy discovery; it never substitutes LAN.
- Create: original base map directory allowlist, mode lists, selected rotation,
  five presets, numeric/toggle rule rows, hostname/password/port, persistence and
  an owned validated launch request. Cooperative catalogue content is skipped
  by the original base catalogue loader00618A00.
- Filters, Add Server, information and ordinary password/console are integrated
  products with their actual frame/control IDs. Invalid IP/port, empty password,
  stale selection, query failure, timeout and service unavailability are explicit.
- The separate type8 connection owner has independent scene/queue/registry and
  **regular** console1/password2. Controller status7 means host mode2; status8
  means client mode1. It requires a copied matching launch request and is not a
  replacement for the ordinary type2 modal10/console13 products. Normal Main
  does not invent a standalone route merely to make this owner reachable.

### Settings and map behavior

0061A860/0061AC10 persist `MPSetingsOrig.sav`: root4E6, host44D and the
4B0/4B0/514(version42) settings nesting; four22-int rule blocks and five presets
of four named rotations. Tag1B7 stores **preset, style, voice, Internet byte,
dedicated byte**. Legacy1AD initializes preset0. Native reads preserve omitted
fields transactionally and reject corrupt chunk bounds. Defaults, map names and
all30 base directories match an independent static-file oracle (8/12/10 maps).

0062E7A0 clones the normal map-row visual plus independent caption; source rows
remain visible.0062E920 updates the cursor ghost atZ=-1 and selected-list live
ordering.0062EA90 accepts strict rectangles: availableX(.08,.26), selectedX
(.28,.46), bothY(-.68,-.30), using the original negative normalized menuY.
Selected→available removes; available→selected copies and inserts at the hovered
row; outside cancels.006197B0 uses stable erase/reinsert, never a team swap.
Per-row updates preserve the list scroll; the transient ghost is excluded from
picking and its caption renders after ordinary labels.

### Information and callback ownership

Kind1 information and kind2 restored-record responses now have distinct owned
inboxes. Per-request identities are kept until DirectPlay Close joins callbacks;
UI lookup uses the requested identity even if a server advertises another port.
No callback response/COM pointer or UI-owned row survives by borrowing.

00725FB0/00725EC0/00725E60/00749AE0 decode player vectors, the11-bit signed
score (1024 remains positive),10/10/10/5-bit statistics, team DWORD and names.
00725C40 decodes signed16 rule fields; field12 is **not transmitted** and remains
explicitly absent. Two10-bit team scores finish the payload. Positive player
counts above64 and malformed/truncated packets are rejected safely.

0062F960 binds named players in reverse order, original team symbols, names,
word04 latency and score columns; Allied uses score[1], Axis score[0], with Axis
omitted for style3. Metadata and transmitted rule values use the original label
IDs.0062F3E0 registers twelve52-byte styles at0083FAD0..0083FD40; native authored
column rows preserve their font/height/anchor/width/color distinctions.00645270
caps column width using text-owner render width+04. The symbol atlas is original
`Text/Fonts/SYMBOLS.tga` (512×256), separate from the256×256 Set2 fonts. Staging
retains ownership/provenance guards and accepts the previous five-file manifest.
Clipping and ownership are native-audited; visual fidelity still needs desktop review.

### Connection and game boundaries

`MultiplayerLaunchRequest` copies host settings, rotation, selected server and
password before replacing any owner.0068AED0's phase model preserves300ms
initial delay, consumes the tick that crosses zero, maps original rejection
codes/labels, waits500ms before regular password2, and preserves nonempty retry,
Escape/No, Exit Game and genuine asynchronous completion as distinct actions.
0068B9F0 resets phase0 on successful password submission. The isolated owner
shares resource/input machinery with preparation menus but owns its own products.

**Session Connect/Host and their auxiliary UDP/GameSpy dependencies are still
unimplemented.** Production Join/Create/standalone report that unavailable
service; their timers and native trace audits never fabricate connected sessions
or successful OS calls. The ordinary console also reports that game loading is
unavailable. Original00722790 host ENUM_HOSTS_QUERY callback007260A0 refuses
queries until a real game owner00698500 exists with progress+28>=100; advertising
a ready synthetic host would contradict the original. Actual status9 receiver
00698510 is Phase2. These limitations keep join/standalone functional gates open;
source factories and typed requests do not close them.

## Verification

Strict i386 cross-builds: application-launch, engine-window and startup-probe.
Existing native regressions and158 Python tests pass. No Windows executable,
original DLL/game or Proton has been run by the agent.

Temporary native ASan/UBSan audits execute production headers and the actual
adapter `.cpp` with trace-only COM interfaces:

- Independent1361-byte settings oracle, all prefixes and20,000 save mutations;
  original30-map catalogue and six independently packed old/new kind0/1/2 replies.
- Player/rule/signed-score edges, every truncated packet prefix and120,000 packet
  mutations; authored server/information columns, viewport clipping and offsets.
- Ten injected adapter acquisition failures, five submission HRESULTs,20 appended
  targets,2000 concurrent callbacks and owned restore/info identities. Cancel,
  Close, restart discard and release ordering pass; this is not live networking.
- Host/client phase timing, every modeled reply code, delayed password, empty
  rejection/retry, copied request ownership, explicit unavailable submission,
  isolated type8 wrapper and map insertion/drop geometry pass.

Ignored audit logs and static oracles:
`analysis/work/multiplayer-current-verification/`. Temporary audit sources,
executables and the bounded Ghidra helper are removed after verification; no new
permanent tests. Existing font-staging assertions now derive resource counts from
its source catalogue. [Function audit](multiplayer-function-audit.md) adds33
partial inventoried client entries and extends006C20F0 without double counting:
**559 partial (554client/five engine), zero complete /18,783 frozen**. No delegated
engine, OS transport, absent-inventory callback or unimplemented game owner credit.

Additional static read-only exports: `multiplayer-ui-completion-20261101`,
`multiplayer-config-20261101`, `multiplayer-profile*-20261101`,
`multiplayer-widget-deps-20261101`, `multiplayer-controls2-20261101`,
`multiplayer-server-rows-20261101`, `multiplayer-row-20261101`,
`multiplayer-player-wire-20261101`, `multiplayer-map-drag-20261101`,
`multiplayer-map-order-20261101`, `multiplayer-information-binding-20261101`,
`multiplayer-information-address-20261101`, `multiplayer-column-layout-20261101`,
`multiplayer-standalone-password-20261101`. Undefined callbacks were exported
using statically confirmed bounds only; these do not modify the frozen inventory.

## Desktop review and next work

Stage the cross-built application for user review only. Exercise ordinary Main→
LAN/Internet browser, empty/error/refresh/restore, scrolling/selection/favorites,
Add Server, filters, Create/map drag and rule changes, persistence and Back.
A controlled responding endpoint is needed to inspect populated browser/info and
ordinary protected-server password behavior. Standalone7/8 owner and its password
retry remain source/native-only until their real service route is available.

Do not mark the plan/family checkbox complete or commit interactive changes before
user acceptance. After UI feedback, recover available session/auxiliary-service
contracts and validate with user-operated controlled endpoints; no fake ready host,
GameSpy replacement or original dedicated-server launch. Status9 game entry stays
in Phase2. Loadout/equipment screens are implemented but **completely untested
until accessible**. Accepted team checkpoint remains commit13863a7; no push.

### First desktop launch correction — 2026-11-01

User launch log stopped after Create Server hostname initialization with the
generic missing-native-constructor error. The map panel `MP_MAPS_PANEL`
(24300000, kind5) had no MultiplayerMenu container branch and fell through to
MenuControl. Route it through existing OptionsGroup recursive construction, as
used by team selection. This retains nested lists, sliders and row factories.
Generic unsupported-kind diagnostics now identify kind/ID/frame. Strict i386
builds, existing native regressions and158 Python tests pass; corrected executable
is staged for user retest only. The previous source verification did not detect
this live factory-composition gap. No desktop acceptance or new function credit.

### Combo-header launch correction — 2026-11-01

The next user launch reached the Create map-list preset and failed with
`Missing visual in menu button: header*`. PreMissionChoice::children incorrectly
set the header definition's normal visual to `text*`. Original00658770 binds the
header to vtable00814320; its initializer+2C resolves to0065ABA0, which sets the
supplied visual direction/pick ID and passes that same visual to00644A80. There
is no child text-plane lookup. Remove the incorrect normal override and retain
direct header binding. This shared correction also applies to source-only loadout
combos; loadout screens remain completely untested until accessible. Existing
native regressions and158 Python tests pass. Desktop launch retest is pending;
no new function credit or commit.

### Browser/Create desktop review corrections — 2026-11-01

User desktop review reached the menus and reported ineffective list actions,
disappearing sort arrows, missing Add Server prompt, captured edits blocking
Yes/No, differing unanswered rows, Create overflow/backing-plane artifacts and
an inverted-bounds exception when map dragging began. Valid discovery could not
be tested in either original or reconstructed application; no live failure or
success is inferred from that observation.

Corrections implemented for the next desktop retest:

-00634EC0/00635290 expose their original busy/cancel captions916/918 and disable
 the competing operation until cancel/15-second expiry. The empty stored-list
 case also enters/exits a visible pending state; it invents no endpoint or reply.
 Internet listing still reports the unavailable GameSpy service. LAN enumeration
 continues to use the real OS adapter; source checks do not validate live discovery.
-00654C60 (vtable00814C28) copies normaldown+localB0 into the other three direction
 planes. Static disassembly654D04..654E60 confirms that authored positions are
 not retained independently. The adapter now aligns all four and restores the
 chosen plane after inherited focus/activation/click paths. Relevant original
 Patch.dta models were extracted statically under ignored verification output.
- Add Server shows English2022 `ENTER THE IP ADDRESS`. Native edit capture
 previously blocked hover focus, then released on press without transferring
 focus, so click reactivated the old editor. After successful release the native
 pointer adapter now resolves focus before forwarding the same press. Refused
 release and motion-only capture retain their existing gates. This is a native
 composition repair, not a claim that an extra call appears in0066B990 itself.
-00637440 initializes new manual host/map display strings to unknown, keeping IP
 only in address fields. Unanswered rows show localized2894 UNKNOWN STYLE,
 numeric9999 ping, spaced player counts and subdued text. Previously saved
 manual records with style4/ping9999 also display the unknown host.
- Numeric child labels/values follow parent row visibility; off-page rules no
 longer keep drawing over the voice row and buttons. Labeled-toggle caption
 surfaces face away from the camera while their seven checkbox art children
 retain world transforms across temporary reparenting. No artwork is replaced.
- Map drag captions use the source row's valid projected bounds plus cursor
 delta, avoiding an empty/inverted world-bounds query for the transient clone.
 Both min/max bounds receive the same translation. The actual draggable visual
 and original drop/reorder logic remain separate; desktop fidelity is pending.

New read-only static exports: `multiplayer-review-widgets-20261101/00654C60` and
`006541E0`; client fingerprint unchanged. Strict i386 application/engine-window/
startup-probe builds, existing native regressions and158 Python tests pass.
Temporary native ASan/UBSan executes actual pointer-dispatch/geometry code:
Yes/No/port focus transfers on the first press; refused/motion-only capture is
preserved;441 cursor translations retain finite, non-inverted caption bounds.
Audit log: `analysis/work/multiplayer-current-verification/review-audit.log`.
Temporary audit code/binary removed afterward. These checks cannot establish
actual engine rendering or live networking. No new function count, desktop
acceptance, commit or push; all multiplayer delivery gates remain open.

### Preset/drag/eligibility follow-up — 2026-11-01

User review found the open Create preset list mixed with underlying map labels,
map rows retained selection tint, drag moved text without its border, favorites
remained enabled without selection, and edits stayed dark during typing.
Corrections staged for another desktop review:

- The Create preset dropdown's records render after underlying labels. Native
 bitmap glyphs beneath its text viewport are split/clipped out, with UVs preserved;
 scene depth alone cannot occlude these later bitmap submissions. Pointer picking
 prioritizes the expanded list within the same viewport. Other preparation combos
 remain source-only and completely untested until accessible.
- Map row activation retains the logical identity but clears movie-row selection
 tint. Hover/selection/reorder paths use the original normal artwork. Drag now
 retains the complete recursive row clone (normal/active planes and nested border)
 rather than copying only its normal leaf with incomplete ancestor transforms.
 The independent caption still uses the source projected bounds/cursor delta.
- Favorites eligibility follows a valid selected server at entry, repaint and
 selection; it is disabled without selection. LAN's existing availability policy
 remains separate from that selection requirement.
- Shared ProfileEdit paints normal/active artwork from actual cursor containment,
 independently of character capture. Leaving an active edit removes hover shading
 while retaining its keyboard capture until the normal release path.
- Verified original loose English TEXTY.txt record2579 (0xA13) is exactly
 `Same version`. Preserve its title/lowercase spelling; do not uppercase it or
 infer a broken base-game resource lookup from the datadisk behavior.

Verification: strict i386 application-launch/engine-window/startup-probe builds,
existing native regressions and158 Python tests pass. Temporary native ASan/UBSan
executes production glyph occlusion against5000 masks, checking excluded area,
finite coordinates and preserved UVs. Production recursive cloning retains the
normal/active hierarchy and a nested border in an owned native fixture. Log:
`analysis/work/multiplayer-current-verification/followup-audit.log`. Temporary
source/binary removed after execution; no new permanent tests. Actual dropdown
stacking, input appearance and dragged artwork still require user desktop retest.
No new function credit or live-network acceptance; no commit or push.

### Click/hover/caret/footer follow-up — 2026-11-01

Further user review corrects the preceding normal-only map artwork assumption:
map rows use hover artwork, while logical selection must not retain movie text
selection tint. The row now tracks hover independently and clears only that tint.

The Create preset remains expanded when pointer focus leaves it. Header and arrow
clicks toggle it; selection, Escape or a press outside its header/arrow/list close
it. The header plane has its own combo-part pick ID. This shared preparation
control remains completely untested in inaccessible single-player loadout flows.

Dragged row/border meshes precede bitmap text in the engine render pass. The host
now excludes underlying bitmap glyphs inside the moving row rectangle and draws
the drag caption last, using the existing UV-preserving glyph exclusion routine.
The input underscore remains a separate final glyph (00647EE0/00648860), with its
quad raised only when necessary to fit one pixel inside the input's lower edge;
ordinary text positions and caret blink width remain unchanged.

An active ProfileEdit previously returned zero from leave_focus, preventing
00656C30 from entering any other control. Original visual leave0065B090 calls
00662770 and restores the normal artwork without ending BF/C0 keyboard capture.
The host now allows pointer focus to leave while retaining character capture;
clicking elsewhere still uses the existing capture release/validation path.

Original browser formatter006363E0 queries the shown record count via010000A2,
uses total/player counters at options+28/+2C and formats the footer with the
string at0083FF00: `%d/%d ----- %d %s`. Its suffix lookup0x22AE (8878) is exactly
`online players` in the original loose English table. The host now displays
filtered/total followed by total player count and that localized suffix, rather
than invented `Servers:`/`Players:` labels. The user's `active players` wording
was an example; the actual English resource is preserved.

Strict i386 application-launch/engine-window/startup-probe builds, existing native
regressions and158 Python tests pass. A temporary ASan/UBSan geometry audit checks
1840 combinations of input style, short-plane height and text length: caret stays
inside the lower border, blinking preserves text positions/UVs, and drag masks
exclude underlying glyphs. Log: `analysis/work/multiplayer-current-verification/caret-drag-audit.log`.
Temporary audit source/executable removed. Function ledger validation passes with
559 partial entries unchanged. Staged only; actual visuals, hover and click
behavior require user desktop retest. No new function credit, commit or push.

### Caret, rule eligibility and input clipping — 2026-11-01

User review still found the caret low. Direct disassembly supersedes the prior
border-clamp approximation:00648C90 loads text cursorY(+6E),00648CAC multiplies
integer font height by float0080F550 (bytesCDCCCC3D,0.1f),00648CB2 subtracts and
00648CB4 truncates to an integer. Ordinary glyphs at00648A7E use cursorY without
that subtraction. The host now applies that distinct caret baseline lift,
promoting height and the float constant before subtraction as in the x87 path.
Normal text and blink-off width remain unchanged. This compares static original
instructions, not an original desktop execution. Evidence:
`analysis/work/multiplayer-rule-caret-20261101/caret.asm`.

00629EE0 explicitly sends02000073(0) to labeled rule checkboxes after setting
their captions. Removed the extra10-byte inset from Allow Respawn, Friendly Fire,
Automatic Team Balance, third-person and crosshair caption planes.

0062D880 first applies stored values and dependent enable states, then mode
messages. Supported base-mode rule eligibility is:

| Rule | Deathmatch0 | Occupation1 | Objectives2 |
| --- | --- | --- | --- |
| Point limit | enabled | enabled | disabled |
| Allow Respawn | disabled | disabled | enabled |
| Friendly Fire / Automatic Team Balance | disabled | enabled | enabled |
| Round time | enabled | enabled | enabled |
| Respawn time | follows stored Allow Respawn | follows stored Allow Respawn | follows Allow Respawn |
| Inverse damage | follows stored Friendly Fire | follows Friendly Fire | follows Friendly Fire |

Dependent refresh is repeated in0062D3C0's tail. Rule-checkbox observers now
refresh after toggling; fill_create refreshes after assigning all values, covering
mode changes and re-entry. Disabled values remain stored. Numeric disable events
reach both editor and caption. Unexposed cooperative mode3 stays outside this
base catalogue route. Original full rule-refresh disassembly:
`analysis/work/multiplayer-rule-caret-20261101/rule-refresh.asm`.

Further user review found the preset header must open without the arrow's click
sound, and long editable text overflowed its frame. Header toggling now returns
the non-accepted click result so0066B110 is not requested. All shared ProfileEdit
text bindings retain an editable viewport in the native renderer: after normal
text positioning (including active caret-following horizontal scroll), glyphs
are clipped to its inset rectangle with interpolated UVs. This applies when
capture is inactive too. Original00647EE0/00648860's text viewport/clip ownership
was already confirmed; the host single-record submission path had omitted it.

Strict i386 builds, native regressions and158 Python tests pass. Temporary native
ASan/UBSan passes2160 caret/long-input cases,5000 clipping/UV cases and84 mode
dependency predicates. Log: `analysis/work/multiplayer-rule-caret-20261101/native-audit.log`.
Temporary audit source/executable removed. No new frozen function credit
(559 partial unchanged), desktop acceptance, commit or push. Staged controller
requires user review of caret placement, long input containment, checkbox caption
alignment, mode switching, live toggle dependencies and silent header opening.

### Rule caption input follow-up — 2026-11-01

User clarified the Create rule-row contract: clicking captions attached to an
input box does nothing and is silent; hovering those captions darkens their
input box. Checkbox-rule captions instead toggle normally with the existing
click sound, and hover white. The user's earlier request for silence on all
rule captions was explicitly corrected; only input-rule captions are silent.

The native composite previously forwarded every click to ProfileEdit. Original
0065B110 rejects clicks whose target is not the editor's identifier; it does
not activate the editor through its caption. MultiplayerNumeric now retains
the full pointer-part target across entry/movement/press and declines caption
clicks before calling the editor. Its caption plane keeps its distinct child
ID. ProfileEdit accepts an optional additional hover rectangle query; this
composite supplies its caption bounds. That changes visual hover only and
does not acquire character capture. Other edits have no additional hover area.

For labeled toggles,00654840/006548B0 already update text focus and toggle-part
focus independently. The host now bridges that text focus into the bitmap
renderer highlight on entry/exit. Checkbox click routing and dependency refresh
remain the existing00654910/00654920 ->04010000 path. Disabled text/controls retain
their existing dimming and eligibility checks.

Strict i386 builds, existing native regressions and ledger check pass. Temporary
native ASan/UBSan audit executes exact production caption click and hover method
bodies with state fixtures:20 caption/input click pairs,18 combined caption/box/
outside/disabled hover cases and checkbox-caption focus entry/exit. Log:
`analysis/work/multiplayer-rule-caret-20261101/caption-audit.log`. Temporary audit
source/binary removed; no permanent tests or new function credit (559 partial
unchanged). Staged only; actual caption picking, sound and appearance require
user desktop retest. No commit or push.


## Desktop acceptance — 2026-11-01

The user reported “Good, I can find no more issues,” requested recording the
checkpoint as done and then explicitly requested committing before next-checkbox
research. This accepts the reachable multiplayer frontend and the Team Selection
follow-ups reviewed in this session. Accepted controller SHA256:
`f2295542f86575c6df66a35863ae31c9085deae85583c922c1e68d46bdabfe83`.
[Acceptance record](multiplayer-desktop-acceptance-20261101.json). This is broad
user desktop acceptance; no individual case execution is inferred.

**Every UI behavior needing a ready server remains untested**, including populated
browser/discovery, server information/player/team/rule displays, live refresh and
favorites, ordinary Join/password/console and standalone connection screens. The
user reports the old server binary implementation appears broken. Phase3 now
covers server reconstruction, actual Host/Connect and required auxiliary services,
then controlled endpoint UI validation. The agent must not execute the original
server binary or manufacture a successful service. Actual game/status9 owner
reconstruction remains Phase2, with live server integration after Phase3 readiness.

Phase1's reachable multiplayer frontend checkbox is complete at this explicit
boundary. Browser/Create/filter-editing/Add Server family units are accepted only
for that frontend scope. Live information and connection validation remain open
in the Phase3 family units. Loadout screens remain completely untested until
accessible. Existing historical native/staged-only statements above describe the
state at those checkpoints and are superseded by this scoped user acceptance.

Strict i386 builds, native regressions and temporary sanitizer audits passed for
the reviewed build. The frozen-function follow-up audit adds five partial client
bodies, bringing the total to564 partial (559client/fiveengine), zero complete.
No live network or original-execution result is inferred from desktop acceptance.
