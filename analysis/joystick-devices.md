# Original joystick list and delegated device boundary

[`JoystickDevices`](../recovered/input/joystick_devices.h) reconstructs client
006DC640 creation and 006DC5E0 close, plus the active frame's reverse polling at
006DCEB0. [`JoystickBackend`](../recovered/host/joystick_backend.h) supplies the
checked i386 IJoy calls needed by this list, the input manager, binding installer
and producer. This is source-only; no live host path or input behavior changes.

## Creation and independent ownership

Creation queries GetIGraph and graphics +48 for the window, then invokes
IJoy.dll `_CreateJoyBase@20` with output-base address, application instance,
window, null DirectInput pointer and flags zero. Graphics +48 takes ONLY the
stack self argument; the two preceding zero pushes belong to the factory.
The DLL initializes its output to null and requires a non-null instance/window.
A false factory result returns without enumeration. A successful base remains
owned even with zero or a negative signed count. Count is queried once at +18;
devices are created in ascending index order using +2C with flags zero.

Each returned device is appended without an extra retain. Raw vector growth
copies pointers without ownership operations. The original false-result branch
calls device +30 with `(1, 2)` and STILL appends the output. The source preserves
this for a valid output. IJoy's actual failure path clears the output, making
that client branch a null dereference; native code throws explicitly instead.
It does not silently skip failed devices or claim a successful full list.

Close releases list references in FORWARD order while the length is still
visible, clears length while retaining capacity, releases the base, then nulls
the base pointer after that call. This differs from InputManager's REVERSE
release of its own separately retained records. Duplicate occurrences own
separate references in each layer. Either layer can release its references
while the other still holds the devices. The native owner closes at destruction;
the original raw-vector exit destructor 006D5DC0 only frees allocation storage,
so original device cleanup depends on an explicit lifecycle call.

The active application frame polls the global list in REVERSE order with flags
one, then invokes the manager's producer with poll FALSE. The list's `poll()`
provides that operation. Do not combine it with `produce(true)`, which would
poll devices again. Snapshot calls remain the producer's job.

## Concrete ABI and limits

Both original modules are borrowed and must outlive all references. The adapter
resolves the exact GetIGraph and `_CreateJoyBase@20` exports, checking their
RVAs. Graphics uses stdcall with explicit stack self; IJoy methods use thiscall
with ECX self. Every delegated method checks its expected vtable target.

| Interface | Slot | Target | Purpose |
| --- | --- | --- | --- |
| LS3DF graphics | +48 | 1006A240 | Window |
| IJoy base | +08 | 100012C0 | Release |
| IJoy base | +18 | 100014C0 | Count |
| IJoy base | +2C | 100018B0 | Create device/output |
| IJoy device | +04 / +08 | 100040A0 / 10004070 | Retain/release |
| IJoy device | +30 | 10002A00 | Configure selector/flags |
| IJoy device | +50 / +54 | 10003870 / 100038A0 | Current/secondary 68-byte snapshots |
| IJoy device | +58 | 10002C00 | Poll/reacquire flags |

Secondary snapshots transform copied button/axis values; delegation preserves
those semantics. DirectInput enumeration, configuration, acquisition and
snapshot transformations INSIDE IJoy remain delegated, not reconstructed source.
The adapter supplies the joystick portion of manager services, not a complete
graphics/keyboard/mouse service.

Native differences: reject repeated initialization until close rather than
overwrite/leak the base; reject success with null base or any null device;
zero-initialize local output; balance a new device reference if configuration/
append throws; use native vectors rather than the original growth ABI. Prior
successfully appended entries/base remain owned after a later failure. Services
must outlive the list; reference callbacks must not throw or structurally mutate
ownership storage. These contracts also apply to manager callbacks.

## Reachability and next priority

The helpers are absent from the frozen client function inventory. A complete
section-byte search found no direct E8 calls or literal stored pointers to
006DC640 or 006DC5E0. The base global 008AECCC has references only inside these
two helpers. The list is initialized empty at 006D5D90; 006DB620 consumes it,
and the active frame polls it. These static findings suggest dormant creation
in this client, but do not exclude computed targets. Lifecycle callers remain
unresolved. Do not wire the helper into startup just to enable enumeration, or
make joystick creation a prerequisite for the original empty-list path.

The list view directly matches graphics input setup's existing view type.
Next prioritize original rw_data search/archive initialization and the actual
remaining application/state startup services. More IJoy internals are not
required for the observed empty list. Concrete graphics/keyboard/mouse manager
services and exact full controller orchestration remain separate work.

## Validation and accounting

A temporary ASan/UBSan audit covers count snapshots, zero/negative counts,
factory failure, success/null guards, first/later device failure, inverted
configuration, allocation-failure reference balance, duplicate ownership,
repeat-init rejection, close/reinitialize, callback-visible length/base,
forward release, reverse polling and repeated empty close. Composition with
the actual InputManager verifies independent retained refs, current/secondary
snapshots, `produce(false)` without duplicate polling and reverse manager
release after list teardown.

A strict Windows i386 fixture instantiates the complete list owner and every
concrete forwarding method and links successfully. It was NOT executed. The
existing native suite, all 158 Python tooling tests and unchanged host build
pass. Temporary fixture sources/executables are removed after validation.

Run `python3 -B tools/validate_joystick_devices.py` to verify all three hashes,
21 instruction anchors, 36 vtable slots and three direct calls. A temporary
negative audit rejects eight changed hash/anchor/slot cases. Read-only Ghidra
exports are ignored under `analysis/work/joystick-list-lifetime`; the guarded
definition script seeds functions only in a `-readOnly -noanalysis` session.

There is NO additional frozen function credit: the two client helpers are
absent from that inventory, and IJoy is outside its two-binary scope. Totals
remain 146 partial (141 client/five engine), zero complete, denominator 18,783.
