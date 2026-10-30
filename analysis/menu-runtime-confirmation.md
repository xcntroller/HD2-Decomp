# Slot-zero runtime confirmation

2026-10-30: `frontend/runtime_confirmation.h` reconstructs the reusable dialog
protocol. It is distinct from the named slot-four Exit confirmation. Both use
the `confirm` model; slot zero has ID zero and separate request/reply state.
The [fingerprinted map](menu-runtime-confirmation-map.json) validates 26 byte
anchors, five vtable slots, 44 direct calls and four exact resource strings.

## Definition and construction

00668790 appends text, Yes, No and OK using the ordinary ordinal-ID helper:
00100000 through 00400000 on the initially empty zero-ID definition. Names are
`text`, `byes*`, `bno*`, `bok`; the asterisks are literal lookup-pattern bytes.
Caption style/label pairs are 24/2004, 13/2001, 13/2000, 13/2013. OK starts hidden.
All three buttons register activation tag 04001000 with observer 006693A0 and
the dialog as context. Ordinary screen +8C binding follows augmentation.
Native visual-pair names reuse the already verified confirm-model bridge.

0066C1D0 loads and positions the model at (0.5, -0.375, -0.5), creates a temporary
zero-ID definition, constructs the subclass, then stores front-end +100 before
binding +8C. The subclass starts with mode FFFFFFFF and two zero reply words.
Success appends that installed screen to the registry; failure leaves the
installed product owned but unregistered. Temporary definition storage dies on
both binding paths. This path does not call root +70. The new construction helper
preserves this ordering with supplied model/product services and native RAII;
null allocations throw. It does not rebuild the original object/SEH ownership ABI.

## Requests and replies

006688A0 opens retained raw text; 00668CD0 opens a localized label. Both do
nothing unless mode is FFFFFFFF. They queue visibility changes in Yes/No/OK
order, then the caption, store affirmative/negative event codes and mode, and
queue 02000096. Changes therefore reach the dialog before it is stacked.

| Mode | Visible buttons | Caption style | Keyboard behavior |
| --- | --- | --- | --- |
| 0 | None | 24 | Consume every key |
| 1 | Yes, No | 24 | Escape cancels; other keys use base screen handler |
| 2 | OK | 24 | Same keyboard rule |
| 3 | Yes, No | 25 | Same keyboard rule |
| Other word | Preserve previous button visibility | 24 | Same rule, except FFFFFFFF consumes every key |

Unknown words are preserved rather than rejected. FFFFFFFF remains the inactive
sentinel even if passed to an opener. Original code imposes no four-value enum
validation. The two reply fields hold complete event codes; replies have target
zero, argument zero and no payload.

006691B0 consumes every key while inactive or in mode zero. Otherwise it tests
the low byte for Escape (one), preserving the full word for base handler
00668670. Escape queues removal 02000097, reads the negative code, stores inactive,
queues a nonzero reply and clears both reply words with 006691A0.

006693A0 rejects a null sender with false and consumes inactive requests with
true. An active callback queues removal first, marks inactive, then examines
sender ID bits 20..25: ordinals two/four select affirmative, three selects
negative, and other ordinals still close without a reply. It clears the codes
after submission. No eligibility, visibility, mode-zero or tag filter is added
to this callback; ordinary input routing owns those gates.

00669090 force-close differs: it stores inactive before queuing removal and
preserves both reply words. It still queues removal when already inactive.

## Shared queue and stack integration

02000096 saves the previous screen, stacks registry slot zero without hiding
underneath, and activates with focus zero. Ordinary 02000002 modal selection
still uses 10200000. 02000097 operates only when the current screen's identifier
family is zero; the test does not use registry index. It hides/removes that
screen and clears the previous pointer, without reactivating the underlying
screen. Missing native runtime bindings are rejected before live stack changes.

These routes are installed in the real menu queue. The host now constructs a
separate engine-backed slot-zero model and owning product in an opt-in diagnostic
(`--menu-confirm --probe-runtime-confirmation`). F9 opens localized Yes/No mode;
mouse and focused Enter/Space activate a button, Escape dismisses. The Yes reply
closes the diagnostic host. This path is desktop-confirmed and is not
yet called by any recovered application observer. The regular Exit path is
unchanged; the diagnostic disallows engine-mouse/picking combinations for now.
Services/queue must outlive the stable protocol object, and it must outlive the
typed button observers that borrow its context.

## Verification and remaining work

Earlier temporary ASan/UBSan audit passes across all modes, unknown mode words, open
guards, raw/localized captions, key forwarding, low-byte Escape, force-close,
null/inactive/unknown-ID observer paths, reply order and state at submission.
The integrated audit uses the actual menu queue, screen event runtime, state,
text and observer implementation with fake visual/services backends: button
activation closes before a deferred reply, retained text survives queueing,
localized replacement releases it, and removal does not reactivate underneath.
Constructor success/load failure/binding failure and an ID-family versus index
stack case also pass. Strict Windows i386 audit compilation, strict host build,
the existing native suite and nine frontend/seventeen launcher checks pass.
Temporary audit files are removed; no permanent tests or agent-side Proton run.
The new host diagnostic compiles under strict i386, and staging-only launch
preparation validates the additional localized OK label and resources.

The first desktop run constructed/bound all four runtime children and processed
the F9 request through 02000096, then the renderer rejected missing style 24.
The host logged 155 frames and clean engine teardown. The style catalogue now
includes original 00641e30 entries 24/25 (font 2, black, left/vertical center,
.04/.03 height). A temporary sanitizer audit passes actual definition captions
and mode-three style through glyph submission. Strict i386/native checks pass;
the user subsequently confirmed display and keyboard activation work. General
paragraph wrapping remains open. The latest slice connects the recovered
constructor helper to this live product; the user desktop-confirmed that change too.

Read-only Ghidra exports are cached in `analysis/work/menu-runtime-confirmation-defined`.
`tools/DefineRuntimeConfirmationFunctions.java` verifies subclass slots, forwarding
branches and callback pushes before transiently defining missing methods. Run it
with `-readOnly -noanalysis` and ExportFunctions.java in the research project.
The earlier unseeded export stopped at missing 006691B0 metadata; its zero exit
status did not indicate full success. The seeded export verified all eight JSON/C
pairs with `decompiled: true`; it discarded research-project changes afterward.

Six new PARTIAL census entries: 00668790, 006688A0, 00668CD0, 00669090, 006691A0
and 0066C1D0. Key 006691B0 and observer 006693A0 are absent from the frozen
inventory and earn no separate credit. Total **104 partial (100 client/four
engine), zero complete, frozen 18,783**. The broader screen checklist entry
remains open for concrete factory/product integration and application services.
