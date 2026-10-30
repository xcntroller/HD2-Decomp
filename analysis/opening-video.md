# Opening video to menu: normal playback integration

Current path: [controller/state delivery](controller-integration.md) supersedes
the preview composition described below. It is committed as `c0e7346` and user
desktop-confirmed. The [playback extension](playback-completion.md) now implements subtitle/audio
ownership, timing, glyph rendering and cleanup; user desktop-confirmed on2026-10-31.
The defaults/unwired-service statements below describe the earlier preview.

Base client SHA-256: `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Engine SHA-256: `12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
Addresses below are original preferred-base VAs. The normal playback path is
built and integrated behind `--opening-movies`; the user desktop-confirmed it.
The full original application startup remains unfinished.

Normal controller startup `006C0820` calls `006C0CA0` after its prerequisites.
`006C0CA0` first checks the command-line alternate path `006C0AF0`; otherwise
it creates a video state, stores it at controller `+0x14`, and calls `0066EAE0`
with `Logo1.bik` and sequence index 1. `0066EAE0` writes the index to video
state `+0x7c`, constructs a retained movie descriptor, initializes the video
state through `00688040`, then invokes its `+0x24` method.

Video-state update `0066ED80` calls `00688C10` and changes state status 2 to
13. In controller update `006C20F0`, status 13 and video type 10 select the
following branch:

| Completed index | Action | Next index | Current video stopped first? |
| --- | --- | --- | --- |
| 1 | `Logo2.bik` | 2 | yes |
| 2 | `Logo3.bik` | 3 | yes |
| 3 | `Intro.bik` | 4 | yes |
| 4 | `006C0EF0(0,0)` to create front end | — | no explicit stop in this branch |

The normal branch is compiled as
[`opening_video.h`](../recovered/frontend/opening_video.h). The controller also
handles indices 0, 5 and 6 for resume/restart/exit routes; those are outside
this normal-launch reconstruction and must be investigated before claiming
all video transitions. Sequence index, state type and status are separate.

The earlier interpretation of `004336C0` as the playback service was incorrect.
It returns singleton storage at `00899728` for a separate music/audio service:
the client uses its slots for track changes, volume fades and frame updates.
It does not supply the video object. `00688040` gets the video through
`GetIGraph()->+0x38`, implemented by engine `10069E80`. That factory allocates
0x8c bytes; constructor `10070D30` installs IShow vtable `1009D9C0`.
Engine `10071630` imports BINKW32's `BinkOpen` and, when the sound driver is
initialized, `BinkSetSoundSystem` with `BinkOpenDirectSound` and its device.

| IShow slot | Engine method | Recovered signature/role |
| --- | --- | --- |
| +04 | 10070FD0 | Release self; closes before decrement/deletion |
| +08 | 10071B70 | Opened byte |
| +0C | 10071630 | Open(self, filename, flags), stdcall; pops 12 bytes |
| +10 | 10071B80 | Close resources |
| +14 | 10071C50 | Play |
| +18 | 10071D10 | Stop/reset |
| +20 | 10072040 | Set x/y/width/height rectangle |
| +28 | 100720E0 | Query source rectangle/dimensions |
| +2C | 10072160 | Query playback status; -1 unopened, 0 playing, 1 stopped, 2 paused |
| +3C | 100724D0 | Set volume, self/float on stack |
| +50 | 10072650 | Draw(self, byte flag); invokes decode/update +4C |

`00688FF0` opens `GameMovie\\%s` with flags 0x400. The decoded video owns
its timing: +4C calls BinkWait/DoFrame, stops on the last frame and otherwise
advances. `00688C10` queries +2C; the ordinary path stops and clears/presents
on status -1 or 1. Wrappers promote client status 2 to controller status 13.
`00688BB0` accepts scan 1 (Escape). Scan 0x39 (Space) is gated by the special
outro +1f latch, absent from normal opening. `00688AB0` fits the aspect ratio
with a five-pixel margin on the limiting axis, truncating the other dimension.
Equal aspect uses the width-limited branch, so 800x600/640x480 yields
x=5, y=4, width=790, height=592. `00688F50` clears, begins a scene, draws +50,
ends the scene and restores IGraph state zero. `00688460` clears/presents three
times, closes and releases the movie; other subtitle/music cleanup is separate.

[`opening_playback.h`](../recovered/frontend/opening_playback.h) reconstructs
that ordinary playback/skip/completion and rectangle behavior.
[`engine_video.h`](../recovered/host/engine_video.h) supplies the verified DLL
boundary and lifetime. The host starts audio before videos, then constructs
the existing compiled menu only after the sequence finishes. Audio and window
settings use existing host defaults, including video volume 1.0. Keyboard input
comes from host Win32 messages with repeat filtered; original graphics keyboard
polling is not enabled. Subtitles, language data, music coordination, original
preferences, descriptor/class ABI and the full startup/controller remain open.
This is a bounded integration step toward the launch milestone.

Validation: strict Windows i386 build, existing native suite and 158 tool checks
pass. Temporary sanitizer/i386 audits cover geometry, completion, Escape/Space,
repeat suppression at the playback state, paused playback, unopened movies and
cleanup ordering. Temporary staging audits cover repeat copies and rejection
of modified/symlink destinations. Actual four-file staging passes without a
Proton launch. The combined [byte map](opening-video-map.json) checks both
fingerprints, 17 instruction anchors, 56 graphics/IShow slots, eight client
video slots, six calls and four movie-name references. No new census credit.

Confirmed desktop command:

```sh
python3 -B tools/run_engine_window.py --menu-confirm --use-confirm-picking --use-engine-mouse --opening-movies --frames 0
```

Check natural movie playback with sound, Escape skips one movie, Space leaves
ordinary logos playing, menu entry, then normal Exit/No/Escape/Yes. A separate
run closed during a movie checks early teardown. The first error and
`analysis/work/engine-window/run/host-runtime.log` are useful if it fails.
The user confirms intended behavior. The log records four successful opens,
menu entry and clean teardown after 1,259 frames. The subsequent
[controller source slice](application-controller.md) is independently audited;
its required application services/state owners and live integration remain open.
Subtitles/music dependencies and original startup still need recovery.
