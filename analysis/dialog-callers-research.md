# Front-end shared-dialog caller research — 2026-11-01

Research only, performed **after checkpoint commit3cee112** as requested. No
runtime code was changed, no original executable/DLL/Proton was run, and no new
desktop acceptance or function credit is claimed. The next unchecked item is
Phase1: validate front-end shared-dialog callers, reply/context lifetimes and
cancellation. [Machine-readable census and source fingerprints](dialog-callers-research.json).

Subsequent implementation is now [native-verified and staged for desktop review](dialog-callers-verification.md).
The research-only and unimplemented statements below preserve this document's
initial checkpoint; the linked verification document records the current result.

## Entire completion boundary

The frozen route census has20 direct shared-prompt helper sites. Six belong to
currently delivered Phase1 callers: one Options site and five profile sites.
Thirteen belong to Phase2: five equipment/items sites, four in-game sites, three
debrief sites and one team site. One Create site needs the deferred Phase3 server
startup/platform service. All20 are assigned in the JSON; none is silently lost.

Close this checkbox only after correcting confirmed Phase1 differences, auditing
the real producer/protocol/queue/reply composition and context teardown, and
obtaining user desktop confirmation for the changed behavior. Existing Options,
profile and multiplayer acceptance is useful evidence, but does not establish
that every caller/cancellation/error combination was exercised. Do not implement
or desktop-enable game/server-dependent flows to satisfy this Phase1 item.

Named Main Exit is slot4, with a distinct protocol. Password/Add Server are
ordinary multiplayer products, also distinct. Preserve all three distinctions.
General Main traversal and broad fidelity checks are later Phase1 checkboxes.

## Exact Phase1 callers

| Call site / original owner | Trigger | Mode / English label | Affirmative / Escape reply |
| --- | --- | --- | --- |
|0061CA99 /0061BEA0|Options Back, unassigned binding|2 /9520|020000C7 /020000C7|
|00621C69 /00621470|Create duplicate|2 /9500|020000C3 /020000C3|
|00621C8D /00621470|Create invalid/general error|2 /9501|020000C3 /020000C3|
|00622403 /00621470|Rename duplicate|2 /9510|020000C3 /020000C3|
|00622425 /00621470|Rename invalid/general error|2 /9511|020000C3 /020000C3|
|00622A69 /00621470|Delete selected profile|1 /2004|02000017 /0|

9500/9510 are `Profile with this name already exists`;9501/9511 are
`Profile with this name can not be created`. Label9520 is `Please assign all
commands before exiting.` and2004 is `ARE YOU SURE?`. These are read from the
installed base English TEXTY table, not inferred from the datadisk.

**Confirmed implementation difference:** ProfileManagement::submit currently
supplies negative reply0 for all four error dialogs. Original instructions push
C3 twice at00621C58/00621C5D,00621C7C/00621C81,006223F0/006223F5 and
00622412/00622417. Escape therefore must restore edit focus throughC3, just as OK
does. The decompiler prints only three arguments at these sites, hiding this
fourth parameter; use the preserved instruction evidence. Delete's negative0 is
correct and must remain0. No fix was made during this research-only pass.

Evidence: `analysis/work/dialog-callers-20261101/profile-{create,rename,delete}-replies.asm`;
prior owner exports `profiles-delivery-20261031/00621470.c` and
`menu-options-subbuilders/0061bea0.c`. Client fingerprint is
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.

## Existing composition and lifetime contracts

- ApplicationMenu connects Options' request to localized mode2/9520/C7/C7 and
  ProfilesMenu's four-word request to the shared RuntimeConfirmation instance.
  Their captures borrow the ApplicationMenu owner; that owner outlives controls.
- Runtime wrappers0066D780/0066D7A0/0066D7C0 preserve nullable dialog behavior.
  Mode2 displays OK, mode1 Yes/No, mode3 Yes/No with style25, mode0 no buttons.
  Opening while active is ignored. Raw text and queued captions own shared text.
-006693A0 and006691B0 enqueue removal02000097 before a nonzero reply and clear
  reply words.00669090 force-close marks inactive, queues removal and sends no
  reply; it does not itself clear stored reply words. Do not conflate the APIs.
- Runtime dismissal removes slot0 without reactivating the underlying screen.
  This preserves profile edit mode/context and avoids ProfilesMenu::shown reset
  beforeC3. Ordinary modal pop does reactivate; do not substitute it here.
