# Front-end Options — native integration, desktop acceptance pending

2026-10-31. Main family6 is integrated. The Options and shared-runtime-dialog
delivery checkboxes remain open until desktop acceptance. In-game family2 is a
separate delivery item and is not enabled here. Nothing committed or pushed.

## Completion boundary and original evidence

Client SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`,
engine SHA256 `12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
Existing [construction catalog](menu-control-programs.json),
[widget contracts](menu-widget-construction.md) and
[character/capture contracts](menu-character-focus.md) are reused.

| Original address | Behavior reconstructed |
| --- | --- |
| 0061BEA0 | First-entry synchronization, entry71, apply6F, Back0A/C4 and unassigned replyC7 |
| 0061CF60 | Options root/tabs, profile text and ruler alignment |
| 0061DBC0/0061E140 | Graphics aliases/quality/features and sound/music pages |
| 0061E390/0061F5A0 | Controls/mouse, five generated binding groups, row clones and two standard layouts |
| 0061F1D0/0061F350 | Game/subtitles/parental and network/voice settings |
| 00620960/00620950/00620B80 | Category selection/linking and refresh of other lists before the selected list |
| 0061FC50/006B3200/006A8BE0 | Standard setup1 chooses primary; setup2 chooses alternate, then refresh |
| 0064D800/0064E530/0064E7A0 | Input child `row|1`, caption child `row|2`, capture warm-up, binding index/caption and refresh |
| 006C84A0 | Candidate polling with mask9A; this caller excludes joystick candidates |
| 006A8230/006A83C0/006A8480 | Reserved scans, assignment and duplicate clearing with mode overlap and P-command exemption |
| 006A85B0/006A8300/006A8290 | First unassigned record, category/caption and one-based contiguous-group ordinal |
| 006AB590/006AB520 | Localized mouse/special-key labels and English engine key-name fallback |
| 00655950/00656700/00655AF0 | Vertical row layout, nine-row window, scroll and ensure-selected-row-visible |
| 006B2950/006A8770/006B3530 | Original Options.bin writer layout, before dirty-binding application |
| 00641E30/00647EE0/00648C80 | Three ruler anchors and blinking underscore for the captured binding value |

Read-only exports are in ignored `analysis/work/options-*-20261031` directories,
alongside the prior `menu-options-subbuilders`, `menu-screen-builders` and
`menu-route-builders-20261031` exports. Function census credit is restricted to
present non-thunk inventory entries; the absent row/update/callback helpers do
not become new counted functions. Original engine calls remain delegated.

## Integrated behavior

`options_definition.h` supplies all root/page identities and original captions.
`OptionsMenu` loads the actual `options` model through the existing cache,
reparents six tabs under GROptions, renames the seven graphics features GD00..06,
and retains each generated frame and every cloned descendant. Each category
contains every configuration record with that category, including both modes;
hidden groupFFFFFFFF records are not invented as visible rows.

The registered screen uses the existing recursive kind-five factory, event
runtime and input forwarding. Binding lists add kinds0B/0D and keep full internal
target bits. The nine-row window uses original spacing bits3D1374BC, invalidates
text bounds after movement, links the real scrollbar, and supports wheel,
arrows, Home/End and Page keys. The two standard-layout buttons reset the real
profile binding configuration and install its current mode through the existing
input manager. They do not create an independent menu input map.

The binding input child uses the existing typed BF/C0 capture owner, selection
clock and character release protocol. A row waits one update before polling.
The actual engine supplies modifier/scan/button/axis values; X/Y candidates are
rejected by006A83C0, wheel directions are accepted. Escape cancels. Reserved
F1..F10/F11/F12/Escape/Pause scans are rejected for assignment. Duplicate clearing
excludes hidden rows, respects modes1/2/3, and permits the original captionsB20
and B28 to share a key. A successful assignment refreshes all categories,
releases capture, restores pointer input and suppresses pointer processing200ms.

Existing widget observers change the selected profile's actual settings and
invoke the original engine/sound/control adapters. First entry queues values
once; reopening retains page/selection state after Back hides all pages. Gamma
availability reads the actual graphics-info bit2. The crosshair preference uses the original mission-owner eligibility
condition; the original connection-rate and joystick/force-feedback
controls remain hidden. Voice enable controls voice-volume availability. No
network service is fabricated for these preference controls.

Back scans the complete configuration for an unassigned kind/code pair. If
found, it opens the genuine slot-zero mode2 dialog, label9520, with both reply
codes020000C7. OK and Escape dismiss the dialog before restoring binding page9,
its actual category and its original row ordinal. Otherwise Back queues hide
for pages6..11, apply6F, then pop03. Apply writes Options.bin, applies graphics,
then installs dirty bindings. The writer preserves dirty bytes/padding and the
three sequential top-level chunks25A0(version141),25A6(binding version42/data),
25A5(settings1201). Failed open returnsfalse and is logged; the original caller
continues applying/returning. Runtime paths use isolated staged profile copies.

