# Ordinary opening playback completion

Implemented 2026-10-31 on top of controller commit `c0e7346`. Strict i386,
native sanitizer, asset comparisons, staging audits and existing checks pass.
**User desktop-confirmed both acceptance runs on2026-10-31; milestone0 item3 is complete.**
The [preparation](playback-completion-preparation.md) preserves exact contracts;
the [map](playback-completion-map.json) checks both original binary hashes.

## Concrete integration

The existing `application_launch.exe` still owns the single graphics/sound/3D
lifetime and controller movie→menu transitions. The ordinary movie backend now
owns a separate ISnd voice, timed sample/text records and actual glyph renderer.
It does not invoke a second preview loop or original client functions.

-006890F0 loading uses the real executable directory/language and CRT existence
  probe, then the recovered numbered/quoted DTA loader into a **movie-local**
  cache. Startup/menu localization remains intact.00689970 consumes sorted
  text values; settings byte3A gates text and leaves `S` audio cues enabled.
- `OpeningSubtitles` preserves atoi prefixes, actual sample creation, Sounds then
  DATADISK fallback on nonzero load status, and original ignored playback results.
  Malformed temporary allocations are reclaimed; undefined numeric overflow
  rejects. Real engine sample/voice factories and distinct refcounts are used.
- `OpeningApplicationState` updates sound first, handles stopped/failed video,
  then still queries actual IShow+38 milliseconds and executes text/audio before
  promoting status2→13. Query failure preserves the previous time value.
- Text activation uses strict wrapping32-bit `start*100 < time < end*100`.
  Active expiry uses `end*100 < time`; last overlapping match wins a search.
  Audio advances once per update, applying actual profile driver volume, binding
  the sample, voice volume1 and play(0,0), even if earlier engine calls fail.
- The actual English800x600 text consumer uses SMALL.tbl/atlas, style8 white
  centered/top text, lower X100..700/Y492..588 box, inset10 and original depth.
  All installed Intro strings are preflighted and fit one line. Unsupported
  paragraphs/wide rendering reject explicitly rather than submit empty success.
-00688F50 composition now has separate movie/text scenes, text state0=1 and
  saved-state restoration. The controller owns presentation. Text uses the
  existing verified Direct3D-state bridge and actual IGraph atlas/triangle calls.
-00688460 cleanup preserves three clear/presents, recognized profile persistence,
  primary fade restoration, movie close/release, voice close/release, reverse
  text then sample destruction, fourteen style-handle removals/reset-1 and text
  destruction. The descriptor clears afterward. Exception cleanup reclaims
  partially acquired consumers before devices; full failure equivalence is item4.

`OpeningTextStyles` owns the reached native style0/8 subset and actual14 handles;
ordinary first cleanup removes style0 and resets the handles. This is not a full
90-style original registry/font-cache reconstruction. Font texture ownership is
explicit and separate from the original style-ID removal operation.

## Desktop acceptance and repeat runs

The user confirmed normal playback/captions, skips/Space/focus/menu/Exit and the
requested missing-movie run. The retained normal log records all four movies
opening, four style8 activations/submissions/clears at the original cue times,
menu render/input, status4 Exit and teardown result=0 after11,306 frames.
Caption activations are161040/181040/183520/186520ms; clears are
163040/182520/185520/187520ms. All staged movies are restored and no omission
backup directory remains. These are actual user-run observations; the separate
audio-cue path remains native-audited because installed English Intro has no S cues.

The staged executable SHA-256 is
`0a2cfd32d3c4fa27c7e8c49fce5f89914b7e5d8714a02c3a5586ec0ce49333d0`.
Agent preparation used `--stage-only`; no Windows executable/DLL/Proton was run
by the agent. The existing staged config/profiles are preserved.

Run from the repository root on the user desktop:

```sh
python3 -B tools/run_application_launch.py
```

Check movie sound, ordinary Space, Escape skips, focus return and Main/Exit.
Let Intro reach2:41 for the first caption; the four original cue intervals are
161–163s,181–182.5s,183.5–185.5s and186.5–187.5s, with strict boundaries.
The installed English Intro has no separate `S` audio cues; the voice sequencing
has native fixture coverage, not an invented desktop audio-cue claim.

Also run:

```sh
python3 -B tools/run_application_launch.py --omit-movie Logo1.bik --omit-movie Intro.bik
```

This deliberately tests real failed IShow opens and progression through the
remaining logos into Main/Exit. The launcher first validates/stages everything,
then temporarily moves only selected staged copies outside GameMovie. It
restores them after success or an exception; original movies remain untouched.
Unexpected replacement files are preserved together with retained backups,
never overwritten. `--omit-movie` is repeatable and cannot accompany
`--stage-only`, which would otherwise restore movies before the desktop run.

The runtime log is `analysis/work/startup-runtime/run/host-runtime.log` and is
replaced by each run. Expected new markers: `Opening subtitles` with actual
loaded/enabled/text/audio counts; timed `Opening subtitle active`, `rendered`
and `cleared`; per-movie `Opening resources released`; existing controller menu
render/input, status4 Exit and teardown `result=0`. Missing opens must progress,
not abort. Both requested desktop runs are user-confirmed; item3 is checked within the
ordinary opening boundary. Full failure/shutdown coverage remains item4/5.

## Verification and limits

- Strict real-adapter i386 build/link passes for application-launch, startup-probe
  and engine-window; the native audit also compiles/links as an i386 fixture,
  never executed on Windows.
- Temporary ASan/UBSan audits exercise actual parser/model, duplicate IDs/BOM/
  empty/missing files, text off/audio independence, malformed records, both audio
  load paths/failures, exact boundaries, overlap policy, wrapping multiplication,
  large jumps, one cue per tick, failed time queries, reverse release and partial
  sample acquisition, ordinary Escape/Space and completion-tail ordering.
- Actual portable render composition is audited for two scenes, state restoration,
  preview/outer presentation distinction and throwing text cleanup.
- An independent raw decode of actual Intro.txt/SMALL.tbl matches all four cue
  intervals, glyph counts, centers, baseline and widths342/219/259/61. Actual
  submissions use white glyphs, depth bits3F43ACC5 and reciprocal depth.
- Temporary launcher audits verify selected omissions/restoration, exceptions,
  partial rename failures, rejected duplicate/escaping names/links, collision
  preservation and unchanged original fixtures. Existing native suite and158
  Python tests pass; evidence map, mutations and frozen function checks pass.
  Temporary audits are removed; no permanent tests were added.

Seven new census-backed partial client bodies are recorded:00688040,006890F0,
00689970,00688630,00688C10,00688F50 and00688460. Existing00641E30/0066ED30
coverage is extended without duplicate credit. Total304 partial (299client/five
engine), zero complete; frozen18,783. Delegated engine methods, absent helpers
and native staging/render glue receive no new original-function credit.

Full original class/container/COW/SEH ABI, generic wrapping/locales, credits/outros,
eager frontend builders, network producers, full global style/resource/font cache,
setup/integrity VM and failure/shutdown matrix remain outside this bounded launch.
The existing device-state bridge remains explicit; this does not claim complete
original text-engine equivalence or desktop coverage of unused special/sample/paragraph branches.
