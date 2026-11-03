# Phase 1 audio, preferences, failures and cleanup — 2026-11-02

Section4's input checkpoint is [desktop-accepted](input-fidelity.md#desktop-acceptance--2026-11-02).
This next checkbox covers the delivered front end's audio, real selected-profile
preferences, resource/service failures and deterministic ownership teardown.
Native verification passes with one bounded cleanup correction. The user has
accepted the desktop checkpoint; the checkbox is complete within this scope.

## Completion boundary

Trace the actual launch/controller through Main, shared dialogs, Options,
Profiles, Cinematics previews/full playback, Credits, reachable preparation and
multiplayer front-end owners, then back/Exit/window-close/failure cleanup. Review
both acquired prefixes and retained owners. Real engine audio/device/codec and
DirectPlay calls remain delegated; native fixtures only instrument those service
edges. They are never installed as application services.

Keep source-only loadouts completely untested until accessible. Game/save
receivers and live sound-world/camera ownership remain Phase2; ready-server UI
and services remain untested until Phase3. Shared-template tests grant no new
delivery or frozen-function credit to those paths. Unfinished Japanese assets
remain expected. Full setup/registry/device-quality expansion belongs to the
following host-approximation audit, not an invented successful startup service.

## Original contracts reused

Confirmed recovery is reused rather than investigated again. Original client
SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`;
LS3DF SHA256 `12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.

| Original evidence | Required behavior and live composition |
| --- | --- |
| 006DB3B0; [sound startup](../recovered/frontend/sound_startup.h) | Driver22050Hz/stereo/16-bit; current bounded host flags0402. Failed/partial initialization closes before silent fallback. Device owners outlive consumers. |
| 0066AD10 / 0066B110; [cache contract](menu-caches.md) | Missing click-buffer load is nonfatal in the application; accepted click queries active then rereads the sound binding before enable. Ordinary keyboard activation remains silent; Options sound-slider feedback is its separate observer callback. |
| 00433550 / 00433510 / 006202A0 | Gate2 controls distinct stored driver/music levels. Music applies current channel scale; sound-slider changes store/apply before click feedback. Music below1 becomes0. |
| 006B6740 / 006B3340 / 006B3210…006B32F0 | Resolve the current profile on each operation; apply graphics, gamma, sound, music, optional control and mouse in original order. No stale pointer after profile selection. Voice controls retain the actual optional-interface boundary. |
| 0061BEA0 / 006B2950 / 006A8770 / 006B3530 / 006A86C0 | Options Apply saves before graphics/binding apply. Three top-level chunks preserve exact60-byte settings and28-byte binding records, including pre-apply dirty/padding bytes. Open failure returns false; inner write results remain ignored. A true return is not proof of durable complete writes. |
| 006B2D80 / 006A8900 / 006B3750 | Missing/invalid outer options falls back to real defaults. Successful binding reading installs dirty bindings; its live records then clear dirty. Ordinary profile changes reapply actual settings. |
| 00439080 / 004370F0 / 00437210 | Music catalog load has its confirmed unusual return value; missing catalog preserves/pads real empty groups. Failed movie Open never becomes playable. Only exact Open result0 starts; gate-clear helper success performs no operation. |
| 00436E90 / 00436F10 / 004368D0 | Primary/secondary fades and queued request switch use actual manager state. Only a downward primary clamp consumes pending; do not replace this with an elapsed-time rule. Exact primary status1 seeks0/replays. |
| 00688040 / 00688460 / 006C20F0; [Credits](credits-flow.md), [Cinematics](cinematics-flow.md) | Credits uses script/fonts and no decoder/voice. Bookmark request/position, select37, restore saved request/position on return. Outro2 forces37 even when already requested. Retained Main/list owners survive playback; previews are silent. Ordinary full movie volume comes from sound preference×.01×1.1. |
| 004361B0 / 004361F0 / 004398B0 | Voices, tracked references, primary/secondary movies and cached samples release through their distinct real owners. Catalog samples release primary then reverse extras. Reset is explicitly composed before manager/container destruction. |
| 006C0920 / 006DCAE0 / 006DC3A0; [shutdown](shutdown-delivery.md), [caches](menu-caches.md) | State/retained state, controller caches and input close while services live; final loaded caches precede3D→sound→graphics→archives. Early launch failures skip normal window persistence. Native cleanup attempts independent operations and preserves the first exception. |

