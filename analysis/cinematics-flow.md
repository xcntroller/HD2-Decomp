# Cinematics flow — desktop accepted2026-10-31

2026-10-31. This implements plan section3's Cinematics checkbox as one flow:
Main FILMS → profile-filtered list → silent looping preview → PLAY → ordinary
full-screen movie/subtitles/audio → the retained Cinematics owner → BACK/Main.
The checkbox and `media.cinematics` are desktop accepted2026-10-31.
[Byte/function/resource evidence](cinematics-flow-map.json).

## Original behavior and dependencies

Evidence is the fingerprinted base client `8c52ce66…307` at00400000 and LS3DF
`12c61eed…56ee` at10000000; full hashes are in the map. Research used read-only
Ghidra exports and static PE reads, never original executable/DLL execution.
Relevant exports are ignored under `analysis/work/cinematics-{research,controls,
list,rows,layout-final}-20261031`; the final row export terminated after the
needed row bodies, so no whole-export success is inferred. Existing confirmed
profile, queue, stack, widget, opening and Credits evidence was reused.

- **00614140** augments owner slot7/id1C000000, model `single mission 2`.
  Slot5 uses the same model name for a different mission screen. The actual
  12,214-byte model is extracted from `models.dta`, with its material textures;
  SHA256 `06ef21ab261a351f890b94a57946405cfd214f3cec74e3ce45158ff8cfb48b5e`.
  Its screenshot material is cleared (+A0, shared material+1E0/+18), `video` and
  `text00` are hidden. No replacement picture or model is supplied.
- It creates real engine dummy `GB_Cinematics`, links `blong` under it with mode1,
  then recursively clones that template ten times as `Anim_00`…`Anim_09`.
  **0066D9F0/007DC3A0** use the source kind at+F8, visual type/factory where kind1,
  virtual+4C copy, link, and recursive children. Native RAII replaces intrusive
  ownership. Clone/copy implementations in LS3DF remain delegated. The actual
  model contains no `selector` string; an unexpected marker is rejected rather
  than claiming the optional general-list marker is implemented.
- Root controls preserve order/IDs: title1C100000(style10/2109), screenshot
  1C200000(kind4), preview1C300000(kind22), PLAY1C400000(style13/2264), BACK
  1C500000(style13/2256), slider1C600000(kind8), list1C700000(kind23), header
  1C800000(style11/159), version1C900000(style40/2006), two hidden legacy buttons.
  Version formatting remains the shared deferred formatter tracked by Main's
  labels checkbox, rather than an invented build number.
- **ProfileMovies/006A7020** supplies ten actual caption/filename pairs, captions
  A0…A9. Rows retain IDs1C701000…1C70A000 and original metadata indices. The
  existing profile reader/reset/writer supplies unlock flags; Intro alone is
  enabled by reset. No all-unlocked list, synthetic profile or substitute clip.
- **00615820** queues list clear02000085, then include02000083 for each enabled
  original row ID. **006558A0/00655950/00655F10** preserve ascending ID order,
  start position, .032 spacing and17-slot page size. Ten entries cannot scroll:
  original range=count−17+1 is negative, and the real linked slider disables
  itself for signed ranges below2. Wheel00655CC0 still forwards rounded120-unit
  steps to that slider; it is not an acknowledged empty service. Moving a row
  refreshes the cached world-space caption bounds. Keys navigate available rows
  and let the parent screen handle boundary navigation.
- Kind6 rows **006550E0/00653C50/00653C80/00653CB0/00653CF0/00653D20** keep
  focus separate from selection. Hover changes text; selection swaps the actual
  normal/active visuals and deselects the previous row (**006566D0**). Press
  already selects, ordinary activation/click repeats its observer path. Selected
  highlighting survives leaving the group and retained movie playback.
- **00615470** reads the sender's stored index, saves the actual filename, then
  queues PLAY enable01000002, preview stop/close020000A0, and preview open020000A2
  with an owned filename payload, in that order. **00615450** selects status12
  for a non-null sender. These callbacks have four stack arguments plus ECX;
  inferred smaller prototypes were corrected during read-only research.
