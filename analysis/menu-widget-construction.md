# Options widget internals and concrete settings wiring

The complete shared-foundation checkbox for kinds8/10/24 is implemented and
native-audited. **Options widgets are source-only.** Actual engine/input/text
adapters, owned internal parts, flat/recursive factory construction and all four
settings observers now compose. Main/Exit retains its accepted button semantics;
no Options navigation/page/apply/back flow is enabled. That delivery remains in
plan section3 and cannot be established by this source milestone.

The prior [settings/cache integration](menu-caches.md) is user desktop-confirmed
and committed as `9a5e88a`. Its841-frame state-exit/result0 logs/window.cfg and
matching build manifest are archived under ignored
`analysis/work/menu-cache-acceptance-20261031`.

[Mixed evidence map](menu-widget-construction-map.json): both frozen original
fingerprints,46 instruction anchors,126 virtual slots (117client/nineengine),
122 direct calls. Reuses confirmed [widget state/input/observer](menu-options.md),
[current profile/cache](menu-caches.md), [factory/container](menu-control-factory-map.json)
and [frame sequencing](menu-update.md) contracts.35 new valid read-only Ghidra
exports cover internal parts, geometry, input and owner cleanup; three existing
constructor exports are reused. Original binaries/project are unchanged.

## Entire completion boundary

- `BuiltOptionsWidget` owns arrows/thumb or toggle halves and the existing
  SliderControl, SelectorControl or ToggleControl runtime. It supplies their
  previously missing concrete backend operations.
- `MenuWidgetVisuals` calls guarded real engine methods and uses the menu's shared
  input, real attached MenuTextObject, current profile and actual event queue.
- `MenuControl`, flat/recursive factories, screen callbacks and the owning menu
  carry that shared context. Nested slider enable checks resolve the actual
  parent control's disabled state. Full internal-part target bits survive input;
  ordinary buttons keep the accepted target canonicalization and callbacks.
- Typed settings observers bind only the named OPTIONS family6 and its confirmed
  widget IDs. Other screen families can supply explicit bindings; they do not
  accidentally inherit preference writes from matching low ID bits.

All supported widget +78 definition-data slots are verified0065CD30 no-ops.
Actual +2c construction precedes definition initialization. Range/item records
run before the context attaches the confirmed Options observers, matching those
builders' range/items-before-observer ordering. Original raw callback addresses
are never executed. Unsupported kinds still fail. Missing surrounding frame/text/
input/settings owners cannot be replaced by successful empty services.

## Constructors and owned parts

| Entry | Construction |
| --- | --- |
| 00664B90 | Slider: base frame/pick ID; arrows IDs parentOR1/2; optional thumb parentOR3; listeners for each arrow then parent; initial parent disable |
| 0064F1C0 | Selector: attached text; same two arrow IDs; previous/next bindings; text layout and initial disable; no arrow-listener registration here |
| 006541E0 | Toggle: attached text/layout, require seven child frames, allocate two four-state halves IDs1/2, initialize yes selected/no unselected |
| 00663FE0 | Arrow: require child count3, find normal/actived/inactived, bind pick IDs and align alternate positions |
| 00664360 | Thumb: find normal/actived/sliderbase, bind real input, validate frames, align active, hide sliderbase, compute travel and set50percent |
| 00663B60 | Half: bind four frames; show empty, hide full/active-full; assign three initial pick IDs/positions |

Arrow roots use `barrow up*` / `barrow down*`, flags10020. Their children use
`normal*`, `actived*`, `inactived*`, flags10001. Thumb root is `slider*`,10020;
its children use normal/actived/sliderbase wildcards,10001. Native guards reject
missing arrows/children where original paths could dereference null. Thumb is
optional; original arrow-only refresh remains real, while querying an absent
thumb throws explicitly.

