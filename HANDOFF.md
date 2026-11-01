# Session handoff — 2026-11-01

## Latest accepted checkpoint

User: “Alright; all fixed, I see no more issues now. Record it, commit and continue.”
**Team-selection checkpoint desktop-accepted** against application SHA256
`6fe0fd2b46361e3955b1582f16dd00aa540f627d0c261c4cefa00177b0192c4e`.
[Acceptance scope and detailed evidence](analysis/singleplayer-pipeline.md#desktop-acceptance--team-selection-checkpoint-2026-11-01).
No extra individual-case execution or full single-player delivery is inferred.

**Loadout/equipment screens are implemented but completely untested until we can
access them.** The user specifically requested preserving this note. Earlier
native data/ownership audits and successful builds are not tests of those screens.
Keep equipment and the full section3 single-player checklist open. Gameplay/save
loading remain outside its agreed boundary; game entry explicitly unavailable.

User authorized committing this integrated preparation/team checkpoint, including
loadout source with that untested status. No push. Existing code/evidence remains
preserved; original assets and exported proprietary material remain ignored.

## Active next work

The user explicitly selected **Multiplayer menus** while loadout testing is deferred.
Continue section3 multiplayer checkbox; do not invent loadout access, unlocks,
working gameplay, or legacy network service responses to close single player.

1. Read the complete multiplayer completion boundary in plan.md and
   analysis/menu-family-delivery.md plus the relevant route/control contracts.
2. Research original service owners/dependencies before implementation. Deliver
   the complete multiplayer UI/service checkbox if feasible; real controlled
   LAN/session endpoints are required. Distinguish unavailable legacy Internet
   service and game entry from working local discovery/connection.
3. Use actual original evidence/addresses, strict i386 builds, relevant existing
   tests and temporary native sanitizer audits. No permanent tests or agent-side
   original game/DLL/Proton/Windows fixture execution.
4. Update handoff/research/plan as work proceeds. No sub-agents unless explicitly
   authorized. Commit new interactive changes only after user desktop acceptance
   and request; the current authorization covers the accepted checkpoint.

## Verification retained

Strict i386 application/startup-probe builds, native regressions and158 Python
tests pass at the accepted checkpoint. Temporary ASan/UBSan audit executes actual
production drag/move bodies in fixtures:108 candidate drop/pick combinations,
10 detach/swap cases,1000 foreground updates;7-pixel inset correction verified.
Prior real-data squad/list/equipment audits and limits are in the pipeline notes.
Temporary audits removed; no permanent tests added.

Ledger remains428 partial (423 client/five engine), zero complete,18,783 frozen;
22 family delivery items remain open. No new original-function credit.

Staged application: `analysis/work/startup-runtime/run/application_launch.exe`.
Log: `analysis/work/startup-runtime/run/host-runtime.log`. Use README's existing
user desktop launch command; agent stages only with
`python3 -B tools/run_application_launch.py --stage-only`.

Build: `make -C recovered application-launch engine-window startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Regression: `make -C recovered test`; `python3 -B -m unittest discover -s tools -p 'test_*.py'`.
Previous accepted baseline: Main446cc90; Profiles977641f; Options/dialogs015f2c8.
