# Original input-manager lifetime and graphics attachment

This source-only checkpoint follows `5871f3c` (binding installation/defaults).
[`input_manager.h`](../recovered/input/input_manager.h) reconstructs the manager
owner and connects its real state to the recovered installer and producer
contracts. It does not change the live host or enable the full controller.

Follow-up: [factory-list ownership and concrete IJoy forwarding](joystick-devices.md)
are now recovered and audited. The factory list releases forward, independently
of this manager's reverse releases. Its creation helper has no discovered caller;
do not add it as a prerequisite for normal empty-list setup.

| Entry | Recovered behavior |
| --- | --- |
| 006C6A20 | Normal global construction: null graphics, empty vectors, axis limits 20/20/20 |
| 006C6CC0 | Validate graphics/device list, clear source tables, retain/append joystick records, set key gate and 400/400/1 limits |
| 006C6C90 | Convert three signed axis limits to floats and return success |
| 006C6FF0 | Clear binding/output lengths and source tables, retaining devices and capacities |
| 006C6B60 | Clear binding/output lengths, release joysticks in reverse, then clear joystick length |
| 006C6AB0 | Detach, then free second output, first output, binding and joystick storage |

## Ownership and initialization

Graphics is **borrowed**: attachment does not AddRef it and teardown does not
Release it. The manager also does not create or close keyboard/mouse devices.
Those remain separate original graphics setup/cleanup calls. Supplied joysticks
are retained via +04 and released via +08. Each occurrence owns one reference,
including duplicate device pointers. Relocating records copies pointer and
snapshots without another reference operation.

Attachment rejects null graphics or any null supplied device before mutating
state. Device validation is last-to-first; retention/appending is first-to-last.
New records have zero current/secondary snapshots. Attachment appends to existing
joystick records, preserving installed bindings/output words and earlier device
refs. It replaces the borrowed graphics pointer without releasing the old one.
It sets the 64-bit key gate to all ones, leaves the mouse gate unchanged, sets
400/400/1 axis limits and clears the byte at original +B4.

The original constructor writes 20/20/20, not the attachment limits. Other native
initial values reflect the actual original instance's zero-initialized static
storage, not a claim about arbitrary placement construction. Allocator tag bytes
and original class/vector/allocator ABI are not reproduced.

## Three distinct reset behaviors

Binding clear resets binding/output lengths while preserving capacities, graphics,
joysticks, gates, limits and attachment flag. It clears the 64-byte scan and key
count tables, but only **four bytes** of mouse counts at +A0. Original +A4/+A5
survive, including the fifth-button count. It does not publish the cleared scan
table to the graphics engine.

Detach clears binding/output lengths before device callbacks, then releases
joysticks last-to-first and clears their length. It preserves source tables and
counts as well as graphics, gates and limits. Repeated detach on an empty manager
performs no device callbacks. The destructor subsequently frees vector allocations
in the original order rather than relying on native member declaration order.

`JoystickInput` now includes its retained device handle. Windows i386 static
assertions verify its original 8C-byte record size, current snapshot at +04 and
secondary at +48. Native 64-bit layouts remain separate from original ABI.

## Composition and limits

`InputManager` forwards map installation to its currently attached graphics and
joystick configuration/poll/snapshot methods to the selected retained device.
The recovered defaults/installer and producer can use that owner directly.
There are no default successful backend bodies. Concrete interface implementations
and joystick enumeration are still required before connecting this to live startup.

Native source rejects negative counts/missing arrays, absent graphics at use and
null active devices rather than following original invalid memory accesses.
Reference callbacks must not throw or structurally mutate ownership storage;
caller-provided device arrays must remain stable across attachment. Services must
outlive the manager. Native append allocation failure releases the just-retained
reference, preserving the already-appended owned prefix. The original attachment
has no guard for that allocation failure; this deliberate difference avoids a
native leak.

## Validation and next integration boundary

Temporary ASan/UBSan/native and strict Windows i386 audits check constructor/
attachment limits, validation before mutation, duplicate refs, zero snapshots,
append-on-reattach, fresh graphics forwarding, poll/getter ordering, source-table
and fifth-count preservation, callback-visible cleared outputs, reverse release,
repeat detach and guarded malformed native calls. A separate allocation audit
forces failure on the first/second append, checks balanced references and verifies
second/first/binding/joystick allocation-free order.

The owner also runs original defaults through native map/snapshot/producer,
controller buffer views and front-end Escape/back, with held-key suppression.
The reset test confirms no implicit graphics map publication. Temporary audit
artifacts are removed after verification; no permanent tests or agent-side Proton
launch are added. Existing native checks and all 158 tooling tests pass; the
live host build is up to date. Evidence/accounting and diff checks also pass.

Follow-up: [graphics input setup and startup localization](startup-localization.md)
are now recovered in source. 006B67B0 resolves the installed language and
006B1090 preloads localized text, rather than profile/control settings. Concrete
device/list/archive setup, state owners and remaining application startup
prerequisites still precede controller wiring.
The existing live keyboard/mouse bridge remains separately desktop-confirmed.
No repeat desktop test is needed for this source-only owner.

The [map](input-manager-map.json) checks both hashes, 19 anchors, 31 graphics slots
and three calls. Six new partial client functions bring totals to 134 partial
(129 client/five engine), zero complete, frozen denominator 18,783. Raw relocation
helper research earns no additional function credit. Read-only original exports
are ignored under `analysis/work/input-manager-lifetime`, `input-manager-release`
and `input-manager-startup-boundary`.
