# Captured edit characters and shared focus entry

2026-10-31: the shared native character-target, focus-entry/navigation-caller
and initial-focus contract is complete. **Unused editable/list/binding/spinner
routes are source-only.** Main/Exit retain their previously accepted flow; no
new text screen, network action or desktop verification is claimed.

The foundation checkbox covers common input contracts and actual caller policy.
Full widget frame construction, text-row/caret rendering, row population,
settings and screen-specific application flows remain in their existing widget
and screen-flow plan items. Unsupported record-data services require a backend;
they are not replaced with successful responses. No live text field is required
to manufacture a default character owner for Main/Exit, which have none.

Source: [edit character target](../recovered/frontend/edit_character_target.h),
[text record selection](../recovered/frontend/text_object.h),
[common character dispatch](../recovered/frontend/common_event_services.h), and
[shared focus entry](../recovered/frontend/screen_focus.h),
[editable input runtime](../recovered/frontend/edit_control.h),
[child navigation](../recovered/frontend/child_navigation.h), and
[explicit focus requests](../recovered/frontend/focus_requests.h).
The [map](menu-character-focus-map.json) guards the exact unpacked client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Read-only exports remain in ignored `analysis/work/menu-character-owner-20261031`,
`menu-edit-methods-20261031`, `menu-focus-targets-20261031` and
`menu-focus-callers-20261031`, `menu-focus-closure-20261031`,
`menu-focus-final-20261031` and `menu-focus-specialized-20261031`. Branches and argument forwarding were checked
against PE-section-mapped original disassembly, not just decompiler signatures.

## Character ownership

Front-end `0066B3E0` sends each character to the current self+10 object's +24.
That is the same object installed/released by `020000BF/C0` in `0066C950`, already
recovered in [common services](menu-screen-foundation.md). The native capture
interface now includes character delivery. A temporary retained owner protects
its native callback lifetime; each subsequent UTF16 unit resolves the current
owner again. Existing preprocessing continues after consumption and latches
whether any unit was consumed. A consumed character suppresses screen scan-key
handling; Tab returning false can leave the scan available to the screen.

| Original method | Recovered contract |
| --- | --- |
| `00662790` / edit +38 | If inactive, enqueue target zero/code BF/argument zero/self in +0C; return one. Do not set active yet. Active returns zero. Duplicate pending requests remain possible. |
| `00662880` / edit +3C | If active, enqueue C0 with the same fields, then emit `04002000/0`; stay active until queue processing. Inactive returns zero. |
| `006629C0` / edit +68 | Already active returns seven. Otherwise set active before text calls. An invalid configured index sets a null string/style-zero record at index zero (append only if empty); the configured index is not repaired. Select its signed low WORD; failure returns zero without clearing active. On success retain the selected text and copy at most capacity bytes into the editing buffer. |
| `00662980` / edit +64 | Inactive returns zero. Deselect index -1; on success emit `0400B000` with the supplied argument while still active; then clear active and return one. |

The common BF dispatcher deliberately ignores acquisition's result, so a
failed text selection can still leave a current active character owner.
A C0 request releases the then-current owner even if it differs from the
requester; identity determines whether the pointer/input mode is cleared.
These existing common-service semantics are retained, including callback
replacement of the current owner.

The first slice incorrectly called that invalid-index operation a localized
append. New caller research contradicts it: `006443C0` receives a null string,
style zero and index zero, then delegates `006440F0`. The source now replaces
record zero when present, appends only to empty storage and performs no
localization. The new audit checks both the null string and style replacement.

Backend contracts require actual text storage, the derived virtual deactivation
callback, current wide-mode value, CRT digit classification and tagged event
emission. The native editable input runtime now composes these with real shared
state, text messages and observers. Frame construction and font-row geometry
remain supplied widget services, with no successful fallback backend.

## Selected text and editing

`00643900` takes a signed SHORT index. A valid index cannot be selected while
selection is already active; that failure changes no fields or dirty flag.
Success stores the index, resets selection ticks and sets active/dirty.
An invalid index is still stored and clears selection while marking dirty.
`00644080` and `00644000` use the stored index independently of the active byte:
get retains the string or returns null; replacement retains the record's style,
marks dirty on success, and fails without a valid stored index.

`00662490` requires a nonzero character and active edit byte. It implements:

- Backspace calls `00662380` and consumes even if deletion fails. Empty text
  succeeds without replacing its record. Legacy mode removes one byte. Wide
  mode scans continuation bytes and checks lead/count combinations of one to
  four bytes; it accepts overlong encodings and four-byte leads through F7.