The Options header profile caption is refreshed on show. General Main labels
and the already deferred version formatter remain their later plan item.
Rendering remains English/800x600; broader localization/layout is not claimed.

## User launch failures and corrections

The first user launch reached the Options model and failed the old256-node
search guard. Actual Options plus generated bindings exceeds that old bound.
Frame search now admits8192 nodes/depth32 with a1024-child limit and retains
duplicate-name rejection. The actual lookup body passed a temporary native
audit with901 nodes and ambiguous/deep/excessive-child rejection.

The next user launch hit an access violation after initializing Control_0.
The Proton exception was in the stack, following the new key-name call. Native
logic fixtures could not detect the wrong original engine ABI. The adapter had
mistakenly passed three DWORDs to IGraph+d0; engine1006F6A0 ends in RET8 and takes
only self/scan. At original006AB54F the31 pushed first belongs to the FOLLOWING
006C8D70 conversion, surviving the virtual call. The corrected adapter passes
two arguments and validates RVA6F6A0. The new modifier query was audited too:
IGraph+c8 at1006F5B0/RET8 takes self and explicit poll0, as at006C8548. Both
signatures now match the machine code. Saved crash logs and corrected compiled
call-site disassembly are in the verification directory. This correction is
not yet desktop-confirmed.

The third user launch reached other Options pages, then opening Video settings
reported `Unsupported original text style layout` and tore down cleanly with
result1. The actual geometry/category selectors produce style20 (center/center,
BIG height.04, colorA0000000 in00641E30); that renderer case is now implemented
and audited. The reported border bleed also exposed selector/toggle inset1;
00661B30 initializes their attached text inset to10. Corrected both widget kinds
and binding captions, retaining explicit ruler inset overrides and accepted
static Main labels. The latest build includes these fixes; no desktop pass is
inferred from the failure report.

## Verification and desktop checklist

Strict i386 application/preview/startup targets build. Existing native suite
and158 tooling tests pass. Temporary ASan/UBSan audits cover:

- Actual OptionsFlow plus MenuEventQueue/RuntimeConfirmation: entry, gamma
  disabled, unassigned guard, OK/Escape reply ordering, category/row restoration,
  save/graphics/bindings order, normal Back and reopen without reinitialization.
- Writer/load roundtrip through actual ChunkFile/profile readers and binding
  installer, settings bytes,92 records/padding and failed-open result.
- Mode-overlap/hidden-row/exempt-caption matrix, reserved scans and candidate
  priority/thresholds.
- Extracted actual OptionsGroup/BindingList/BindingRow bodies with native frame
  interfaces: recursive routing, window visibility/layout, capture warm-up,
  assignment, Escape, focus refusal, release and active-capture destruction.
- Actual ruler/underscore glyph generation, anchors/color,500/1000ms clock and
  empty captured value, style20 centering/alpha and inset10; actual frame-search
  body bounds/ambiguity. Partial construction before capture installation also
  destroys safely.

Fixtures establish native behavior, not original DLL execution. No original
game, DLL, Windows fixture or Proton was executed by the agent. No permanent
tests added. Logs and build manifest: ignored
`analysis/work/options-verification-20261031`.

Desktop acceptance still requires launching the corrected staged build, opening
every page, moving sliders/toggles/selectors, both standard layouts, all five
binding categories and scrolling, keyboard/mouse/wheel assignment, cancel and
duplicate reassignment, then Back's unassigned dialog via both OK/Escape. Restore
all bindings, Back to Main, reopen and restart to check isolated persistence.
Also check Main/Exit/Credits/Cinematics returns and clean shutdown. The two user
failure reports are not successful acceptance of any of these cases.

## Follow-up — static text planes, binding scrolling and availability

User reported that most Options behavior worked, but binding command captions
stayed at the top with white rectangles, page headings showed black rectangles,
and binding scrolling was broken. This is partial feedback, not whole-flow
acceptance. Further research found two missing original construction/layout steps:

- Static text constructor0065C680, after00661B30, calls frame+c with direction
  `(0,0,-1)`, sets its pick ID, updates frame+24 and sets attached-text inset1
  before layout. These visuals are back-facing layout/pick planes, not visible
  heading artwork. The native text factory now performs that setup for static
  labels, including the binding row's internal `hktext*` caption. This supersedes
  the prior binding-caption inset10 workaround; editable value fields retain10.
-00655950 writes row LOCAL+B0/B4/B8 and dirty flags directly.00656700 moves the
  group through SetPos using its WORLD+70/74/78, calls UpdateFrame, then refreshes
  visible rows. LS3DF1001C9F0 converts world SetPos arguments through the parent;
 1001E610 propagates dirty transforms/bounds to descendants. The native list had
  mixed local positions with world SetPos and skipped descendant updates. It now
  captures the group's world origin, writes row-local positions with original
  frozen-bit/dirty masks, updates group and rows, then invalidates both input and
  caption text bounds. This passed translated-parent native fixtures, but the
  later screenshot disproved complete correction; see the rotated-parent finding below.

