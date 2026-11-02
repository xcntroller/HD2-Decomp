# Front-end dialog caller implementation — 2026-11-01

Implementation/native verification of the next Phase1 checkbox, following
[the complete caller research](dialog-callers-research.md). **Desktop accepted2026-11-02; the Phase1 checkbox is complete.**
Original execution was not performed by the agent.

## Changes

Original00621470's four create/rename error sites push020000C3 for both replies.
ProfileManagement::submit now suppliesC3 for Escape as well as OK. The actual
application router handlesC3 before generic handlers, requesting focus08100000.
Delete remains mode1/2004/02000017/0; Options remains mode2/9520/C7/C7. No new
caller, event code, menu action or successful service has been introduced.

The composed audit also exposed a native notification ordering gap.0066BEA0
calls menu update twice before its only queue drain. A runtime request can
already be active while the current screen still points to the browser. The
old multiplayer notifier cleared pending_error_ before calling the runtime
service; a busy service ignored that second request and lost its message.

RuntimeConfirmation now exposes its existing active-state predicate. The
ApplicationMenu error adapter returns whether it accepted a request, declining
when absent/busy. MultiplayerMenu clears its owned pending text only on accepted
submission. An error waiting behind an active/queued dialog remains owned and
is submitted after the dialog is dismissed. The runtime protocol still ignores
busy opens exactly as before; its original mode/queue semantics are unchanged.
This is a host notification fix, not new original-function completion credit.

## Native audit and limits

A temporary C++ ASan/UBSan fixture composes the production ProfileManagement,
OptionsFlow, runtime request wrappers, RuntimeConfirmation, MenuEventQueue,
ScreenStack and real application event routing. Native screen/services fixtures
record visual/focus actions and deterministic profile failures; they do not
claim filesystem, engine rendering or desktop behavior. It checks:

- All four profile error branches through both OK and full-word Escape: eight
  paths verify exact mode/label/reply words, preserved create/rename mode,
  removal-before-focus and no underlying-screen reactivation.
- Delete Yes/No/Escape, saved target spelling and cancel/reopen with a different
  selected profile. Only the affirmative branch mutates the service fixture.
- Options C7 from the genuine Back-validation producer, both OK and Escape,
  with the missing binding unchanged or repaired before the reply. Correct row
  restore versus the existing changed-context guard is checked.
- Raw text survives caller lifetime, expires after queued/render records release
  it, and an attempted busy request does not replace the active caption.
- Force-close modes0/1/2/3 emits no affirmative reply. Closing the queue with
  pending removal/delete reply drops those records before owner destruction.
- The exact production multiplayer error-submission block and fail method are
  extracted into the native fixture. It checks first-pending-error retention,
  busy rejection before slot0 activation, waiting during a modal, delivery after
  dismissal, return to ordinary Add Server modal14, and no delivery after stack
  teardown. No native networking endpoint is fabricated.

The audit passes with address/undefined sanitizers. Log retained at
`analysis/work/dialog-callers-20261101/caller-sanitizers.log`. Temporary source,
executable and scratch log are removed after verification. No permanent tests
were added. Existing native regressions,158 Python tests, all three strict
i386 targets and function-ledger validation pass. Function count remains
**564 partial (559client/fiveengine), zero complete /18,783**.

The original census remains6 Phase1 sites,13 Phase2 sites and one Phase3 Create
firewall continuation. Loadouts are completely untested until accessible; all
ready-server UI stays untested until the Phase3 server reconstruction. This pass
does not change those boundaries. Arbitrary invented late reply messages are
not new application operations; the audit checks actual queued replies are
discarded on teardown and ordinary cancellation produces none.

## Remaining desktop gate

Use isolated copied profiles. Trigger duplicate create and rename errors,
dismiss with Escape, and verify editing resumes immediately; repeat with OK.
Exercise accessible general-error prompts, delete No/Escape and Yes, Options
unassigned-binding return, and network/unavailable error dismissal back to the
correct browser/address screen. The native audit covers difficult deterministic
failure/teardown branches; do not infer individual desktop execution from it.

After user confirmation, record this scoped Phase1 caller acceptance and close
the complete checkbox. Until then keep it open and do not commit the interactive
changes. Prior accepted checkpoint remains3cee112; its executable is preserved
at `analysis/work/multiplayer-current-verification/accepted-3cee112-application_launch.exe`.


## Desktop acceptance — 2026-11-02

The user confirmed: “Looks good, record it as completed and commit. Don't continue.”
This accepts the Phase1 caller milestone and authorizes its commit, then stopping.
Accepted controller SHA256:
`aa5fa4d1ae3cadaa3cc350d24e8603845e1ad68b723ebfe34d86eea7f3e7d8d5`.
The whole bounded Phase1 caller checkbox and dialogs.callers family unit are
complete. This broad confirmation does not imply each native case was manually
exercised. The earlier pending-review statements preserve verification history.
Phase2/game/equipment callers and Phase3/server-dependent behavior remain deferred;
loadouts and all UI requiring a ready server remain explicitly untested.
No function count is added:564 partial, zero complete /18,783. No push authorized.
