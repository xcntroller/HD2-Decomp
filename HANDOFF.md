# Session handoff — 2026-11-01

## Latest result

User desktop-accepted the reachable multiplayer frontend and all Team Selection
follow-up fixes: “Good, I can find no more issues.” User requested committing this
checkpoint **before** researching the next checkbox. No push is authorized.
[Acceptance and exact scope](analysis/multiplayer-pipeline.md#desktop-acceptance--2026-11-01);
[machine-readable record](analysis/multiplayer-desktop-acceptance-20261101.json).

Phase1's reachable multiplayer frontend checkbox is complete. Browser/Create/
filter-editing/Add Server family units are accepted at that bounded scope.
**All UI needing a ready server is untested**, including server information,
populated discovery/browser, live refresh/favorites, Join/password/connection and
standalone screens. The user reports the old server implementation appears broken.
Phase3 now covers server reconstruction, Host/Connect/auxiliary services and all
ready-server UI validation. Do not execute the original server or invent success.
Phase2 retains level/game/status9 owner reconstruction, with live integration
waiting for the rebuilt server. Loadouts remain **completely untested until accessible**.

Function audit adds five newly credited frozen follow-up bodies:0062D880,
006363E0,00637440,00654C60,00648C80. Total **564 partial (559client/fiveengine),
zero complete /18,783**. Historical audits keep their checkpoint counts.
[Audit](analysis/multiplayer-function-audit.md); [ledger](analysis/function-status.json).
Strict i386 builds, native regressions,158 Python tests and relevant temporary
sanitizer audits passed across the accepted review sequence. No original game,
DLL, Windows fixture or Proton was executed by the agent; temporary audits removed.

## Accepted build

`analysis/work/startup-runtime/run/application_launch.exe` SHA256:
`f2295542f86575c6df66a35863ae31c9085deae85583c922c1e68d46bdabfe83`.
User desktop acceptance is recorded; no live server success is claimed.
Earlier accepted team checkpoint13863a7 is preserved as
`analysis/work/multiplayer-current-verification/accepted-13863a7-application_launch.exe`
(SHA2566fe0fd2b46361e3955b1582f16dd00aa540f627d0c261c4cefa00177b0192c4e).

## Next work

After the requested checkpoint commit, **research only** the next unchecked item:
“Validate front-end shared dialog callers, reply/context lifetimes and cancellation.”
Research the entire boundary and dependencies; do not implement it in this turn.
Existing entry points: `analysis/runtime-dialog-integration.md`, its evidence map,
`analysis/menu-family-delivery.md#shared-dialogs` and the20-site prompt inventory.
Phase1 covers reachable profiles/Options/network failure/unavailable callers;
single-player/game callers stay Phase2 and server-ready callers stay Phase3.

No subagents unless explicitly authorized. Preserve original files and user
changes; no original execution or new permanent tests. Commit interactive changes
only when requested and desktop accepted; never push without instruction.

Build: `make -C recovered application-launch engine-window startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Checks: `make -C recovered test`; `python3 -B -m unittest discover -s tools -p 'test_*.py'`;
`python3 -B tools/function_progress.py --check`. Stage without execution:
`python3 -B tools/run_application_launch.py --stage-only`.
