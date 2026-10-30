# Original binding-to-input-buffer production

Client `006C7870` is reconstructed in
[`input/binding_producer.h`](../recovered/input/binding_producer.h). This is
source-only and supplies the producer missing from the earlier controller/input
recovery. It does not install guessed bindings or change the live menu.
Checkpoint `4e0d923` contains the preceding desktop-confirmed pacing/keyboard
integration and audited character preprocessing.

## Device and frame order

The manager is at 008AEA48 in the original main loop. Joystick records are
visited last-to-first; each optionally polls +58(1), then always gets current
+50 and secondary +54 snapshots. With polling enabled, graphics pump +14,
mouse poll +E8 and keyboard poll +9C follow in that order. All return codes are
ignored. Getters then read the packed 64-bit keyboard state (+B8), mouse buttons
(+104), signed X (+EC), Y (+F0) and Z (+F4).

The ordinary main-loop call at 006DCEE4 passes polling flag zero. Mouse and
joystick polling already occur outside the producer there; do not blindly
add a second device poll while wiring this source. The surrounding keyboard,
focus and frame ordering remains a separate integration task.

Mouse motion rounds signed integers to float, clamps magnitude to the configured
axis limit, then divides by that limit. The original does not supply a fallback
for zero, negative or NaN limits. Six mouse-count bytes and 64 keyboard-count
bytes are copied for this call. Persistent source gates update using
`gate = ~(gate & held) ^ held`. Releasing a source rearms its gate; a consumed
held source stays suppressed until release. Startup gate/configuration values
must come from the original initialization path.

## Binding evaluation

Twelve-byte records hold direction, 16-bit device kind, 16-bit selector and
32-bit device index. Evaluation is last-to-first into two DWORD vectors holding
float bits, matching `006C72B0` and the recovered controller's borrowed views.

| Kind | First output | Second output |
| --- | --- | --- |
| Keyboard (1) | 1 when first matching configured scan's bit is held | 1 while its persistent gate permits delivery |
| Mouse button (2; codes 1/2/4/8/16) | 1 when that button is held | 1 while its persistent gate permits delivery |
| Mouse axis (2; codes 100/200/300 hex) | Normalized X/Y/Z, optionally positive-only or negated negative-only | Copy of first output |
| Joystick analog (3; selectors 0..7) | Current axis with the same direction filter | Copy of first output, regardless of secondary snapshot |
| Joystick button (3; 100..1FF hex) | Current button bit | Secondary button bit |
| Joystick POV (3; selectors at least 200 hex) | Current angle direction mask if available | Secondary angle direction mask if available |

Digital counts decrement as bytes, including zero-to-255 wrapping. The last
configured binding to use a source clears its persistent gate. Reverse order
matters when counts and bindings disagree. A missing keyboard scan yields a
zero mask via the original 64-bit shift helper; it must not alias bit zero.

Unknown kinds preserve both previous output words. Unknown mouse selectors
clear first output but preserve second. Joystick processing clears both first;
an absent device or unrecognized analog selector leaves them zero. These are
original branches, not native defaulting policies.

POV selectors take their index from bits 4..7 and direction from the low nibble.
The classifier preserves the exact float radian bounds 0.1, 1.5, 1.6, 3.12,
3.16, 4.69 and 4.72, including boundary asymmetries. Its NaN branch yields mask 4.
Native code rejects indices outside the four-POV arrays instead of following
original out-of-bounds reads.

## Configured keyboard snapshot

[`input/keyboard.h`](../recovered/input/keyboard.h) also recovers IGraph +B4 /
`1006F4B0` map installation and +B8 / `1006F500` packed snapshot production.
Installation rejects null input or counts above 64 with 80000007, preserving the
old map. Otherwise it zeros all 64 bytes and copies the configured prefix. A
nonnull zero-length input succeeds. Aliasing the cleared map retains the
original zero-before-copy behavior.

Snapshot bits follow map positions, not scan numbers. A nonzero scan whose
current state byte has its high bit set contributes its position's bit. Scan
zero is disabled; duplicate scans can set multiple bits. This map is distinct
from the fixed character-translation table.

## Validation and remaining integration

Temporary ASan/UBSan checks cover every packed position/state-byte combination,
map validation/aliasing/zero fill, device/getter order with both polling modes,
all 256 digital count values, reverse shared binding delivery, five buttons,
three motion axes and direction/sign/saturation cases, all eight joystick axes,
32 button bits, four POVs, invalid-device and original stale-output paths.
POV results match the original x87 status-word branches over 100,000 sample
angles, exact boundaries and their adjacent floats, infinities and NaN.
The actual native producer feeds controller buffer views into front-end
preprocessing: Escape emits back, holding suppresses it, release/repress rearms
it. The same instantiated audit compiles strictly for Windows i386. Existing native
checks, all 158 tooling checks and the strict Windows host build also pass.
Temporary audit files are removed after verification; no permanent tests or
agent-side Proton launch were added.

Follow-up: [binding installation and both original default layouts](binding-installation.md)
are now source-recovered and audited, including 006C7480 and mode/dirty
application. Actual manager creation/device ownership, profile persistence and
remaining startup services still precede live integration. Native vectors replace original allocator/vector/class ABI.
Callbacks must not invalidate borrowed joystick output storage. Undersized native
output vectors throw rather than reproducing original memory corruption. Native
float arithmetic does not recreate x87 exception state or signaling-NaN behavior.
No application service is replaced with a successful no-op.

The [evidence map](binding-producer-map.json) validates both hashes, 41 byte
anchors, two engine slots and three direct calls. New partial entries are
client `006C7870` and engine `1006F4B0`: total 118 partial (113 client/five engine),
zero complete, frozen denominator 18,783. `1006F500` is absent from that inventory;
no new credit is claimed for it. Original exports remain ignored under
`analysis/work/startup-input-prerequisites` and `engine-keyboard-snapshot`.
