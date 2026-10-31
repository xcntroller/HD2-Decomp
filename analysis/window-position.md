# Startup window position persistence

[`frontend/window_position.h`](../recovered/frontend/window_position.h) recovers
only the window-position portions of 006DBFC0 and 006DC3A0. The original
client and engine bytes, window vtable slot and file-name references are
fingerprinted by [the evidence map](window-position-map.json). The containing
startup and teardown wrappers are not reconstructed.

After input/localization and a protection-state operation, startup calls
`dtaOpen("window.cfg", 0)`. A negative signed handle skips the entire block.
Otherwise it reads four bytes each into the left and top stack words, closes the
handle even if it is zero, fetches the graphics window, calls SetWindowPos with
x=left, y=top, width=height=0 and flags 0x15, then fetches graphics/window again
and calls SetFocus. All read and window-operation results are ignored. The two
reads use distinct existing stack locations and may be short; the native caller
supplies their initial bytes, including any effects of preceding protection code.
The helper does not invent their values.

During 006DC3A0, after five earlier application cleanup calls and before cache
and driver destruction, `dtaOpenWrite("window.cfg", 0x10000)` starts saving.
A negative handle skips everything. Otherwise the client fetches the window,
calls GetWindowRect, writes its left and top as separate four-byte little-endian
words and closes. It ignores all results. On a failed GetWindowRect the original
uses whatever remains in its stack rectangle. The native boundary accepts caller
initial bytes and preserves them on failure; Win32 does not promise output on
failure, so it cannot reproduce arbitrary partial modifications in that case.
The helpers return whether the file opened for the native caller's convenience;
this is not an original startup success condition. No automatic cleanup on
exception or additional I/O error policy is implied.

The concrete adapter forwards to the borrowed original rw_data module, shared
with archive and text services, and retrieves the original graphics singleton
and checked +48 window accessor at each call. Module and archive lifetimes
remain the caller's responsibility. The [shutdown sweep](shutdown-delivery.md)
now connects save before device closure. A successful real rectangle snapshot
seeds the subsequent ignored rectangle failure; without a valid snapshot, saving
is skipped. User desktop acceptance now records an eight-byte saved position
(left47/top21); see the delivery evidence. Startup restore remains source-only,
and full setup/protection/original cache lifecycle remain open.

Validation: temporary ASan/UBSan native audit covers negative and zero handles,
all short-read lengths, signed coordinate decoding, operation ordering, ignored
write returns and output bytes. Strict i386 fixture instantiates and links both
adapters without execution. Existing native suite, 158 Python tests, unchanged
host build and fingerprinted evidence map pass. No permanent tests added.
