# Phase 1 presentation fidelity — 2026-11-02

Section4's first checkbox covers the delivered front-end layout, draw order,
clipping, font/localization selection and resolution-dependent coordinates.
The user explicitly requested **“Add other resolutions and languages now”**
instead of restricting this check to English at800×600. Implementation and
native verification are complete. The user has accepted localization and the
[team-removal/widescreen corrections](team-removal.md); section4's first checkbox
is complete. Main traversal and its four Options
corrections were already [accepted](main-traversal.md#desktop-acceptance--2026-11-02).

## Completion boundary and dependencies

This is one presentation checkpoint across existing reachable menus, shared
dialogs, Options, Profiles, Cinematics/subtitles, Credits, the bounded team screen
and reachable multiplayer front-end. It includes text/picking/drag coordinate
agreement, list row heights, translated glyphs, clipping, retained modal
backgrounds and cursor ordering. Existing artwork and engine model transforms
remain the geometry source, with the bounded menu camera aspect correction
documented in the follow-up above.

Required dependencies are the actual graphics dimensions; installed loose text;
the original ANSI/CE font metrics and atlas sets; existing scene/frame bounds;
and the established startup language, localization and archive owners. No
replacement fonts, invented translations or successful service placeholders are
introduced. Gameplay/loadouts remain Phase2; ready-server-dependent UI remains
untested until Phase3. Dimensional changes in those shared adapters are compiled
source work and do not establish their desktop delivery.

## Original evidence

Client fingerprint remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
[Static map](menu-fidelity-map.json) checks10 instruction anchors, three direct
calls and four resource references against that exact file, without execution.
Existing cached exports are reused; confirmed subsystems were not re-recovered.

| Original body | Contract used here |
| --- | --- |
| `0063DC50` | Query render width; Set1 below800, Set2 from800 through1023, Set3 from1024. Czech/Polish useCE, other installed western byte languages useANSI. CE fonts5/6 and SYMBOLS4 are independent resources. |
| `00640310`, `006404C0` | Load atlas and256 five-float metrics; use original glyph aspect/UVs and rounded advances. |
| `006B67B0`, `006B6750`, `0048FBC0` | Cached installed language, language comparison and text directory; Japan/Russian or FontConfig request wide mode. |
| `006B1090`, `006E4CF0`, `006E5240`, `006E4FB0` | Loose localized-text loading, original byte parser, first-ID-wins cache and lookup. |
| `00644A80` | Render width/height and world bounds determine text rectangles; horizontal inset scales with width, Y uses height×4/3 and the original half-pixel offset. |
| `00641E30`, `00645270`, `00645F20`, `00646A10` | Authored styles, owner-width column caps, paragraph wrapping and measured glyph layout. Ruler styles27–29 use primary small font3, not an unconditional CE font. |
| `00646940`, `006469C0` | Block height and first-row leading; retain style10's accepted title baseline. |
| `00647EE0`, `00647D00`, `00648860` | Intersect the viewport, skip empty intersections and submit glyphs; byte and wide character routes differ. Native clipping interpolates glyph UVs to enforce the same rectangle bounds. |
| `0066A150`, `006937F0`, `00693B60`, `00693C10` | Normalized pointer/frame coordinates and team drag displacement/drop; existing recovered paths now receive actual dimensions. |

Exports: `analysis/work/menu-text-styleabi/0063dc50.c`,
`menu-text-fontabi/00644a80.c`, `menu-text-renderowner/00647ee0.c`,
`menu-text-styledefs/00641e30.c`, and
`credits-layout-research-20261031/00645f20.c`, all under `analysis/work`.
Existing [scene map](menu-scene-map.json), [text map](menu-text-map.json),
[construction contracts](menu-widget-construction.md),
[Credits contract](credits-flow.md) and [team evidence](singleplayer-pipeline.md)
retain their original ABI and ownership boundaries.

## Implementation

`MenuPresentationSession` owns one launch's dimensions/language and restores the
previous diagnostic context on teardown. The application passes dimensions to
graphics creation, cursor initialization, text/subtitle viewports, object bounds,
pointer transforms, engine picking, profile selection and drag/drop adapters.
Existing authored pixel row heights scale from their600-pixel design surface;
normalized font heights round against the actual render height.

The launcher accepts `--width`, `--height`, `--language` and `--list-languages`.
Default size remains800×600. Without `--language`, the original registry query
selects the language; an explicit selection overrides only this launch's cache
and does not write the installation registry. Dimensions are bounded to
320×200 through16384×16384; native verification samples common sizes and exact
font thresholds. This is per-launch windowed rendering; live resizing,
fullscreen setup and a new language-selection screen are outside this change.

Installed menu translations: **Czech, english, EnglishUS, french, german,
italian, spanish**. Original ANSI/CE bytes pass directly to the corresponding
256-glyph fonts. Metrics and textures select the same family/SetN for menu,
subtitle and Credits renderers; Credits retains its separate CE style slots.
The diagnostic preview keeps its earlier resource aliases and defaults.

`stage_menu_presentation.py` validates all12 ANSI/CE BIG/SMALL atlas/metrics
pairs across Set1/2/3. It publishes immutable texture aliases in the isolated
workspace; startup staging already copies the original Text tree. It validates
all destinations before writing and refuses differing existing aliases. Original
512-pixel TGA metadata includes a footer without the usual finalNUL; both that
original variant and standard footers are validated. Profile/config copies keep
their existing preservation policy.

General attached text, profiles, columns and subtitles clip glyph quads and
interpolate UVs. Style10 retains its evidenced leading offset. Oversized
translated words advance by at least one glyph and wrap within the available
width; overflow below a text rectangle is clipped rather than crashing. This
bounded native handling is not a claim that every original paragraph-layout
quirk or the complete original text-object ABI is reconstructed.

The audit exposed integer glyph coordinates crossing a fractional column edge
at799-pixel width. Columns now clip all four sides instead of only their final
row's bottom. Modal rendering retains the established background scene/text →
overlay scene/text → drag text → cursor order. The modal pass restores saved
frame visibility and driver state on normal return and exceptions. Dropdown
text masks exclude intersecting background glyph fragments before overlay text.

The user confirms Japanese localization was unfinished; its missing main file
is expected. Japanese assets here contain only `TEXTY_DD.txt`, without `TEXTY.txt` or
FontConfig. Polish/Russian translations are absent. Wide/GDI font configuration
and Japanese metrics have a different dependency/format and remain unavailable;
selecting an unavailable language fails explicitly. Only LangEnglish.dta is
installed; the existing recovered archive owner preserves the original English
archive fallback while loose text supplies the selected translation. This does
not establish localized speech archives or broad Unicode input support.

## Verification

- All three strict i386 Windows targets compile/link: application-launch,
  engine-window and startup-probe. No Windows fixture is executed.
- Existing native suite and all158 Python tooling tests pass. Frozen ledger
  check passes: **564 partial (559 client/five engine), zero complete /18,783**.
  This adapter work adds no function credit.
- Temporary ASan/UBSan audit composes the actual production submission collector,
  geometry/parser/font code and modal-pass body. Visual/device services are
  native fixtures. It uses real installed font metrics and localization data.
- Matrix: seven installed languages × nine modes:640×480,799×600,800×600,
  1023×768,1024×768,1280×720,1280×1024,1920×1080,2560×1440. It checks3843
  Main/confirmation/Options captions,35289 subtitle layouts (including installed
  movie/briefing cues), byte preservation, paragraphs, profile rows, columns,
  rulers, glyph bounds/UVs, font family/SetN, cursor bounds/frame coordinates,
  Credits, nested modal restoration and failure cleanup.
- Temporary Python audit checks real12-pair staging, digests/idempotency,
  changed-output preflight, symlink rejection, preserved profile/config files,
  unavailable languages, dimension bounds and malformed TGA rejection. Original
  font assets retain their digests.

Logs: `analysis/work/menu-fidelity-20261102/{sanitizers,staging-audit,evidence,
windows-build,native-tests,python-tests,function-progress}.log`.
Temporary audit generators, source and binaries are removed after verification;
no permanent tests are added. Native fixtures establish bounds and ownership,
not actual D3D rasterization, artwork alignment, translated speech or exhaustive
desktop coverage. The original executable/DLLs and Proton are never run by the
agent.

## Desktop acceptance — 2026-11-02

The user confirms: **“All fixed and works as expected. Record as complete,
commit and continue.”** This accepts the presentation checkpoint and all four
reported Character Select/widescreen corrections against executable SHA256
`bb8cbe802887e64d25f7ea8f4c700f47faaa822730c1c0214e26239331340071`.
Localization was confirmed separately in the preceding feedback. The requested
commit is authorized; no push is requested.

[Machine-readable acceptance](menu-fidelity-acceptance-20261102.json) preserves
the user quote, executable and limits. This is the user's overall confirmation,
not a claim that every suggested resolution/language/case below was individually
executed. Source-only loadouts, game/save receivers and ready-server UI remain
excluded. The next checkbox is held/repeat input, eligibility, focus, wheel/drag
and modal cancellation.

### Review instructions retained for provenance

Run the normal user launcher with the desired flags, for example:

```sh
python3 -B tools/run_application_launch.py --width 1280 --height 720 --language Czech
```

Compare English800×600 with640×480,1024×768 and one widescreen mode. Check Main
title/buttons, Options captions/binding capture, profile list/edit dialogs,
retained modal backgrounds, Credits and subtitle placement. Inspect Czech
diacritics and a long-caption language such as German/French; confirm picking,
scroll selection and drag positions still agree with artwork. Check dropdown
occlusion where reachable. Preserve the previously accepted Options hover rules.
The acceptance above closes this review; no additional case-level execution
is inferred from the overall confirmation.

The staged executable hash and preparation commands are recorded in
[the handoff](../HANDOFF.md).