Slider arrow press bindings are parent/05006023 and parent/05006024: tagged
04006000 emits queued01000023/24 to the parent. Thumb uses parent/05008025;
04008000 emits queued01000025. It is then explicitly set to zero percentage.
Parent initial disable precedes range initialization. Selector arrows use
0600601B/1C, producing queued0200001B/1C.

The toggle's seven frame names are yes_empty*,yes_full*,yesactived_full*,
no_empty*,no_full*,noactived_full*,noactived_empty*. **Both halves use the same
noactived_empty frame as their fourth active-empty visual.** This is verified
from006543CE–006543EB and006544C5–006544E2 stack arguments, not inferred as eight
independent frames. Construction touches only empty/full/active-full; shared
active-empty pick ID/position is repaired when that half uses it.

Alternate position copies preserve the original bit80 guard, copy local+b0/b4/b8
and rewrite flags `(flags & FFFFFEDF)|40000000`. Engine methods and models own
frames; native internal UI products borrow them and release no invented engine
references. Part owners are destroyed in reverse insertion order. Selector
item storage is released before its internal arrows.

## Arrow timing and visual state

00664140 gates focus on disabled, selects active and updates inactive/active/
normal.006641C0 clears repeat; enabled leave selects normal and updates normal/
active/inactive.00664220 always stores the disable byte, selects normal when
zero/inactive when nonzero, hides active and updates all three.

00664270 enabled press arms repeat, stores threshold300 and emits04006000.
006642A0 release clears repeat.006642B0 compares that threshold as a signed DWORD
to **shared input idle+40**, not elapsed frame time or click timer+3c. If due and
no button is held, it disarms and resets300. If due and any button is held
(selector0), it storesFFFFFFFF and emits04006000. The negative threshold makes
subsequent eligible listener updates emit again.006641A0 click emits04012000,
returns1 and does not perform ordinary base activation.

Each slider arrow and the parent queue0200007A registration, in constructor
order. Thumb is not a listener. The existing reverse-listener update phase runs
these after the single queue drain. Events emitted there wait for the next drain.
The parent slider's ordinary focused +1c update also remains present; it is not
suppressed merely because the object is a listener. Selector/toggle +1c are the
verified no-op0064DA20.

Native listener identities use full control IDs rather than truncated pointers.
Weak lifetime guards prevent calls into destroyed products; cancellation clears
the registry callback, and the menu context removes its bindings before queue
and input destruction. A callback is retained across its own update. Original
00657D70/006602C0/00660170/0064F410 show reverse child and string/text/collection
cleanup, without a successful invented per-widget unregister service. The native
context cleanup is an explicit lifetime adaptation, not original pointer ABI.

## Thumb travel, drag and percentage

00664570 uses the **frame world box+10/+14/+20/+24**, refreshed via frame+58
unless flags100 is set. It does not substitute visual bounds+170. Half normal
width/height inset the sliderbase box, then X is clamped0..1 and Y-.75..0.
Subtract normal local+b0 from X and+b8 from Y, preserve signed travel extents,
and choose vertical when width<height, horizontal otherwise. Do not normalize
or reorder negative/inverted vertical extents.

006649F0 compares requested percentage to the stored float, clamps0..100 and
maps horizontal left+width*percent*.01f or vertical bottom-height*percent*.01f.
Sequential original comparisons clamp the coordinate.006647F0 sets root
position through frame+08 then updates+24.00664980 computes the inverse
percentage and reports whether its stored float changed.

00664810 checks only selected-axis relative motion.0066A120/150 normalize
actual shared cursor/motion to x/width and -y/height*.75; root world position is
read after the real matrix refresh. The coordinate is cursor minus normal local
+b0/+b8, sequentially clamped, then root position/percentage are updated.
00664540 emits04008000 only on percentage change.006647A0 supplies direct set/
query messages01000021/22 through the owned thumb, preserving mutable query
output instead of copying it away.

