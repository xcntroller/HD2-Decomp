# Original binding installation and compiled defaults

This source-only slice follows producer checkpoint `143f518`. It reconstructs
original binding mutation, configuration application and both compiled default
layouts. It changes no live menu interaction and does not read/write user profiles.

## Shared manager installation

[`binding_installation.h`](../recovered/input/binding_installation.h) recovers
`006C7480` and its five-argument wrapper `006C7440`.

Negative indices fail before mutation. A new index grows the twelve-byte binding
vector with zero records, then the first and second output vectors with zero
words. Existing output words remain unchanged. Output vectors grow only when
the binding vector itself needs growth, preserving the original branch boundary.

Replacing a keyboard binding finds its first matching low scan byte, decrements
its byte count, and clears/publishes that scan slot only when the count becomes
zero. A new keyboard binding reuses a matching slot or the first zero slot;
registering a new scan publishes all 64 bytes through IGraph +B4 before incrementing
its count. Scan zero matches an empty slot without publishing a new map. Counts
wrap as bytes. Mouse bookkeeping handles only selectors 1/2/4/8/16; analog and
unknown selectors do not change counts.

Old bookkeeping is removed before replacement validation. A full scan map or
invalid joystick index therefore returns false after earlier mutations, retaining
the old binding record and output words. There is no rollback. Joystick +30 gets
flag 4 only for direction 3 with selector strictly below FF; every other selector/
direction gets zero. Map and joystick callback results are ignored. The replacement
record is copied after callbacks. A null replacement zeros the record, preserving
outputs and gates. Unknown device kinds are accepted verbatim.

The old record's kind is reread after a map callback before mouse bookkeeping.
The native source preserves that callback ordering. Replacement storage must
remain valid across growth/callbacks; a pointer into a growing binding vector is
not a supported borrowed input. Native allocator exceptions, bounds exceptions
and bool return replace original allocator/class ABI and unspecified upper EAX
bytes; only the original AL result is reconstructed.

## Configuration application

[`binding_configuration.h`](../recovered/input/binding_configuration.h) represents
the separate 28-byte settings records: manager index, kind, selector, direction,
mode, numeric group/caption metadata and dirty byte at +18.

| Entry | Recovered behavior |
| --- | --- |
| 006A8600 | Install one setting with device zero, ignore failure, clear dirty after callback |
| 006A8630 | Apply exact matching mode in reverse order, regardless of dirty |
| 006A8690 | Accept modes 2/3 only; install matching records before storing mode |
| 006A86C0 | For entry mode 2/3, apply dirty records in reverse; tags 2/3 require matching current mode, other tags are shared |
| 006A8740 | Clear settings vector only, preserving installed manager bindings and mode |

The dirty pass checks mode validity once at entry and rereads current mode for
each record. Callbacks can therefore change eligibility of later records. Iteration
uses the initial count with fresh element lookups; callbacks must preserve active
borrowed record storage. Appended records are not visited in that pass.

## Both original default layouts

[`binding_defaults.h`](../recovered/input/binding_defaults.h) contains all **92
ordered records per layout**, reconstructed from `006A8C00` and `006AA090`.
`006A8BE0` selects alternate only for selector one; all other values choose primary.
Each constructor clears settings, appends all defaults with dirty one, then runs
the current-mode dirty pass. It does not clear installed manager bindings or choose
a mode. Numeric metadata and intentional mode-specific reuse of manager indices
are retained. This is original compiled data, not a host-selected key layout.

A temporary bounded instruction decoder tracks MOV/LEA/PUSH/POP/arithmetic and
known record-constructor/append calls in both straight-line original functions.
It independently reconstructs all fields and order from assembly, then compares
them with exported literal assignments. All **184 compiled native records** also
match that independent result. Only the meaningful first 25 bytes are compared;
native padding is initialized, while original default padding is unspecified.
The original record constructor ignores its eighth argument; the trailing empty
string pointer in decompilation is not another settings field.

Fresh mode 2 defaults produce 63 manager output words; mode 3 produces 64. Mode
selection preserves shared installed records and replaces the matching mode's
records. Profile-file version/chunk parsing (`006A8900`) remains unreconstructed;
its fallback and successful-load paths now have recovered configuration consumers.

## Validation and next work

Temporary ASan/UBSan and strict Windows i386 audits cover growth/zero fill,
16-bit wrapper truncation, negative indices, all 256 key-count values, same-key
replacement map callbacks, scan-zero behavior, mouse counters, full-map and bad
joystick failures, FF boundary/mode flags, ignored backend errors, callback-mutated
old kind, dirty clearing on failure, reverse mode overwrite, mode changes during
callbacks and both layouts across valid/invalid modes.

An end-to-end source audit uses original defaults, actual native installation and
packed keyboard snapshot, the recovered producer, controller views and front-end
preprocessing. Escape emits back, holding suppresses another action. No fake
production service bodies or new desktop path are introduced. Temporary audit
artifacts are removed after final checks; no permanent unit tests were added.
Existing native checks and all 158 tooling tests pass, the live host build is
up to date, and evidence/accounting/diff formatting checks pass.

Next: original input-manager creation/device ownership and attachment to graphics,
then live controller integration alongside the remaining startup prerequisites.
Static references identify global construction/destruction wrappers `006D5D50` /
`006D5D70` to `006C6A20` / `006C6AB0`; graphics initialization calls `006C6FF0`
then `006C6CC0` at `006DB64B` / `006DB675`. These bodies are not reconstructed here.
Original profile persistence, application prerequisite/profile services, character
targets and focus/navigation callers remain open.

The [map](binding-installation-map.json) validates both fingerprints, 21 anchors
(including both complete default constructors), two engine slots and 191 direct
calls. Ten new partial client entries bring totals to 128 partial (123 client/five
engine), zero complete, frozen denominator 18,783. Native vectors/allocator/class
ABI and the missing integration keep these entries partial.