- **00615660** on hide queues preview stop/close and clears the filename. It does
  **not** disable PLAY again. Therefore reentering can retain enabled PLAY with
  an empty filename; **006C20F0** then takes its original Credits fallback.
  The reconstruction preserves this quirk rather than adding an eligibility gate.

## Preview and retained playback

**00667E40** creates one IShow at construction. **00667EA0** opens
`GameMovie\\<filename>2` with flags0, fits, sets volume0 and plays.
The format at0084062C is `GameMovie\\%s2` (push00667F2C): append literal2,
including the existing extension. PLAY retains the unsuffixed full-movie name. Stop/close
retains IShow for reuse. **00667C90/00667D50** project actual video-frame bounds
using float−4/3: width-limited clips align to the top, height-limited clips center
horizontally; no ordinary opening five-pixel margin. **00668000** draws with
IShow+4C backdrop0 (engine100725D0), graphics/driver state preservation, then
restarts on status1/−1. The host uses its established separate Begin/End scene
pass. Original failed Open can fit undefined dimensions; the native adapter
keeps an unopened preview closed instead. No failed stream is called playable.

**006C20F0** status12 copies the selected filename, retains the existing front-end
owner without shutdown, creates the real ordinary type10/sequence0 movie state
and begins that file (empty filename selects Credits). On status13 it restores
the retained pointer **before** movie shutdown/deletion, then status1. List,
selection and preview owner remain allocated; Back, separately, uses06001003.

**0066EAE0** sets the special-outro latch to0 for this descriptor. Consequently
Outro2 has ordinary Escape/EOF behavior here, with no invented Space/EOF prompt.
**00688040** does have an Outro2 music branch: bookmark request/position and
always queue37 for2000ms, including when37 was already requested. Ordinary clips
use the original .011 fade; **00688460** remembers the real movie in the isolated
profile, restores bookmarked music or fade1, then releases resources. Cleanup
still proceeds when persistence fails, retaining the first error.

**00641E30/00645F20** style8 enables whitespace wrapping. The existing opening
renderer now wraps printable English single-style subtitles before the last
fitting space, using the real font advances and original centered/top layout.
This is needed by the actual Arctic/Africa/Outro1 subtitles. General rich text,
wide-character layout and arbitrary overwide words remain outside this adapter.

## Assets and failure policy

Staging copies and hash-checks the eight available loose catalog movies:
Intro, IntroArctic, IntroNorway, IntroBurma, IntroAlps, IntroNormandy, Outro1,
Outro2 (`.bik`). Loose `IntroAfrica.bik` and `IntroCzech.avi` are absent. This is
not a playback dependency: the user identifies these as unused development
remnants, since Africa/Czech campaigns have no intros. They retain their original
catalog identities/flags but are ignored in missing-media diagnostics. No synthetic
unlock or substitute clip is introduced. Each of the eight installed full movies
also has an original `.bik2` preview; all eight previews are now staged separately.
Other unexpected missing media still receives a diagnostic and actual open/error
handling.

The movie stager preflights before publication, accepts only original BIK/RIFF
headers, rejects overlapping installation/output, targeted symlinks/hardlinks
and changed destinations, and publishes new files without overwrite. Existing
startup/menu stagers preserve isolated config/profiles. Model/frame/media
acquisition failures unwind native owners; codec behavior remains engine-owned.

## Startup failure and clone ownership correction

The first user desktop attempt failed at startup with `Missing visual in menu
button: Anim_00`, result1, on build0087faab…722c3. The prior native clone fixture
checked acquisition counts but failed to look up surviving children after return.
It therefore missed a real lifetime error; its success did not establish this
part of the engine integration.

Static engine inspection confirms **1001D680** Link only maintains borrowed
parent/sibling pointers (+108/+10C/+110); it does not AddRef. **1001C370** Release
decrements +E4 and destroys at zero. Original **007DC41E–007DC448** recursively
creates children and does not release each returned allocation. The native helper
had instead released every local child owner at loop end, removing the visuals.
It now returns `MenuFrameClone`, which retains recursive child owners alongside
the root, with descendants destroyed before the parent. Cinematics rows own these
complete trees. No fake visual, renamed substitute or suppressed binding error.