- Tab returns false. Carriage return emits `04016000/0`, then unconditionally
  calls the derived +3C with zero. Escape emits `04017000/0`, rereads active,
  and only if still active restores the retained original text and calls +3C.
  An observer that clears active makes the Escape branch return false.
- Fullwidth digits FF10..FF19 normalize to ASCII before either insertion path.
  Wide mode for values >=80 encodes through `0064D5F0` and requires existing
  byte length plus encoded length **strictly less** than capacity. This path
  bypasses the excluded-byte string and numeric restriction, consumes failed
  encoding/capacity requests and replaces text only after an insertion.
- The byte path uses the low byte. An optional excluded-character C string
  also matches its terminating NUL. Numeric mode delegates digit classification
  with the signed-byte argument and otherwise allows only an initial minus.
  A nonzero accepted byte can fill the buffer exactly to capacity. A truncated
  zero returns false when not already consumed by those filters.

`0064D5F0` rejects surrogate values with -3, and requires output capacity greater
than its one/two/three/four-byte result (-4 otherwise). It writes no terminator.
It accepts four-byte values below 200000, including values above Unicode's
modern limit. For values >=200000, original code sets FFFD but falls into the
**two-byte** path, producing FF BD. The reconstruction preserves that quirk;
ordinary front-end UTF16 delivery does not reach that range.

## Focus callers and initial policy

`00656B20` is shared by screens and groups. Target **15** calls the object's
virtual +20 with 15, maps handled to seven, and performs no ID lookup. It does
not mean "choose the first child." Ordinary targets perform three fresh
lookups for validation, eligibility and the index stored before child +30.
The screen adapter now uses the complete shared helper; `DefinedScreen.key`
provides the screen-specific `00668670` body. The observational pointer probe
explicitly rejects keyboard requests rather than providing a fake key backend.

A complete table inventory in the map records **50 UI tables and 12 distinct
key methods** in 00813C80..00815B00, using the shared eligibility signature and
including derived lookup overrides. All four screen tables use the same
`00668400` activation policy. Their +20 methods are ordinary `00668670` or
slot-zero `006691B0`; their +24 character method is false. Every character-enabled
UI table uses `00662490`, now served by the typed edit runtime.

Existing screen traversal `00668580/006685F0` selects through `006686F0`, passing
zero to leave and 15 to enter. The earlier confirmed no-direct/literal-reference
finding is retained. The actual UI key routes resolve the navigation question:
ordinary screens perform focused-child dispatch/activation, and list navigation
has separate callers and helpers. No shared Tab/arrow traversal binding is
invented or left as a requirement to guess later.

The native child policies now preserve each original distinction:

| Original code | Key/focus policy |
| --- | --- |
| 00656AA0 | Ordinary group validates focus and child eligibility before forwarding the full key word. |
| 00657880 / 00658050 | Specialized groups validate focus but omit eligibility. |
| 006554B0 | List first offers the full key word to an eligible child; Up/Left and Down/Right step through list children, scrolling at visible boundaries and queuing the current top index to +58 when nonzero. |
| 006553A0 / 00655430 | List previous/next do not wrap or exclude subordinate IDs. Leave and enter both receive 15; leave veto is handled without transfer. |
| 006554B0 Home/End/Page | Home/End recursively step to a boundary and return false. Page uses a saved bottom-minus-top span, returns true on early failure, false after a full span. Virtual recursion and child-first delivery remain. |
| 0064E770 / 0064EFD0 | Active binding capture swallows all scans and deactivates on low-byte Escape; its derived wrapper forwards the full word to the internal child. |
| 006502C0 | Minus/keypad subtract and equals/keypad add consume regardless of decrement/increment helper result. |
| 00656C00 | Group deactivation validates, rereads focus and forwards the argument to the child's +3C. Recursive native groups now expose this endpoint. |

Pointer focus and leave now retain the original virtual calls and fresh lookups;
portable fixed host storage no longer hides those semantics in the shared helper.
Explicit screen input binding accepts supplied heterogeneous native adapters;
host availability still binds only implemented controls.

Bounded initial focus follows already recovered [stack/lifecycle callers](menu-screen-foundation.md):

- `00668120` initializes screen focus to -1. Regular selection in `0066C950`
  passes zero to `00668400`; Main has no implicit first eligible child.
- Named modal push passes explicit target `10200000` (Yes). Activation enters
  that target before showing/emitting the screen notification.
- Slot-zero confirmation and reactivation after pop receive zero. Visibility
  gates still apply; a visible underlying Main is not reactivated by the gate.
- Actual pointer repicking can replace requested focus. Existing host picker,
  confirmation fallback and focus-loss/capture safeguards are separate policies.

