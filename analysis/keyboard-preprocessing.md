# Front-end keyboard preprocessing and text FIFO

This source-only slice recovers client `0066B3E0` and helper `0066B380` in
[`keyboard_preprocessing.h`](../recovered/frontend/keyboard_preprocessing.h),
using the existing screen/Escape tail. The live keyboard device integration is
separately desktop-confirmed, including focus loss/return. This new code does
not change the live menu or add text fields.

## Original order

The front-end blocked byte at +2D returns before any callbacks. Otherwise +44
receives both input buffers; availability global 008AE4E0 is checked afterward.
The ordinary menu's +44 is the known no-op 0067EA80, but the recovered shared
path retains the callback. IGraph +A0 supplies the scan before character delivery.

Legacy mode takes one IGraph +A8 value, truncates to an unsigned byte and calls
character target self+10, virtual +24, if present. A consumed character suppresses
screen key delivery. Wide mode drains low bytes through a zero terminator, at
most 1023 reads, then calls MultiByteToWideChar(CP_ACP, 0, source, -1, output,
1023). Conversion return value is ignored; the helper returns whether more than
one source byte was read, counting the terminator. Empty input still calls the
converter. Each nonzero UTF16 unit gets the original KEY_VALUE diagnostic and a
fresh character-target lookup. Consumption is latched, but delivery continues
through all remaining units, including separate surrogate units.

If no character consumes input, low scan 4E with Alt, Shift and Control queues
target zero/code 0200007D/argument zero and returns. Modifier queries occur in
that order with short-circuiting. This event is not live-wired by this slice.
For zero scan, fallback priority is Control, Alt, Shift; a left-side state query
selects left or right scan. With no modifiers, non-null secondary input's float
at +B8 synthesizes Escape when nonzero, including unordered/NaN. Zero and negative
zero return without screen delivery. That float is read after character callbacks,
not cached before them. The existing tail then handles screen consumption,
drag cancellation and queued back, resolving current screen afresh.

## Separate text queue

IGraph +A8 is `1006F2D0`, distinct from +A4 / `1006F470`'s scan-translated
character. [`text_queue.h`](../recovered/input/text_queue.h) reconstructs the
32-DWORD buffer at 100D1E48. Slot zero stores the last result; pending input
occupies slots 1..31. The WM_CHAR branch in graphics pump `1006A470` appends to
the first zero slot when lParam's high bit is clear, dropping input when full.
The getter shifts 31 DWORDs left before reading slot zero, then zeros slot 31.
+AC (`1006F300`) clears all 32 words. Values retain DWORD width in
storage; the front end performs byte truncation. Only this message insertion
branch is reconstructed, not the complete pump or keyboard-message mode.

## Scope and validation

Concrete application input-buffer production, character targets, ACP conversion
and diagnostic services remain required before live integration. No successful
stand-ins are provided. The normal menu has no new text-input flow. Focus entry
and navigation callers remain an open part of the checklist.

Native source zero-extends the meaningful scan byte; the original initializes
only the low byte of its local DWORD and forwards uninitialized upper bytes.
Native source throws on 1023 unterminated bytes rather than repeating the original
stack overread, and initializes wide output storage. Non-null secondary input
must contain at least 47 words; its native view does not recreate original class ABI.

Temporary ASan/UBSan audits cover 10,000 queue operations, capacity/drop/clear,
DWORD preservation, all nonzero scan bytes, modifier combinations and left/right
selection, shortcut gates, blocked/empty gates, consumed characters, multiple
UTF16 units and consumption latching, failed/empty conversion, callback-mutated
input, NaN/signed zero, drag/back and unterminated-source rejection. The same
instantiated code compiles strictly for Windows i386. Temporary audits are removed
after verification; no permanent tests or agent-side Proton run were added.
Existing native checks and all 158 tooling checks pass; the live host build
remains up to date.

The [map](keyboard-preprocessing-map.json) validates both original fingerprints,
26 instruction anchors, 12 graphics slots, one client slot and three direct calls.
`0066B380` earns one partial entry: 116 partial (112 client/four engine), zero
complete, frozen denominator 18,783. The other indirect functions are absent
from the frozen inventory and receive no new census credit. Original read-only
exports are in ignored `analysis/work/frontend-key-preprocessing`,
`frontend-defined` and `engine-character-fifo-defined`.