The replacement temporary ASan/UBSan audit models non-owning engine links and
checks ten complete trees/40 live frames, lookup of normal/active/nested visuals
**after** function return and vector moves, child-before-parent unlink/release,
12 injected allocation/copy/link failures, source preservation and count/depth/
null guards. Strict builds, native suite,158 tooling tests and evidence/ledger
checks pass. The corrected executable is staged; actual desktop retry is pending.
Failure trace and corrected hashes: `analysis/work/cinematics-clone-fix-20261031`.

## Preview filename correction after user desktop report

After the clone-lifetime correction, the user reports the flow otherwise works,
but selecting a movie plays the full `.bik` in the small preview. This exposed an
omitted literal2 in the original00667EA0 format. Corrected preview opens append2
(`Intro.bik2`, not `Intro.bik`); PLAY and profile persistence keep `Intro.bik`.
The original format/resource reference now has a byte-checked evidence anchor.
All eight installed previews are staged with the same no-overwrite/header/hash
checks as full movies. Africa/Czech unused entries no longer produce warnings.

A temporary ASan/UBSan audit compiles the actual video_event body against a native
video recorder and checks all ten catalog paths, retained unsuffixed payloads,
flags0, muted play/fit, stop/close, unopened streams and tag/target guards. Stager
checks cover16 actual assets/eight previews, reuse, full/preview separation and
modified/symlink/hardlink/bad-header rejection. Strict i386 builds, native suite,
158 tooling tests and evidence/ledger checks pass. Corrected preview desktop
verification remains pending; the user report does not establish new-binary
acceptance. Logs/report/hashes: `analysis/work/cinematics-preview-fix-20261031`.

## Verification and desktop checklist

Strict i386 application-launch, engine-window and startup-probe compile/link;
existing native suite; all158 Python tooling tests; temporary ASan/UBSan audit of
actual list/profile/clone/controller/state/font code; evidence maps and frozen
ledger checks pass. The audit covers sparse/all/empty active lists, stable IDs,
focus versus selection, preview aspect fits, clone allocation failure, retained
pointer identity, Escape/EOF/error, Credits fallback and throwing close. It
renders all63 installed Intro/Outro text cues through the actual ANSI metrics;
11 wrap. A separate temporary stager audit checks actual assets, identical reuse,
modified files, symlink/hardlink, bad header and installation overlap. An additional temporary audit compiles the actual CinematicsGrid class body
against native fake frames/rows, covering sparse ordering, caption-bound refresh,
retained selection/deselection, keyboard boundaries, linked wheel forwarding,
inactive-row message delivery and clear. Windows engine/frame/audio integration
was compiled only. No desktop run by the agent,
no permanent tests. Logs and hashes: `analysis/work/cinematics-verification-20261031`.

User desktop test, English800x600:

1. Run `python3 -B tools/run_application_launch.py`; enter FILMS from Main.
   Confirm actual available captions, distinct row positions, hover/selection,
   muted looping preview, disabled scrollbar and pointer/keyboard navigation.
2. Select Intro or another profile-enabled installed clip, press PLAY, then
   Escape; also let a clip finish. Confirm full-screen audio/subtitles and return
   to the same Cinematics screen/selection without replaying the launch sequence.
3. BACK/Escape returns Main. Reenter, test selection again, then Credits and Exit.
   Check music restoration and clean window-close during preview/full playback.
4. Broader unlock/file/profile failure cases use additional isolated copies;
   do not edit the original profile. An all-enabled fixture is not evidence of
   natural profile progression. Missing media, decode/audio behavior and exact
   visuals still require the relevant desktop cases; none is inferred here.

After acceptance record the actual tested binary hash and user scope, then check
`media.cinematics`/the plan item. Next implementation is shared runtime dialogs.


## Desktop acceptance — 2026-10-31

User confirmed **“Yup, works well. Record it, commit and continue”** after the
corrected preview/PLAY/return checklist. Accepted build SHA256
`c6b7c1157a32ca46a7721ada21aff5cdb4983bd15dc87e817fc718772d124f06`. This is user-reported acceptance of the requested
Cinematics scope; no additional per-case logs or agent desktop execution are
inferred. The preview and clone issues above are historical corrected defects.
Plan/media.cinematics checked; next implementation is shared runtime dialogs.
