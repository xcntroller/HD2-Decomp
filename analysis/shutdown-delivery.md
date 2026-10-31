# Bounded failure and shutdown delivery

Implemented and user desktop-accepted 2026-10-31; milestone0 item4 is checked
within the bounded launch scope. This extends the accepted ordinary
opening → English800x600 Main/Exit checkpoint, using the
[prepared contracts](shutdown-preparation.md) and [original evidence](shutdown-map.json).
No original game/DLL/Proton code was run by the agent. No commit requested here.

## Delivered behavior

The launch now retains acquired owners across its exception handler and uses one
ordered cleanup composition for every acquired prefix. It restores the window
hook, closes/destroys the application state while its services remain alive,
releases startup consumers, attempts window persistence, closes3D→sound→graphics,
then closes archives/destroys startup storage. Outer COM and module guards remain
alive throughout. Each independent cleanup operation is attempted; the first
exception survives later failures and drives teardown result1. Normal completion
remains process result0. Original early failures leak portions of normal teardown;
this native acquired-owner safety is an explicit difference, not original parity.

Controller shutdown now releases the active owner and attempts mouse close even
when state shutdown throws. It still retains a successful reinitializing state.
Partial menu cleanup deactivates the stack while products/text/scene exist, then
releases all owners even if deactivation fails. Its destructor supplies the same
fallback before implicit member destruction. Opening state shutdown consumes its
live flag before its callback, so a failing close cannot repeat persistence or
fade. A failed begin instead leaves partial resource reclamation to the actual
OpeningBackend destructor and never persists an unstarted movie.

Final cold control shutdown implements the reached0072E460 path: verify absent
optional interfaces/+4EE0, run actual cold0072E4E0 reset even after status4 reset,
delete startup locks0 then1, clear the flag and restore gate7. The peer-owner lock
survives until owner destruction. Partial pre-device startup only deletes acquired
locks/record storage; it does not call graphics reset on a failed device. Dirty
network/interface/resource ownership remains an error before unsupported work.

Startup teardown independently attempts sound reset/sectors/catalog, acquired
mouse/keyboard close, input detach and joystick release, then the four real data
owners and the ascending19-slot module shutdown. Sixteen ordinary slot+08 methods
are proven RETs. Items checks500 constructor-zeroed slots/empty queues and clears
the cold auxiliary-list counter; populated state is rejected. Vertex actually
clears its loaded records under gatebit2. Unsupported registrations are rejected
on fresh reads, and a failed Items branch does not prevent later Vertex cleanup.
The shared startup probe uses acquired-prefix cleanup too; the preview retains
its established device lifetime. Physical engine release callbacks remain under
the existing nonthrowing/correct-ABI contract; native failure injection does not
establish recovery from corrupted DLL objects or faults inside foreign code.

Completed runs save window.cfg before device closure through the already
recovered helper. A real GetWindowRect snapshot seeds its ignored subsequent
rectangle failure; if no valid snapshot/window exists, saving is skipped.
Negative signed open handles also skip saving; handle0 and the two four-byte
little-endian writes retain the original contract. Early failed launches skip
persistence. File-write results remain ignored, as in the original.

Actual native movie/voice/text/model/texture/portrait owners are released while
their devices exist. Directory lists are not loaded caches. Complete original
client cache trees/producers, extra setup edit-font/pools, debug/transient owners,
remaining application owners, full setup/protection, live network/gameplay and
other video/menu modes stay excluded; no successful stand-in services were added.

## Repeatable failure and desktop acceptance

The launcher adds `--invalid-game-data`. After ordinary validated staging it
installs a six-byte unknown-root loose GameData override in the isolated run
folder. It preserves any previous staged file and restores it on normal exit or
exception. Ambiguous Windows names, links and unsafe destinations are rejected;
unexpected replacement/content changes preserve user files and backups. The
context also composes with omitted-movie restoration. Installed files/archives
are never modified. `--stage-only` cannot combine with desktop-only fixtures.

The actual GameData loader now exposes its rejected root as diagnostic data.
Expected fatal-run evidence is `rejected-root=2`, `game-roots=0`, `prepare=0`, no
profile-selection/movie entry, all acquired cleanup phases and result1. This
proves selection of the override instead of assuming an archive was hidden.
A missing movie/options/profile is a fallback, not this fatal-loader test.

