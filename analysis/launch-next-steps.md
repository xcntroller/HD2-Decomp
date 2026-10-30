# Next-session launch priorities

Current starting point: [controller/state delivery](controller-integration.md) is
implemented and user desktop-confirmed; milestone0 items1 and2 are complete.
The [opening playback extension](playback-completion.md) now implements the
reached subtitle/audio owners, rendering and cleanup. User desktop acceptance
on2026-10-31 completes item3. Next is failure/partial-startup/normal shutdown;
keep full failure coverage distinct from the accepted ordinary launch.
[Playback preparation](playback-completion-preparation.md) now supplies the
concrete parser, time/audio ABI, text layout, cleanup and acceptance sequence.
Earlier [controller preparation](controller-integration-preparation.md) retains
the preceding state/frame contracts.
The sections below preserve earlier dependency research; use the current delivery
contract to distinguish integrated owners from remaining services.

Planning review, 2026-10-30, after commit050b112. No source behavior changed or
new binary behavior established by this review. Ledger:278 partial, zero complete.

Follow-up: [launch gate/control initialization](control-startup.md) now confirms
the setup caller and implements0072E1A0/00725A50 with actual Win32 lock storage.
Ledger is now280 partial. The dependency table and gate writer addresses there
supersede the initial investigation questions below; full ownership/lifecycle
and protection still remain before integration.

Next integration checkpoint: [startup executable](startup-integration.md) is built
and desktop-confirmed by the user on 2026-10-30. It resolves reached wrapper payloads to
real Win32 imports and composes resource/input/data/settings/module startup.
The original binary self-integrity VM is not ported; full setup/state/controller
integration remains distinct. Both completion markers and teardown result=0 are
confirmed; first delivery item is complete. Research concrete video/menu owners
and controller composition next. Ledger now284 partial.

## Assessment

The bounded launch now composes input/bindings, localization, archives/directories,
data/profiles/settings, catalog and music runtime under the recovered controller.
Its ordinary Main/Exit path is desktop-confirmed. Remaining launch work concerns
playback subtitle/audio owners and cleanup/failure completeness; the old preview
is retained as a diagnostic, not the current delivery path.

Do not treat older analysis documents' remaining-work lists as current inventories.
For example, controller prerequisites006B65D0/006B6620 now have recovered data/
profile bodies and an owning subset in application_startup_data.h. That subset
explicitly does not implement the full application constructor/destructor.
Likewise the early sound preparation notes predate the full runtime recovery.

## Recovered source index

This replaces milestone0's growing list of implementation checkboxes. These are
audited source components, not completion of the original integrated launch.
Update the relevant contracts as work advances; keep delivery outcomes in plan.md.

| Area | Existing recovery and contracts | Still required for launch |
| --- | --- | --- |
| Controller/opening | [Integrated state/controller launch](controller-integration.md), four movies/Main/Exit desktop-confirmed | Subtitle/audio completion, skip/missing-movie and failure matrix |
| Input | [Bindings/defaults](binding-installation.md), [manager](input-manager.md), [joysticks](joystick-devices.md), [DLL forwarding](graphics-input-services.md); shared launch input connected | Broader character/capture targets and device/failure matrix; no invented enumeration |
| Text/window | [Localization/cache](startup-localization.md), [window persistence](window-position.md); launch captions connected | Subtitle rendering and broader localization/resolution fidelity |
| Resources | [Archives](startup-archives.md), [directories](resource-directories.md), [chunk files](chunk-file.md) | Protection boundary, full ownership/order and reached cache consumers |
| Data/profiles | [HumanStore](human-store.md), [GameData/profiles](startup-data.md) | Full application lifetime and live composition |
| Configuration/settings | [Options/profile selection](startup-settings.md), [game.cfg/data owners](startup-owners.md) | Shared gate/control lifetime and remaining application owners |
| Launch gate/data | [Control startup](control-startup.md): explicit gate7 initializer, record defaults and real locks | Full control/interface creation, lifecycle and protection; gate can change later |
| Sound/music | [Catalog/primary selection](sound-catalog.md), [movie coordination](sound-movie-channels.md), [registered data/update/reset](sound-runtime.md); reached launch music/runtime connected | Subtitle audio and playback/cache/restore/spatial branches proven necessary |

