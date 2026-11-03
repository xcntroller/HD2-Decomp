# Original resource paths and fullscreen — 2026-11-02

User-requested Phase1 follow-up before closing the phase: remove redundant
development resource staging and implement fullscreen. Source/native verification
is complete; **revised package resource loading and fullscreen are user desktop-
accepted2026-11-02**. The earlier portable package remains accepted and preserved.
No Phase2 planning, commit or push is included in this follow-up.

## Completion boundary

The normal application and newly assembled portable package use original model,
sound and texture archive paths, plus the installed original Text tree. They
require no Models/Sounds/MenuText directory, Maps extraction or renamed font
atlas. Opening/subtitles, Main/shared screens, Options, Profiles, Credits,
Cinematics and the existing preparation/network boundaries share this policy.
The separate engine-window diagnostic retains its prior aliases/stagers. Old
development workspaces and accepted packages are not pruned or overwritten.

Fullscreen is an explicit per-launch exclusive mode at the requested resolution,
32-bit color/default refresh. `--fullscreen` and `--windowed` are mutually
exclusive switches; default800×600 windowed and legacy ENGINE --frames syntax
remain supported. No in-menu mode selector, runtime Alt-Enter switching, setup
registry writes or invented display-mode persistence is added.

## Original evidence

[Static map](resource-fullscreen-map.json) validates both frozen original
fingerprints,14 instruction anchors, three calls and two font-path references
using tools/validate_engine_host.py. The validator's short console summary does
not print client resource counts, but validates those entries too.

| Original address | Contract |
| --- | --- |
| 006DB0F0 /006DB1CF | Client copies dimensions/color/refresh into graphics initialization and ORs flags2 for fullscreen. Other setup/presentation flags are independent. |
| 1006C2D0 /1006C3E5 | Engine tests flags2, clears child-window/menu flags, zeros placement/child rectangle and creates the fullscreen window/device. Unsupported display modes fail; no native success fallback is substituted. |
| 1006DA30 /0061BEA0 | Initialization settings getter and original Options gamma gate; flags+2C bit2 enables gamma. Existing gate now naturally reflects the selected mode. |
| 1006B3E0 /1006B4ED–1006B548 | Original Present detects DEVICELOST, tests cooperative level and delegates reset to1006BE50. Engine reset, device resources, focus/input and final device/gamma cleanup remain delegated. |
| 0063DC50 | Original font manager builds Text/Fonts/{Ansi,Ce}/SetN paths and independent Text/Fonts/SYMBOLS paths. Existing width/language selection is reused unchanged. |
| 00640310 | Original loader passes the original atlas path to IGraph+3C and reads metrics through dtaOpen/Read/Close. Native metrics still use the original loose Text files; full original font ABI is not newly reconstructed. |
| 006DB6E0 /006DBE60 /006DBFC0 | Already integrated archive setup and root registration; roots need not exist physically. See [archives](startup-archives.md), [directories](resource-directories.md). |
| 004290A0 /0044A1C0 | Already integrated model/sound cache loads through registered directories and actual engine loaders. See [caches](menu-caches.md). |

Existing exports under analysis/work/mission-boundary, graphics-host,
menu-text-styleabi and menu-text-fontabi are reused. Focused read-only PE
disassembly checked the fullscreen and reset branches; no original code ran.
The original texture loader chooses texture encodings through its delegated
implementation. This change does not decode/rewrite .555/.565 or archive DX
textures and makes no new claim about those cache formats.

## Implementation and lifetime

Configured menu_font_texture now returns the original path paired with each
selected metric file. Symbols use Text/Fonts/SYMBOLS.{tbl,tga}. The ordinary
localized menu already receives StartupSession's actual text cache, so its
diagnostic labels.bin fallback is unused. Historical diagnostic defaults remain.

stage_menu_presentation(... aliases=False) validates all12 ANSI/CE atlas/metric
pairs plus SYMBOLS in the staged original Text tree without writing aliases.
Missing/different files, malformed original font payloads and linked destinations
fail before publication. Existing alias staging remains the default for earlier
diagnostic callers. The application launcher and package builder stop invoking
menu extraction, diagnostic text and Credits-alias stagers. Package validation
rejects accidental Models/Sounds/MenuText/Maps directories, including empty ones.
This applies to clean new assemblies; existing user workspaces retain old files.

fullscreen_settings preserves the verified56-byte ABI and uses flags2, zero
placement/child rectangle,32-bit color/default refresh. The existing engine
owns initialization and rejection/cleanup. The selected mode and failed engine
status are logged. Fullscreen does not overwrite window.cfg with fullscreen
coordinates; normal windowed persistence is unchanged.

