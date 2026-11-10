# Loading presentation — P2.2 source work

2026-11-04. This is part of the full P2.2 checkbox, not a delivered loading
screen or a smaller milestone. The game initializer still declines loading.
`game/loading_screen.h` and `host/game_loading_screen.h` now compose the
single-player presentation owners and real engine/graphics/font endpoints.
[Concrete loading allocation](mission-loading-integration.md) now supplies the
source loader and bounded real GEffects update. Flag170 belongs to SAME
application24 GameConfiguration; positive external transport remains required.

## Original behavior

006D3F20 calls006C9640 with resource, mode bit2 (scene enabled), low mode bits==7
(Space wait), true (retain through activation), and the actual descriptor.
006C9640 first closes the prior screen, resets progress and stores the two flags.
When enabled, create a separate kind13 scene and match the physical viewport.
Read driver states15/19/11 as booleans, set0/1/1, open
`missions\<resource>\loader` through scene+F4 with flags0/callback0/userdata0,
then restore those three saved booleans. A negative open releases the scene and
returns with availability false. It does not fabricate a successful loader asset.
Native exceptions restore every acquired state and discard the acquired prefix.

Create a kind3 camera and activate it in this loading scene. Its direction is
(0,0,1), position(.5,-.375,-15.5), range(.2,500), orthographic mode1/extent2,
followed by update. Create kind2 light `C_LoaderScreen`, type4, RGB(1,1,1),
intensity1 and flags11, and add it to the primary sector. The scene's camera
reference and sector's non-owning light membership follow the existing engine
contracts. Assembly006C9974..006C997C pushes **both intensity1 and self**; the
first repaired decompilation omitted intensity and corrupted later stack aliases.
`loading-screen-abi-v2/` corrects that call and restores the later this pointer.

Walk the fifteen00842FF8 text descriptors in reverse, query scene+7C(maskFFFF),
hide each found visual and create its actual font/text owner. Inset is5; bind
world bounds, clear records, then populate in original order. Captured Tutorial/
Alps1 loader bytes contain `briefing`, `title`, `textbar`; all eleven MP strings
and the optional version string are absent. The actual engine still determines
which frames exist; byte presence is only structural research, not a frame query.

For the actual kind3 single-player mission, briefing uses original+40
(`words40[0]`) and title+44 (`words40[1]`), skipping FFFFFFFF. Do not use the
request's menu caption or the resource name as a title substitute. Tutorial is
10450/4741; the second captured descriptor is spelled **Alps1**,10340/4631.
Keep that selected spelling; uppercase capture filenames do not rewrite it.
Textbar starts empty. Optional version uses00644200 style40/ID2006's existing
formatter. MP descriptor/list/objective producers remain unsupported and reject
if required, rather than returning empty successful text.

Query `loadbar`, draw twice, clear the two pending-name vectors while retaining
capacity, then set availability. Availability also becomes true when scene
creation is disabled.006CA2C0 reads that byte, not scene existence.
Engine FindFrame10046920→1001DF00 returns a borrowed pointer without addref.
The original consumes the loadbar's existing engine reference in006CA2D0 before
scene release; the dedicated `take_loading_bar` bridge matches this transfer.
It must not be generalized into ownership of every FindFrame result.

006CA470 pumps messages first. Actual application+170 selects00731720's external
percentage branch (truncate progress×100), otherwise a present scene scales
the bar to(progress,1,1), renders scene+78(0), saves graphics state0 through+78,
sets it1 through+74, begins text, draws labels14..0, ends text, restores state,
presents, then clears(0,1,3). The native bar preserves its unused fourth scale
lane rather than copying an original uninitialized stack word. Native draw
failures still end an acquired text phase and attempt state restoration.

006C95B0 closes only when flag6 is clear;006C95C0 closes only when it is set.
006CA2D0 sets progress1 and draws when available. For flag5 with a scene, invoke
actual GEffects+10(100), set textbar ID2040/style80, then repeatedly pump messages,
read graphics+9C keys, draw, and poll graphics+A0 until scan39 (Space). Clear the
prompt, clear/present three times, then destroy labels14..0, light, loadbar,
scene and camera, clear pending names, and reset progress/availability. Normal
explicit close retains this interaction; native destructor/failed-prepare cleanup
only discards acquired owners and never initiates an interactive wait.

## Text and engine boundary

`host/loading_text_geometry.h` reuses the recovered glyph, paragraph, clipping
and actual font metrics bodies. Style80 is font2, height bits3D3851EC (.045),
center/center. Style84 is font3, height3CB43958 (.022), justified/top with word
wrapping; its last paragraph row retains ordinary spacing. The shared paragraph
helper now accepts the normalized height, preserving .02 for existing callers.
The optional style40 uses the existing version left/bottom layout. Original
unsupported MP styles reject; no renderer or placeholder font is fabricated.

The host opens the actual scene and uses the original checked virtuals/exports,
world-bound rectangles, font cache and D3D device. Font resources are acquired
before binding geometry, and labels clear their text before matching font-owner
destruction. Glyph drawing runs inside the outer original begin/end text phase,
restoring its viewport/render states after submission. It does not create nested
graphics begin calls. Engine methods remain delegated and earn no new credit.

## Verification and remaining work

Temporary ASan/UBSan audit passes prepare/finish/Space/render ordering, boolean
driver-state restoration, complementary finish gates,127 injected prepare/close
endpoint failures and acquired-prefix/normal destruction. Engine, font, input,
GEffects and external-progress endpoints in this lifecycle audit are fixtures.

A separate native glyph audit passes28 actual Tutorial/Alps1 captions across
seven installed byte languages,140 layouts at640×480,800×600,1024×768,1280×720
and1920×1080 using actual ANSI/CE font metrics. It checks font selection,
.045/.022 glyph heights, clipping, disabled colors, version layout and seven
unsupported/mixed-style guards. Rectangles are fixtures; this establishes no
authored-frame or desktop geometry verification. Strict i386 template
instantiation passes against GameLoadingScreenBackend. Existing native
regressions pass. Windows targets compile/link only; none are run.

Evidence/logs are under `analysis/work/mission-world-20261103/`:
`loading-screen-full.asm`, `loading-screen-render.asm`, `loading-screen-abi-v2/`,
`loading-find-frame/`, `loading-screen-table.json`, `loading-text-style-values.json`,
`loading-sp-caption-fixtures.json`, `loading-screen-native-audit.log`,
`loading-text-native-audit.log`, `loading-screen-i386-audit.log`,
`loading-screen-native-regressions.log`, `loading-screen-windows-build.log`.
Original client/engine fingerprints remain those in the mission loading contract.
Temporary audits are fingerprinted before removal; no permanent tests are added.

Continue the actual runtime/module and full actor/loading/activation composition,
including Car callbacks. No successful loader, first world frame, new frozen
function credit, desktop verification, package, staging, commit or push follows
from these source-only owners and native fixtures.

Required mission HUD source continuation: [HUD loading contracts](hud-loading.md). Cursor and99 texture borrowers verified; actual HUD startup/panels/teardown/render remain.
