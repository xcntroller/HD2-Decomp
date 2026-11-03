# Game owner construction and controller integration

2026-11-03. P2.1 construction is implemented and integrated, with native/build
verification complete. **User accepted the bounded unloaded-construction/error-return
checkpoint2026-11-03, confirming the constructor log. Both P2.1 checkboxes are closed.** This supersedes the missing-backend/controller limits
in the [initial owner checkpoint](game-owner-start.md), whose verified loading
boundary and Tutorial selection remain valid. [Evidence](game-owner-integration-map.json),
[function/verification index](game-owner-integration.json).

## Observable behavior and ownership

Continuing from the ordinary Tutorial team screen now reaches the actual kind7
receiver. Previously `PreMissionMenu::advance_setup` displayed the unavailable
prompt before the controller could attempt game construction. It now requests
status2; the existing descriptor transition releases the preparation menu, copies
the real selected mission tree and complete squad/equipment payload, installs a
type1 `GameApplicationState`, then invokes initialization with its owned kind3
mission. The game marker is not passed as the mission argument.

Construction now supplies concrete native HUD, mission text, overlay and table
owners, real original-engine kind13 scene/callback/viewport setup and late game
registry publication. The startup session borrows the real owner/scene while they
exist. Audio listener stays null: no loaded camera/sector has been established.
`mission_active()` now queries actual owner presence independently of that audio
view. Graphics bit20 is scoped/restored for the activation boundary.

The required006CA780 level loader is **still unavailable**. Initialization returns
failure, never status5/running or successful loading. The receiver shuts down and
deletes the entire unloaded owner, clears borrowed views, then constructs the
existing error-dialog UI. The user sees “The mission could not be loaded.” Its OK
returns to Main through the existing kind7/back path. Repeat entry reconstructs a
fresh owner. Actual loading/actor/gameplay work stays in P2.2/P2.3.

Expected `host-runtime.log` lines after successful constructor execution:

```
Game owner constructed: mission=Tutorial soldiers=... text-owners=77 scene=... status=0 loaded=0
Game activation declined:006CA780 loading pipeline is unavailable; graphics flags restored; loaded=0
```

Earlier acquisition errors instead log `Game owner initialization failed:` with
the actual resource/engine error and use the same released-owner dialog path.
No original game/client functions, successful placeholder renderer, dummy camera,
synthetic loaded-world view or fake animation loader is called.

## Required construction recovered

Original addresses refer to the established base-client fingerprint. This batch
reads only the previously mapped owner graph and five missing cleanup callees;
it does not reopen the postponed exhaustive function review.

- 0049A420 receives0049FEF0 at **this+30**, then00494980 at+2F90. Its32 short records
  call0049A5E0 at+10928+i*0C. Eight final text owners begin+10BC8 and the final
  one is+6B85C. ECX receiver offsets matter; untyped decompilation omits them.
- 0049FEF0 creates one text owner+20,12 two-text composites004A0410, four six-text
  composites004A05D0, one+1FE4 text, two seven-text arrays, four0x94 track records
  and four final text owners. Together with the outer HUD this is **77 text/font
  references**. Constructor does not load HUD artwork;0049A940 does so after
  mission loading. Track defaults include-1 at+8C and1 at+90. Native initialized
  fields preserve colour255/255/255, period50 and0x3CB43958, zero counters/timers
  and disabled HUD flags. Unwritten original buffers are not made into invented
  models, mission data or rendering state.
- 00496CC0 initializes a text owner with physical dimensions truncated to WORD,
  zero geometry/flags, indices-1/-1, inset10 and a font-manager reference.
  0048ECF0 releases fonts before text records. The native HUD destroys these text
  owners in reverse construction order. Populated layout/glyph/HUD artwork
  consumers remain later recovery work; this completes the unloaded constructor
  behavior rather than a renderer or the full original object ABI.