fullscreen_frame_ready is native frame policy, without original-body credit:
pause draw/update submission while not foreground, iconic or DEVICELOST; call
original engine Present when DEVICENOTRESET, recheck and resume only when ready.
The Windows adapter uses the same verified borrowed Direct3D device bridge as
existing text rendering. Unexpected HRESULTs propagate to existing launch owner
cleanup. No native Direct3D Reset or successful fake recovery is provided.
The CLI rejects repeated/conflicting display switches. The Python launcher
forwards them, and the portable LaunchFullscreen.cmd forwards to Launch.cmd.

## Verification and review

Revised portable folder: `dist/HD2-Phase1-clean/`, entries HD2FrontEnd.exe,
Launch.cmd and LaunchFullscreen.cmd. ZIP: `dist/HD2-Phase1-clean.zip`,
2,028,879,915 bytes,493 payload files (61 fewer than the accepted earlier package).
Client SHA256 `745174a2fc7f3cf6d23253d49c2ffb76faa63a9fe6f0eba57f463a375e926573`.
ZIP SHA256 `a95a1396230231c035a94be4dd892e74845780d28a54c65bc226f68700be383e`.
The current source build matches the revised packaged client. Accepted earlier
dist/HD2-Phase1 client3547347c…6070b and staged client e11fb644…384a8 are preserved;
neither is overwritten or claimed identical to the revised build.

- Strict i386 application-launch, engine-window and startup-probe builds pass.
- Temporary native ASan/UBSan audit runs actual parser/settings/font-selection
  and frame-guard code: default/legacy/mixed switch order, missing/repeated/
  conflicting/overflow arguments; all byte languages, independent CE fonts,
  all width thresholds and original metric decoding; inactive, ready, lost,
  successful/repeated/failed reset and exception propagation. The same fixture
  strictly compiles/links for Windows i386 without execution.
- Temporary Python audit exercises actual no-write font validation, missing/
  modified/linked resources, historical diagnostic staging, clean package/ZIP
  assembly, accidental extract rejection, no-overwrite/failure cleanup and
  stage-only display-switch forwarding. Synthetic dependency/media boundaries
  are patched only within the fixture; real font staging and package logic run.
- Existing native regressions,158 Python checks, dependent presentation map,
  original evidence map, frozen ledger and whitespace checks pass. No new
  function credit or permanent tests. Detailed logs and source hashes are in
  analysis/work/phase1-resource-fullscreen-20261102.
- Real revised artifact: every folder/ZIP payload matches SHA/size/CRC, ZIP paths
  and empty directories match the folder, all seven DLL imports resolve to the
  local bundle or declared platform imports,13 original font pairs validate,
  helper directories are absent and user state starts empty. All581 pre-recorded
  original/accepted/writable file identities are unchanged, including the old
  ZIP. Artifact verification is in that audit directory's artifact-verification.json.
  Temporary audit sources/native/Windows binaries were removed after recording
  their hashes; no original code, Windows fixture or Proton was executed.

User desktop review must use the **fresh revised folder**, with no copied Models,
Sounds, Maps or MenuText from the old package. Check opening → Main/create a
profile, translated fonts/symbols, Options/click sound, Cinematics/subtitles and
Credits. Run LaunchFullscreen.cmd or
`HD2FrontEnd.exe --fullscreen --width 1024 --height 768`, then verify pointer alignment,
gamma, Alt-Tab/return and Exit/desktop restoration. Reopen windowed for comparison.
Use a driver-supported resolution; unsupported modes should log failure and exit.
These are requested checks, not claims that every desktop case was performed.

Full game/load/save/results remain Phase2; loadouts remain completely untested
until accessible; ready-server UI/services remain untested until Phase3. Broader
save metadata is open. This follow-up does not widen their accepted boundaries.

## Desktop acceptance — 2026-11-02

The user confirms fullscreen works through the Python launcher, then resolves
the reported package startup problem and states: “Nevermind, I got it working.
Good, that's done.” This accepts the resource/fullscreen follow-up and closes
its section5 checkbox. No root cause for the initial launch issue was supplied;
no startup logging fix or runtime change was implemented during that inquiry.
The suggested review cases above are not separate claims of executed checks.

At the user's intervening desktop run, the compiled, development-staged and
clean-packaged clients all match745174a2…26573; the earlier e11fb644 staged
identity above describes the preservation snapshot at package assembly time.
The observed development log records fullscreen2560×1440, Czech/CE Set3 and
normal state-exit/teardown result0. That log supports the script run; the user's
statement supplies package acceptance. No agent desktop execution is claimed.
Package creation-time pending-review metadata is historical and superseded here;
the accepted ZIP is not rebuilt for this acceptance record. Function counts
remain564 partial/zero complete, with no commit or push requested.