The confirmed launch failure fixture, opening subtitle/movie lifetime audits,
cache-family cleanup audits, profile persistence and service-boundary audits stay
authoritative for unchanged behavior. Current preparation shutdown already wraps
queue deactivation before releasing controls/text/content/scene. Discovery stop
disables reception, cancels/closes/releases the client and joins callbacks before
request identities are cleared. This review does not claim foreign DLL faults
or throwing engine release callbacks are recoverable.

## Cleanup correction

`ApplicationMenu::shutdown` previously invoked multiplayer cancellation before
its exception aggregation. Cancelling a busy browser calls `browser_busy`, which
enqueues captions/eligibility updates. Native queue allocation can throw after
discovery has stopped; that exception previously bypassed all remaining explicit
menu releases. The multiplayer destructor also invoked potentially throwing
cancellation directly from a nonthrowing destructor.

The application now aggregates cancellation with deactivation, releases its
products, dialog/widget/list/input/text/model owners and scene, then rethrows the
first error. The multiplayer destructor catches cancellation errors while its
owned members proceed through ordinary destruction. Ordinary behavior and
discovery's real cancellation operations are unchanged. This is native exception
safety extending the original success-path contracts, not a newly recovered
original function body or full ABI/failure equivalence.

## Verification

A temporary native ASan/UBSan audit uses production sound startup, click,
settings/observer, OptionsFlow, profile writer/reader, catalog/channel/reset and
launch-cleanup templates. It also extracts the **actual** StartupSession music
methods, ApplicationMenu shutdown and multiplayer destructor bodies into native
instrumented service fixtures. These substitutions test ordering/ownership;
they do not execute the Windows scene, DirectSound or DirectPlay runtime.

Coverage includes initialization result/ready combinations; silent/null/active/
rebound click sound; factory/Open failure and gate-clear behavior; settled-menu
music→Credits37→bookmarked menu/position, forced Outro2 and unknown-bookmark
fallback; independent/gated sound/music levels, primary EOF restart; repeated
reset with primary/secondary/sample release; missing catalog; exact preference
roundtrip/path/dirty/padding, open/short-write/default fallback, profile-switch
observer storage, music-zero clamp and Apply order. Cleanup tests inject both
browser cancellation and screen-deactivation errors, verify first-error priority,
release through scene and repeat-close idempotence; a destructor error must not
terminate. Eight launch cleanup phases are each independently faulted.

The same cleanup audit with the previous unguarded cancellation **fails** its
retained-owner assertion. The corrected production body passes. The fixture also
strictly compiles/links for Windows i386, without execution. All three Windows
application targets build; existing native regressions,158 Python tests, frozen
ledger and whitespace checks pass. Fingerprinted sound catalog/channels/runtime,
startup settings, caches, shutdown, Credits, Cinematics and Options hover maps
validate. No permanent tests are added; temporary sources/binaries are removed.

Logs/source and fixture hashes: `analysis/work/audio-preferences-fidelity-20261102`,
including `verification.json`. The previous accepted input executable is preserved
there as `accepted-input-application_launch.exe`. Staging preserves all eight
existing isolated profile/config files by hash. No original game/DLL, Windows
fixture or Proton is executed by the agent. No commit/push requested or performed.
Ledger unchanged: **564 partial (559 client/five engine), zero complete /18,783**.

## Focused desktop review supplied

Current rebuilt/staged executable SHA256:
`e11fb644abd669a04deae5b56bb095763ceec59401ad391ca76718cbcb9384a8`.
Launch normally with `python3 -B tools/run_application_launch.py`; accepted
resolution/language overrides remain available. No full route repetition needed.

1. Adjust sound/music separately, including music0; confirm pointer click and
   sound-slider feedback, ordinary silent Enter/Space, then Back/reopen/restart
   with the same profile. Switch profiles if convenient to check separate values.
2. Enter/leave Credits twice; play/skip an available Cinematics movie, return to
   its retained list, then Main. Check music restoration and silent previews.
3. Exit normally and close the window during Credits/playback or a busy browser;
   confirm clean termination. Existing missing-media and invalid-GameData launch
   checks are already accepted; repeat only if a new failure appears.

## Desktop acceptance — 2026-11-02

The user confirms: “Audio is good, you can record the checkbox as complete.”
This accepts this checkpoint on the executable identified above, including its
bounded cleanup correction. It is an overall result, not evidence that every
suggested case was individually executed. Section4's audio/preferences/failure/
cleanup checkbox is checked. No commit or push was requested. The user separately
requests investigation of Credits' high-resolution entrance delay against the
original, followed by the host-approximation/delegated-call audit.