- 0063DB50/0063DBB0's game-side lifetime is represented by one shared `GameFonts`
  allocation retained by all77 text owners, with weak factory lookup and last
  reference cleanup. It loads actual SYMBOLS, CE BIG/SMALL and selected BIG/SMALL
  metrics/atlases in that order, using accepted width/language paths and decoder.
  Metrics go through the real rw_data loose/archive resolver. Atlas acquisition
  is shared with the accepted menu via `load_font_atlas`; any returned reference
  is adopted before a negative result is rejected. No extra staging is needed.
  Wide/GDI/native singleton ABI remains excluded. The large0063DC50 decompilation
  extends beyond its84-byte frozen body; it is dependency evidence, **not new
  function credit** for those out-of-body font-loading instructions.
- 00698510's inline0x28 mission-text constructor creates no renderer: flags0,
  selected/timer words-1/-1, empty name and entry vector.0048F400 resets selection
  to-1/ticks0 and clears an existing renderer.0048EF40 releases renderer, disables
  text and destroys entries in reverse;0048EFC0 releases each entry's strings
  forward. Native `MissionText` retains these real empty/ownership contracts.
- 00524840 destroys a0x24 table record's+20 payload only when flags+14 lack0400.
  The native allocation backend stores the actual DWORD element-count cookie,
  destroys records backwards, frees the cookie allocation, preserves borrowed
  payloads and clears owned pointer/low flag bits. It supplies0051DD60/00523B20's
  previously missing concrete frees. On64-bit native audits cookie alignment and
  pointer width grow; i386 record stride/cookie/payload offsets are asserted.
- 004950C0,004AE2B0,0049FD10 and004A4A20 confirm the still-empty nested cleanup
  paths. Native queues are real owned containers, initially empty; they are not
  calls back into the original client. Their later producers and populated
  original record representations remain explicit partial-function limits.

Engine scene setup retains the fingerprinted [initial interface map](phase2-boundary-map.json).
The installed stdcall/RET12 callback now invokes the recovered dispatcher. Actual
viewport operations and disabled mission-text branch are bound. Initial HUD/
debug/full-viewport flags correspond to original PE zero-fill. Active world HUD,
control, network, debug and effects render services **reject use before world
activation**; they are not successful empty implementations. No scene render is
requested by this status0 owner. The bridge latches unexpected callback exceptions
without unwinding C++ through LS3DF. Live render/simulation/shutdown remain
P2.2/P2.3 work;00698F80 is not replaced by an unloaded destructor.

## Verification

One consolidated sweep passed existing native regressions,158 Python tests and
strict application-launch/engine-window/startup-probe compile/link. A final affected
build/native audit also checks the new graphics flag guard. Windows code/DLLs were
**not executed** by the agent. No desktop verification is claimed.

Temporary ASan/UBSan audit uses the production native HUD/text/session, table
allocator, borrowed-view and receiver helper bodies. It covers100 owner lifetimes,
77 shared font references, five constructor-stage failures,86 allocation-failure
prefixes including late publication, copied objectives/soldier/equipment data,
populated owned/borrowed table payloads, vector capacity, duplicate/cleared world
views and100 failed receiver→dialog lifetimes. The actual production helper is
checked for install-before-initialize and destroy-before-error-UI order; a supplied
success fixture verifies retention only, not actual world success.1,024 graphics
flag/failure cases include exceptions and failed restoration retries. All13 actual
ANSI/CE/SYMBOLS metric tables decode successfully. Native font/engine endpoints
remain lifetime/order fixtures; real texture/scene execution awaits the user.

The linked evidence/index preserves original body hashes and newly added partial
entries without any new complete-function/engine/CRT credit. Temporary audit
sources/binaries are removed; hashes/logs/assembly remain ignored under
`analysis/work/game-owner-integration-20261103`. No package, original-file edit,
runtime staging, commit or push was performed.

## Desktop acceptance — 2026-11-03

The user accepted this checkpoint and explicitly confirmed the expected constructor
log: “Yup, the log shows this message. Record as complete, commit and continue.”
This records user acceptance of unloaded construction/error return; individual
repeat/fault-injection cases remain established by the native audit above. The
agent did not execute the desktop or original engine. No world loading is claimed.

P2.1 is complete within that boundary. Next is the complete P2.2 animation/loading/
Scene/Scene2/resource/camera pipeline. Reuse confirmed ownership research.