006644A0 enabled focus switches active and emits04003000, but returns1 even
when disabled.006644F0 leave unconditionally restores normal and emits04004000.
00664310 disable hides both; enable shows normal without forcibly hiding active.
Programmatic sets are silent; drag queues the parent's value-change path.
Native nonfinite/zero-division guards throw instead of undefined conversions.

## Toggle halves and layout

00663EE0 stores the inverse selected/empty byte. Focused selection swaps only
active-full/active-empty; unselected focus repairs the shared active-empty
frame's picking/position. Nonfocused selection swaps empty/full.00663C70 gates
already-focused/disabled enter;00663D50 gates leave.00663DD0 only swaps active
visuals while focused and does not change the parent's value or notify it.
00663B00 disable equality-gates alpha0.4/1 on empty/full only.

Parent01000099 follows00663EA0: detach all four frames using LinkTo(null,1),
set parent direction(0,0,-1), then reattach all four to parent with mode1.
The shared fourth frame participates in each half's original calls. General
paragraph layout/original text class ABI remains the established boundary;
selector/toggle hover uses the existing bounded single-record text state.

## Concrete settings composition and factory behavior

The four recovered observers now run against OptionsSettings<StartupSession>,
which resolves the current selected profile each operation and applies actual
startup-owned graphics/gamma/sound/music/mouse/voice services. The native product
wraps its actual runtime sender for query/ID access; observers retain their
confirmed ordering and silent preset updates.

Family6 confirmed local IDs: gamma604000, sound706000, music707000, mouse806000,
voice-volumeB06000; geometry605000; features606000..60C000; invert804000,
subtitlesA02000,parentalA03000; voice-enableB04000. Connection selector B02000
has no fabricated settings observer. Preset values queue seven silent updates,
then seven availability updates and apply. Feature toggles only store a byte in
the current quality block. Voice still stores a low byte but tests the full
argument to select volume availability. Sound changes preview the actual click.

The heterogeneous leaf product preserves integer callback returns (including
slider base activation7) and full part targets. Flat/recursive factories accept
8/10/24 only when real widget/text/input context exists. Recursive slider
re-enable resolves its live parent; source-only failure paths reject malformed
construction without publishing a successful widget. Existing buttons retain
normal/active behavior, original pointer/activation calls and canonical root ID.

## Verification and limits

Temporary native ASan/UBSan audit instantiates the **actual host MenuControl,
MenuControlTreeBackend/DefinedControlTree, MenuWidgetContext/MenuWidgetVisuals,
BuiltOptionsWidget/part headers, original control/event queue helpers and
OptionsSettings**. Engine methods and scene frame lookup are supplied native
stand-ins; no original DLL or Windows binary is executed. The audit checks:

- Real three-part construction, IDs/visibility/alignment, mutable thumb query,
  queued press/drag, signed idle arrow repeat and original wheel rounding.
-10,000 generated positions, independent analytic travel inverse, profile switch,
  all five float settings, all seven feature widgets, preset availability/no
  feedback, all three byte toggles and voice-enabled volume routing.
- Listener registration order, callbacks after product destruction, reverse
  lifetime cleanup, malformed/incomplete frame rejection, arrow-only query
  failure and actual recursive parent-disable/re-enable guard.
- Actual ordinary button visuals/callback results and target canonicalization,
  plus rejection of automatic Options observers on another screen family.

The same fixture links for Windows i386 without execution. All three strict
Windows targets, existing native suite and158 tooling tests pass. New/reused
maps, deliberately corrupted map rejection and frozen accounting pass.
Temporary audits are removed; no permanent tests are added.

23 eligible client partial bodies are added: **369 partial (364client/fiveengine),
zero complete, frozen18,783**. Absent part methods/delegated engine helpers get no
credit. No class allocator/SEH/vtable ABI, original general paragraph renderer,
complete options builders/navigation/apply/back/persistence, bindings editor or
new desktop Options behavior is claimed. Source-only completion needs no
invented desktop test; any later enabled flow needs actual user acceptance.