1. Run `python3 -B tools/run_application_launch.py`, skip into Main, move the
   window and Exit/Yes. Expect final control/consumer cleanup, window write,
   3D→sound→graphics→archive order and result0. Staged run/window.cfg should be
   eight bytes containing the signed left/top coordinates.
2. Run the same command twice, closing the window once during a movie and once
   in Main. Expect reason=window-close, ordered cleanup and result0.
3. Run `python3 -B tools/run_application_launch.py --frames 10` for automatic
   reason=frame-limit cleanup/result0.
4. Run `python3 -B tools/run_application_launch.py --invalid-game-data`.
   Expect the documented fatal loader trace/result1 and restored staged data.

Runtime output is `analysis/work/startup-runtime/run/host-runtime.log`; the next
run replaces it. The launcher/runner can expose the retained output for review.
Only the user ran the desktop checks. Broad failure coverage is not inferred
from one run. Item5 remains the final launch-matrix/commit checkpoint.

## Validation and accounting

Temporary ASan/UBSan audits run144 acquisition/cleanup fault combinations through
the actual portable cleanup/controller/frontend-state composition, checking first
errors, reference destruction before device close, independent releases and
repeat cleanup. Additional actual-template checks cover frontend initialize/update/render failures,
partial/throwing movie
begin/close, status4 plus final cold reset, partial lock acquisition, failing
reset, rejected+4EE0 and retained peer lock, actual vertex allocation/release,
dirty Items/replacement registration rejection and the six-byte GameData loader
fixture. These use native instrumented interfaces, not the original DLL runtime.

A temporary17-case Python audit covers fixture installation/restoration,
case-insensitive names, no original override, retained inode/content, failed
installation/write/body, replacement/symlink/deletion/in-place changes, ambiguous
paths, absent workspace marker, new user files and omitted-movie composition.
Audits are removed after verification; no permanent new tests were added.

Strict i386 application-launch/startup-probe/engine-window builds, existing
native tests and158 Python tests pass. Both-hash shutdown evidence, four negative
mutations, frozen inventories/ledger and whitespace checks pass. No Windows
fixture, original DLL, original game or Proton was executed by the agent.

Four census-backed client bodies0072E460/0048DA80/007DCE00/0078A980 receive bounded
partial credit. Existing006DC3A0 coverage is extended without duplication. Native
coordination, absent RET/vertex shutdown bodies, delegated calls and excluded
owners earn no additional credit. Ledger308 partial (303client/fiveengine), zero
complete, frozen18,783. Counts are source progress, not desktop delivery.

Staged application SHA256: `3d2258a4ee3f95c2da0ed72caf03072a3c1e9ea4d2d95eb78ca01ae6b4e072ac`.

## Desktop acceptance — 2026-10-31

The user requested log inspection and then explicitly confirmed the other runs
behaved as expected (normal Exit, movie/menu window close and frame-limit exit).
The retained final log directly confirms the intentional invalid-root run:
root2, game-roots0, vertex-tables5, prepare0, no profile/movie continuation,
restored window hook, final cold reset/locks, consumer cleanup completed1,
3D→sound(initialized0)→graphics→archives, teardown result1 and the original
application-preparation error. No window save was attempted on the failed run.
The Wine log contains no matched unhandled application exception/page-fault/
access-violation report; generic Wine tracing/fixme output is not a crash result.

Staged and built application hashes match the hash above. window.cfg is8 bytes
(`2f00000015000000`), left47/top21. The temporary GameData directory/override is
absent and no invalid-data or omitted-movie backups remain. The final failed run
preserved this earlier normal-run position file.

Snapshots and hashes are preserved in ignored [work/shutdown-acceptance-20261031](<work/shutdown-acceptance-20261031>),
including manifest.json, native/terminal/Wine logs and window.cfg. Normal Exit,
window-close and frame-limit runs overwrite those logs; their accepted outcomes
rely on the user's explicit confirmation, not retained per-run traces. This distinction is
explicit and no missing logs are recreated. Item4 is accepted within this scope;
ledger308 partial/zero complete is unchanged. No new Windows run or commit by agent.
