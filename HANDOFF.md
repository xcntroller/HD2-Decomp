# Session handoff — 2026-11-02

## Latest result

Phase1 shared-dialog callers are **user desktop-accepted2026-11-02** and marked
complete in plan/family tracking. User requested recording acceptance, committing
and **not continuing**. Stop after the commit; no further research or implementation.
[Verification, complete scope and desktop checks](analysis/dialog-callers-verification.md);
[original20-site research](analysis/dialog-callers-research.md).

Profile create/rename error dialogs now send020000C3 on both OK and Escape,
matching original00621470 stack arguments. Delete's zero negative reply stays
unchanged. A second native ordering gap is fixed: multiplayer error text remains
pending when the runtime dialog is already active but its slot0 activation is
still queued. The ApplicationMenu adapter acknowledges accepted requests;
RuntimeConfirmation exposes its existing active-state predicate. No new UI route
or successful network service was introduced.

Temporary ASan/UBSan composes actual ProfileManagement, OptionsFlow, runtime
protocol/wrappers, MenuEventQueue/ScreenStack/application routing and the extracted
production multiplayer notifier. All four profile errors x OK/Escape, delete
Yes/No/Escape, Options C7 with unchanged/repaired bindings, shared-text lifetime,
busy/force-close, queued-reply teardown, canceled/reopened profile identity and
pre-pump/network-modal notification cases pass. Native visual/profile services
are fixtures, not desktop or real filesystem/network acceptance. Log:
`analysis/work/dialog-callers-20261101/caller-sanitizers.log`.
Temporary audit source/binary/scratch log removed. Strict i386 targets, existing
native regressions,158 Python tests and ledger validation pass.

## Accepted builds

Accepted `analysis/work/startup-runtime/run/application_launch.exe` SHA256:
`aa5fa4d1ae3cadaa3cc350d24e8603845e1ad68b723ebfe34d86eea7f3e7d8d5`.
User acceptance is recorded in the verification document. The agent did not
execute the original game/DLL, Windows fixture or Proton. Commit requested; no push.

Accepted multiplayer/Team Selection checkpoint is **3cee112**, committed before
caller research at the user's request. Its executable is preserved at
`analysis/work/multiplayer-current-verification/accepted-3cee112-application_launch.exe`
(SHA256f2295542f86575c6df66a35863ae31c9085deae85583c922c1e68d46bdabfe83).
[Acceptance](analysis/multiplayer-pipeline.md#desktop-acceptance--2026-11-01).
Earlier13863a7 team executable is also preserved in that verification directory.
No push is authorized.

## Next work

**Stopped at the user’s explicit request after recording and committing this
accepted milestone. Do not continue until instructed.** Keep remaining plan items,
including the user's Phase1 route review/original comparison/standalone build and
Phase2 scope-planning entries, untouched.

Six original caller sites are Phase1;13 equipment/in-game/debrief/team sites are
Phase2 and one Create Windows XP firewall prompt is Phase3. Current native network
failure callers are audited without inventing the deferred original continuation.
All UI needing a ready server remains **untested** until Phase3 server/Host/Connect/
auxiliary-service reconstruction. The old server implementation appears broken
per user testing; do not run its original binary. Phase2 retains level/game/status9
reconstruction. **Loadouts are completely untested until accessible.**

Function credit remains **564 partial (559client/fiveengine), zero complete
/18,783**. Existing00621470/006688A0/00668CD0 evidence was extended without double
counting. No subagents unless expressly authorized; no permanent new tests.

Build: `make -C recovered application-launch engine-window startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Checks: `make -C recovered test`; `python3 -B -m unittest discover -s tools -p 'test_*.py'`;
`python3 -B tools/function_progress.py --check`. Stage without execution:
`python3 -B tools/run_application_launch.py --stage-only`.
