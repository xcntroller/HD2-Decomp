# Team and browser selection follow-up — 2026-11-03

Current status: implemented, compiled, native-verified and **user desktop-accepted
2026-11-03**. The user clarified that the one-soldier selection was Lone Wolf,
whose capacity is intentional. The defects are missing dimmed slots, unreliable
stationary picker double-clicks, and selectable unreachable browser rows.
No package or commit is requested. Test through the Python launcher.

## Original contracts and corrections

`006908B0` selects one-soldier capacity for Lone Wolf and the two documented
standalone missions; capacity is unchanged. Original `00691E10` calls that
predicate at `00691EBB`, then sets alpha to .5 on `table19`, `table20`, `table21`,
`member01`, `member02`, `member03`. The native definition now dims those six
authored backplates/borders through visual virtual +A8 → engine `100368F0`,
which stores alpha at frame+1EC. The first slot remains available.
Cached export: `work/single-pipeline-image-team-20261031/00691e10.c`.

Original `006942D0` calls membership toggle `00694C40` at `00694361`, then
queues `010000A1,04005000`. Common dispatcher `0066C950` handles that by saving
the current receiver/model, repicking and updating pointer focus. This matters
because `0066B990` refreshes ordinary pointer focus on movement; assigning a
soldier shifts the picker beneath a stationary cursor. The team callback now
queues the original refresh after its mutation. The shared 400ms click timer
and accepted stale/empty portrait guards remain in use. First-picker selection
after every removal is the earlier explicit user policy described in
[team-removal.md](team-removal.md), not a full-body matching claim.
Cached export: `work/single-pipeline-screen-callers-20261031/006942d0.c`.

The user's first desktop recheck exposed a missing integration dependency:
“Screen model notification service is not connected.” The new event reached
default throwing `MenuEventActions` methods. `PreMissionMenu::Actions` now
connects screen model lookup, actual engine picking at the current cursor,
pointer focus and focus leave to its existing products. The queued receiver is
retained by the common service. The initial temporary fixture bypassed these
host bindings and did not detect the omission; the corrected audit includes
the production event queue and extracted host methods.

Original server constructor `00724EF0` starts ping at9999. Browser `00635AD0`
recognizes a responding record with ping<9999; responding-only filter `00635E40`
uses the same threshold. Cached exports are in
`work/multiplayer-client-20261101/` and `work/multiplayer-callbacks-20261101/`.
Unanswered rows remain visible with disabled text, but now also reject row
activation/selection and double-click join. Keyboard navigation skips them.
Refreshing a previously selected record to unreachable clears its selection
and action availability; direct choose/select/join paths also guard eligibility.
This implements the requested selection policy using the original sentinel;
it does not claim the entire original browser body is matched.

[Fingerprint-checked map](team-browser-selection-map.json) preserves five
client/engine byte anchors, two client calls and six original frame references.
No new original function credit: ledger remains564 partial, zero complete.

## Verification and desktop boundary

Strict i386 application, preview and startup builds pass. Existing native tests
and158 Python tests pass. Temporary ASan/UBSan audits exercise extracted team
callbacks, real pointer/focus helpers, repeated stationary clicks, normal and
one-soldier capacity, stale removal callbacks, first-picker selection and exact
six-frame dimming. Browser audits cover real row/list methods, mixed and entirely
unreachable results,3000 navigation keys, row reuse and9998/9999 boundaries.
Pre-fix native variants reproduce the picker/browser regressions.

The added integration audit runs100 notifications through production
`MenuEventQueue` and the extracted pre-mission model/picker/focus bindings, then
tests leave. Omitting the binding reproduces the user's exact exception;
the connected version passes sanitizers and compiles/links as an i386 fixture.
Windows fixtures are never executed. Temporary audit sources/binaries are removed
after recording hashes/results under `work/team-input-followup-20261102/`.

The Python launcher was used with `--stage-only`, preserving original assets and
staged user state. No original game/DLL, Proton or Windows program is run by the
agent; no agent desktop execution or package. Ready-server UI and loadouts retain
their previous untested status.

The approved [function-completion and selective matching pass](function-completion-plan.md)
is scheduled before Phase2 implementation. These regression fixes do not complete
that separate checkbox.

## Desktop acceptance — 2026-11-03

After the notification-service correction, the user confirms “Good job. Record
as done, and start work on the next checkbox.” This accepts the follow-up as a
whole; it does not imply separately observed execution of every suggested case.
Built and staged client SHA256:
`b1cd39d51736fa01739c560caa72b22b81e1723e11487fb18568831040d4d902`.
The retained runtime log still records the earlier missing-service failure;
acceptance comes from the user's statement, not a claimed new successful log.
All recorded original assets and staged writable identities are unchanged.
One previously recorded dist executable is now absent; it is not recreated.
No package, commit or push performed. Next is the approved function audit and
bounded completion pass, followed by Phase2 dependency planning.
