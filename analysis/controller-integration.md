# Controller/state launch integration

Delivered and user desktop-confirmed on 2026-10-30. This closes milestone0's
second item for the ordinary four-movie sequence and compiled Main/Exit menu.
[Original contracts and research](controller-integration-preparation.md) and the
[fingerprinted evidence map](controller-integration-map.json) preserve the source
addresses. Full opening audio/subtitles and the failure matrix remain separate
items; this is not the complete original application or menu.

## Run and scope

Build the strict i386 target with the project's Windows compiler:

```sh
make -C recovered application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++
python3 -B tools/run_application_launch.py --stage-only
```

The user can run `python3 -B tools/run_application_launch.py` on the desktop.
Staging copies required archives, DLLs, movies and menu assets beneath
`analysis/work/startup-runtime`, preserving existing staged config/profiles and
leaving original game files unchanged. The launcher defaults to an interactive
run. `--stage-only` performs no Windows execution. The agent used only staging,
compilation, native audits and static evidence checks.

`application_launch.exe` owns one graphics, sound and 3D lifetime. Startup
resource/input/text setup, application preparation and profile reset are split
at the recovered controller boundaries. `ApplicationController` installs owning
single-movie states, advances Logo1/Logo2/Logo3/Intro and installs the front end
before calling its initializer. The compiled Main/Exit runtime belongs to that
state; it has no separate frame loop.

## Recovered behavior now connected

- Binding constructor006A81C0 restores mode2 at006A81D4. Real default installation
  produces63 bindings and63/63 buffer words; explicit mode0 still stays empty.
  The menu borrows the initialized keyboard and uses checked engine polling,
  scan/character/modifier calls rather than reopening it.
- The normal006DCAE0 frame samples input, ticks the driver and renders before
  fixed/variable controller updates, then maintains control data and presents.
  Existing fresh-state callback rules, delay expiry and one100ms subtraction
  remain intact. Base pending status commits only when not2; ordinary fixed
  state callbacks are evidenced RET methods, not invented successful services.
- Ordinary opening playback uses the actual IShow owner, shared sound/profile
  settings, volume multiplier1.1, primary fades, Escape and failed-open
  progression. Shutdown clears/presents three times before profile/fade cleanup
  and video release. Recognized Intro completion reaches the real profile writer
 006B09A0 through006B1290;006A6D80 writes ten movie flags in reverse order.
  Display-to-physical profile mapping, strings, slot IDs and settings chunks are
  preserved. Playback subtitle/audio owners are not covered by this delivery.
- Frontend scene, text and cursor rendering use actual engine resources and
  loaded startup captions. Existing screen/control/queue/pointer products now
  consume controller-driven input. The ordinary0066BEA0 path retains both
  derived date and sound updates.006269B0's date event targets04300000.
  Menu music uses the recovered catalog/selection/runtime with shared ownership.
- Cold standalone control hooks0072C3B0/0072CD50/0072C5D0/0072CE70 implement real
  empty-queue and inactive-record checks, actual locks, signed/wrapping50ms
  accumulation, one subtraction/excess reset and disabled timer semantics.
  Reset uses the real cold firewall predicate, graphics flags and peer-owner
  lock. Populated queues, active peers/interfaces and changed modes reject
  before unsupported mutations; there is no whole-service gate7 success stub.
- Exit requests state status4. The controller shuts down and releases the menu,
  closes mouse input and resets cold control before returning false to the
  outer loop. Screen/input/text/scene consumers close before shared devices;
  archives close afterward. Global loaded-cache trees are distinct from directory
  registration lists and are not claimed cleared by that subset.

## Desktop acceptance

The user confirmed that the requested desktop interaction works. The retained
`analysis/work/startup-runtime/run/host-runtime.log` records all four movies
opening, state type10 sequences1–4, menu state type2, actual scene/text/cursor
submission, pointer and keyboard activity, confirmation cancellation and Yes.
It records `bindings=63`, `input-words=63/63`, `status=4`, menu shutdown, cold
control reset and teardown `result=0` after1,692 frames. The tested/staged
executable SHA-256 is
`6970f498ed9fe192fe614a9713d48c0ca88a55c3e4d7e376de7c35dbfe0e11ae`.
The user's confirmation includes hover, keyboard and focus return. No missing-
movie desktop run, subtitle fidelity or full failure/shutdown matrix is claimed.

## Validation and remaining work

Strict i386 compile/link passed for application-launch, startup-probe and
engine-window. Existing native checks and158 Python tests passed. Temporary
ASan/UBSan audits covered controller state replacement, callback ordering,
status4 teardown, input63/63, cold control predicates/accumulator/locks and profile
persistence. An independent raw decoder checked every emitted profile chunk,
NUL-terminated string, word and reversed flag. Temporary staging checks covered
mutable preservation, immutable mismatches, links/overlaps and original hashes.
Temporary audit fixtures were removed; no permanent tests were added.

The static map checks both original binary hashes,76 instruction anchors,
35 client/19 engine vtable slots and65 direct calls; altered evidence rejects.
These checks prove the recorded boundaries, not whole-function equivalence.
Thirteen newly implemented census-backed client subsets bring297 partial
functions (292client/fiveengine), zero complete, against the frozen18,783.
Absent state helpers/RETs, delegated engine calls and native glue earn no credit.
0072E4E0's tiny frozen body does not receive credit for a larger exported flow.

Next is milestone0's third item: reconstruct and connect the reached subtitle
text/audio ownership, remaining playback music/skip/cleanup branches, then verify
ordinary, skipped and missing-movie transitions on the desktop. Preserve the
working shared controller rather than extending the old preview loop.

English800x600 and Main/Exit remain the explicit UI boundary. Eager fourteen-
screen initialization, other menu actions, special/outro video states, original
object ABI, network producers/restore/spatial services, global resource-cache
lifetime, full registry/setup bootstrap and binary integrity VM are not delivered.
The native boundary calls decoded Win32 APIs directly; no always-success VM or
unavailable service is substituted. Failure/partial-startup cleanup and the full
launch/shutdown desktop matrix remain milestone0 items4 and5.