- Options C7 recomputes the first missing binding rather than borrowing an old
  row pointer. If it no longer exists, the native guard consumes the reply.
  Otherwise it restores Controls category and the corresponding binding row.
- Profile flow owns selected_ and pending_delete_ strings. A delete reply acts
  on the saved spelling; create/rename C3 requests editor activation. Cancellation
  emits no delete reply. Audit retained pending_delete_ after No/force-close and
  any late reply; do not invent a new cancellation event without evidence.
- Application shutdown stops multiplayer, closes the stack and clears pending/
  deferred queue records, destroys products/observer contexts before protocol,
  and then releases services/queue and content owners. Audit successful, partial
  and throwing teardown with an active dialog/capture; retain that ordering.

Relevant code: `runtime_confirmation.h`, `profile_management.h`, `options_flow.h`,
`screen_stack.h`, `menu_event_queue.{h,cpp}` and `host/application_menu.h`.
Earlier foundation audits already cover nullable/raw/localized requests,
unknown/zero modes, busy-open, mode0 key consumption and forced close. Reuse that
confirmed contract; focus the next temporary audit on real caller composition.

## Network and deferred callers

The original Create site00633954 is mode3/8938/CA/CB. Label8938 explicitly asks
whether H&D2 should open required Windows XP Firewall ports. It follows
00732500, which gates on GetVersionExA, initializes COM/security and creates a
network-sharing COM manager; this is **not a generic port-in-use prompt**.
CA sets the permission byte+1F0 to1;CB sets it0; both queueC9, which opens the
ordinary connection console13. Current host startup honestly reports unavailable
before this live service path. Do not fabricate the prompt or its continuation
to make the Phase1 coverage matrix green. Original owner export006333C0 and new
`dialog-callers-20261101/create-firewall-probe.asm` preserve the evidence. Complete
this caller with Phase3 server/Host services and applicable platform support.

Current multiplayer failure notifications use genuine raw text, mode2 and zero
reply words through ApplicationMenu. pending_error_ owns its string, retains only
the first pending error, and waits while slot0 is current before submitting.
Audit notification ordering during browser/regular-modal return, busy dialog,
network cancellation and shutdown. These native unavailable-service messages
must not be mistaken for a recovered original server-success prompt.

Deferred Phase2 site groups are explicitly enumerated in the JSON by owner:
00673DD0/00678AC0/00678CE0 (equipment),0067F550/006827A0 (in-game),0068CE00
(debrief),00691020 (team). The original raw helper and force-close census sites
belong to those deferred owners; no invented Phase1 mode0/force-close UI action
is required. Foundation native contracts still need to remain intact.

## Next implementation sweep and verification

1. Correct only the four profile error-dialog negative replies toC3. Preserve
   delete's zero negative reply and existing Options C7 behavior.
2. Compose actual ProfileManagement and OptionsFlow producers with runtime
   wrappers, RuntimeConfirmation, MenuEventQueue/ScreenStack and routed reply
   handlers in a temporary native ASan/UBSan audit. Cover all six Phase1 sites
   with real modes/labels/words; assert removal-before-reply and editor/category
   restoration. Use isolated temporary profile storage or explicit service
   fixtures for controlled errors; fixtures are never desktop/live acceptance.
3. Include duplicate/general create and rename errors, OK versus low-byte Escape,
   delete Yes/No/Escape, changed/missing binding context, ignored busy requests,
   retained text after caller mutation, force-close with no reply, stale/late
   reply handling and active-dialog owner teardown. Reuse known protocol behavior
   without adding undocumented guards or successful placeholder services.
4. Audit real multiplayer unavailable/error producer composition and its stable
   text/context through cancellation/teardown; leave server-ready and Phase2
   callers explicitly untested/deferred in the matrix.
5. Run the strict three-target i386 cross-build, existing native/tool regressions,
   ledger check and temporary sanitizers; remove audit code/binaries. Stage only.
   User desktop check: duplicate create/rename, dismiss with Escape then resume
   editing, delete cancellation/confirmation on isolated profiles, Options missing
   binding restore, and network error dialogs returning to the correct screen.
6. After that confirmation, record the scoped caller acceptance, close this whole
   Phase1 checkbox and update handoff. Commit only if requested. No new permanent
   tests, original execution, server simulation or unrelated flow implementation.

All dependencies needed for this Phase1 sweep already exist. The confirmed C3
difference prevents marking the checkbox done from research or past broad menu
acceptance alone. The runtime implementation remains exactly checkpoint3cee112.