Sound playback00433FA0, restore00435240 and spatial00432BE0 are not yet recovered.
The small query helpers do not establish completion of those bodies. Establish
normal-launch reachability before expanding them; do not weaken required behavior.

## 1. First milestone: establish and close the launch dependency boundary

Start with existing evidence, not another broad sound implementation:

- Continue the writer/lifetime trace from [confirmed initialization](control-startup.md).
  Identify subsequent actual startup,
  update and shutdown values; preserve distinct bit1/bit2 uses. Do not seed it
  with a convenient constant to make recovered services succeed.
- Reconcile construction/setup006DBFC0, application006B4A60/006B6010 and controller
  startup006C0820. Record only direct/transitive prerequisites of normal opening
  playback and first menu entry, with construction and reverse teardown order.
- Check the unresolved006DB8C7 →00A59000 protection-state boundary early. Archive
  recovery excludes it explicitly. Establish what the real wrapper needs before
  spending another session on downstream consumers; never replace it with success.
- Separate already implemented operations from absent concrete owners/adapters.
  Include controller flags/mouse/input, alternate-command-line normal branch,
  delay callback and frame/fixed-frame services. Check address/slot meanings
  against original callers rather than trusting old generic service labels.

Deliver a short dependency table with original call sites, current source,
missing owner/body and whether normal launch reaches it. Then reconstruct one
coherent gate/control or owner-lifetime slice that this trace proves necessary.
This is a planned investigation, not a claim that reachability is already known.

## 2. Close the required state and sound dependencies

Recover concrete video/menu application-state lifetime around0066EAE0,
00688040,0066ED80/00688C10 and the null-descriptor006C0EF0 transition. Existing
opening playback does not establish full original state construction/destruction.
Keep status, state type and sequence index separate, and preserve replacement
during callbacks. Account for subtitles/music cleanup and actual menu resources.

For sound, reuse the registered data, full update/reset and query contracts.
Investigate00433FA0 playback/cache population,00435240 restore and00432BE0 spatial
routing when the launch trace requires them. Do not assume that all gameplay
audio branches must precede opening/menu delivery. Conversely, a required branch
cannot be replaced by an empty cache or success callback. The opening state's
GameMovie object and sound-manager primary/secondary IShow slots are distinct.

The sound adapter requires a real live scene/camera-sector owner. Its borrowed
view does not create that owner; the base camera slot8C is purecall. Recover the
actual derived owner if reached. Optional control0072EA20/00742210 similarly
needs real interface lifetime or proof of the original absent-interface branch.

## 3. Compose and validate the original launch

Build the independent launch target only after every reached service has a real
body/owner and established order. Reuse source components instead of duplicating
them in host glue. Keep the confirmed preview available as a separate diagnostic.
Audit normal startup, missing resources, partial initialization, state replacement
and teardown before requesting a desktop test. Compile/link i386 fixtures only;
use temporary native sanitizer audits and existing tests, then remove fixtures.

User desktop validation must cover natural four-movie playback, skips, missing
movies, menu entry/input and clean teardown. Interactive changes require that
confirmation before commit. Do not expand menu flows or replace the engine first.

## Read selectively

- [Controller](application-controller.md), [opening video](opening-video.md).
- [Startup owners](startup-owners.md), [settings](startup-settings.md),
  [archive boundary](startup-archives.md).
- [Current sound contracts](sound-runtime.md), then linked movie/catalog evidence.
- Source: application_controller.h, application_startup_data.h, profile_selection.h,
  sound_manager_data.h, sound_runtime.h and host/sound_services.h under recovered.

Use existing ignored exports under analysis/work and fingerprinted maps. Do not
re-decode confirmed loaders, redo whole inventories or count more accessors as
launch progress. Update the handoff with the next verified dependency after each
milestone. Native ABI differences and source-only recovery remain explicit.