Availability was checked against original code, not overridden to make every
control clickable. Gamma keeps0061BEA0's actual graphics-info+2C bit2 check.
0061F1D0 disables label4EF4 (layered English text “Allow Crosshair”; original frame
name `parental lock`) only when00698500 returns a mission owner. The previous
native definition disabled it unconditionally. Both this control and graphics
featureGD03 now use the actual mission-owner condition; front-end no-mission
Options permits crosshair, while graphics presets still govern feature toggles.
No in-game delivery is inferred from exercising that conditional natively.

All three strict i386 targets rebuilt. Existing native suite and158 tooling tests
pass. Temporary ASan/UBSan audits exercised the actual list and local-position
method bodies with translated parents, caption descendants, every scroll offset,
clamping, selection, key/wheel routing and frozen/dirty flags; the actual static
text initializer's direction/inset/update/invalidation order and repeat behavior;
and both original mission-availability branches. No permanent tests or additional
function credit. Latest logs/manifest: ignored
`analysis/work/options-label-verification-20261031`. Agent only prepared/staged
Windows files; corrected label appearance and scrolling await desktop feedback.

## Follow-up — rotated binding parent confirmed by desktop log

The user reported that rows still overlap and supplied a screenshot. Instrumented
build4aa72b25e13d3717c0c1186b78465b7fec3881d50c3082d428e8574d3167e5d7
then established that dispatch, row selection and coordinate writes run. For
Movement rows0/1/11, requested/local Y was0/-.036/-.396, but world Y remained
.040950 and world Z became0/.036/.396. Input and caption Y bounds were identical
for every row. Flags2000001D/1C excluded a frozen-transform failure. Thus the
earlier translation-only fixture missed the real loaded page's rotation.

The host's generated groups inherited `keysscreen01`'s rotated basis through
LinkTo mode0. Original00655950's local-Y spacing and00656700's world-Y scrolling
therefore used different axes in this loaded host hierarchy. The host now links
each new binding group with mode1, preserving its initial world orientation and
scale, then sets world origin0 and reparents the prototype with preservation as
before. This is an explicit host coordinate-frame adaptation, not a claim that
original0061F5A0 passed mode1: its call at0061F636 passes0. Engine1001D680 tests
mode bit0 at1001D6A6, caches the old world matrix, and computes local coordinates
through the inverse parent at1001D821..1001D85D. This provides the required
world-aligned group without changing asset files or scattering world-position
workarounds through row layout. Tabs retain their existing link policy.

Gamma research also resolves the remaining uncertainty. Original0061BEA0 reads
IGraph+4 and tests +2C bit2. Engine1006DA30 returns100D1D18 (RET4), the graphics
initialization settings; +2C is flags100D1D44. Engine1006C2D0 branches on bit2 for
windowed/fullscreen setup. The host explicitly uses flags80 for windowed startup;
the user log confirms `flags=00000080 fullscreen=0 enabled=0`. Custom quality has
no effect on this gate. Gamma remains disabled intentionally; the bridge now
checks the exact settings getter RVA and logs this condition on Options entry.
Earlier references to this flag as an adapter capability were inaccurate.

Strict i386 application/preview/startup builds pass, as do the existing native
suite,158 tooling tests and ledger validation (415 partial, zero complete).
A temporary ASan/UBSan fixture extracted the actual group-creation lambda,
refresh and local-position bodies. It covered rotated and translated parents,
all category sizes and scroll offsets, constant depth, row/text bounds and the
nine-row visibility window. The previous mode0 variant failed the same fixture.
Temporary audit sources/binaries were removed. No original binary was executed
by the agent. Detailed logs, disassembly and the user's transform log are retained
under ignored `analysis/work/options-transform-diagnostic-20261031`.

Built/staged SHA256:
`4244e2e8c5b4174ff90c164aac3b06775f4b571dd4da1d94f47c8db015fa3128`.
Targeted list/row logging remains to check actual engine coordinates on the next
desktop run. This build still requires user confirmation of row spacing,
scrolling/category changes and binding capture. No new function credit, commit,
desktop acceptance or completed plan checkbox is inferred.

## Desktop acceptance — 2026-10-31

The user confirmed “Options now works as intended” for build
`4244e2e8c5b4174ff90c164aac3b06775f4b571dd4da1d94f47c8db015fa3128` and requested completion and commit.
Front-end Options and its shared runtime dialog integration are accepted.
This records the user's overall flow acceptance; it does not invent separate
case-by-case desktop results. The actual log confirms vertical row spacing
(world Y changes by.036, depth stays fixed), category/scroll events and clean
teardown. Options persistence result0 remains an actual ignored write failure,
not proof of a successful profile write; selected-profile and filesystem
persistence trials continue in the next profiles/saves milestone. Native
serialization roundtrip/failure coverage remains as recorded above. In-game
Options and every other shared-dialog caller stay open.