Programmatic `02000016` is a distinct second-stage focus request, not an
implicit first-child rule. The instruction-site manifest records its actual
producers. Profile handler `006211E0`'s C3 branch queues target `08100000`; this
branch is integrated at the original application-handler priority. The record
edit handler `0065B430` then copies its selected row to the edit record index,
ensures visibility, refreshes rows and requests BF capture.

`00637990` is reconstructed as the separate Password request: set menu mode7,
queue caption on `28200000` (style14/ID2002), push modal slot10, then queue that
editable child's focus. List-row construction `00656ED0` likewise requests the
new row's actual identifier after initializing it. Other discovered application
and in-game producer bodies belong to their screen flows; all use the same
completed targeted-focus endpoint. No later screen is enabled by this work.

## Derived editable focus and field messages

The four character tables share acquisition, release and +24 handling, while
`EditControl` composes their distinct input policies:

- Base edit activation/deactivation queues BF/C0. Plain edit's +38 is a seven
  no-op, but its programmatic-focus event directly calls BF. Plain focus leave
  refuses while text selection is active; otherwise it emits leave twice through
  derived/base sequencing.
- Visual edit rejects disabled enter/activation, updates its active/normal pair
  in original order, and returns seven from leave while emitting leave twice.
  Exact-ID pointer clicks toggle capture. Configured numeric ranges clamp after
  successful deactivation/release, using actual text queries and required CRT
  parsing. Equal limits disable the range. Numeric setters/queries preserve
  mutations to the caller's event code and payload.
- Record edit focus entry returns one. Its explicit focus event selects the
  configured row before BF. Ordinary record activation restores hovered text
  selection, queues row-count/offset notifications, refreshes and emits activation;
  it does not implicitly acquire character input. Record population/geometry
  messages require the record-widget backend rather than falling through to
  base success.

Base `00662200` capacity/numeric/excluded-byte messages, mutable text query,
programmatic activation and conditional deactivation are connected. The query
tries index zero, reads the stored selected record and deselects, even during
capture. Plain/visual layout/inset overrides respect attached-text availability.
Capacity replacement deliberately clears native storage rather than retaining
uninitialized original allocation contents.

Text hover reset/restore and visible-record selection preserve their active
selection gates and rereads (`00643E00/00643BF0/00643C30`). Row visibility remains
a concrete geometry service. Caret state `00643F60`, reached by `006621E0`, uses
wrapping DWORD tick arithmetic: visible below signed500, invisible at500..1000,
reset only above1000. Rendering the caret remains a text-renderer task.

## Native differences and verification

Native shared immutable strings and typed retained owners replace intrusive
32-bit pointers, raw buffers, segmented queues and SEH. Buffer allocation is
safe and initialized; the original all-continuation backspace buffer underread
is rejected. Native text selection starts at -1 without claiming full original
text-object construction. No constructor census credit is taken for default
field declarations. Derived callbacks and locale services remain explicit.

Temporary ASan/UBSan audits passed for the completed native edit runtime,
real state/text/observer composition, capture queue, preprocessing, targeted
screen delivery and every child-key policy. Tests covered null/style-zero
replacement, delayed capture, plain/visual/record distinctions, duplicate leave
notifications, observer effects, commit/cancel, active text queries, capacity
replacement, numeric event mutation/clamping, correct pointer targets, record
focus ordering, Profile C3 through the actual queue, Password request ordering,
all14 named initial-focus requests, modal/slot-zero policy, exact caret boundaries
and overflow, scroll notifications, list non-wrapping/veto/Home/End/Page returns,
full-word specialized forwarding and binding/spinner scans. There were 10,000
independent generated list-navigation comparisons; the prior character audit
also covered 10,000 wide edit sequences and 2,097,154 encoder inputs.

The completed instantiated audit compiles strictly for Windows i386, and its
Windows fixture links without execution. Application launch, preview and startup
probe compile/link strictly; existing native checks and all158 tooling tests
pass. The map validates **73 instruction anchors, 112 virtual slots and 73
direct calls**. Related evidence maps and frozen accounting pass. Temporary
audits were removed; no permanent tests, original/Windows/Proton execution,
staging, desktop verification, commit or push was added.

This continuation adds17 partial frozen-inventory entries; the preceding
uncommitted character slice added nine. Total: **334 partial (329 client/five
engine), zero complete; frozen18,783**. Absent handler bodies including00662490,
006554B0, 0065B430 and006211E0 earn no census credit. Full ABI and delegated
widget services receive no complete-function credit.

```sh
python3 -B tools/validate_frontend.py --map analysis/menu-character-focus-map.json
python3 -B tools/function_progress.py --check
```
