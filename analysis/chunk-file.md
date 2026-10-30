# Original application chunk-file lifecycle

[`io/chunk_file.h`](../recovered/io/chunk_file.h) reconstructs the original
read/write chunk-file object used by the application data and profile loaders.
The [DTA boundary](../recovered/host/chunk_files.h) resolves checked exports in
the same borrowed rw_data module used by the engine and archive registration.
This is source-only. No original file API, game or Windows executable was run;
all write audits use memory buffers.

The early [`ChunkReader`](../recovered/io/chunk_reader.h) remains a separate,
strict portable research reader. Its parent/file-length checks and exception
behavior are useful for inspecting assets, but cannot silently replace the
original object's boolean results, partial-transfer effects or write protocol.
No new successful application-loader service stubs were introduced.

| Original entry | Recovered behavior |
| --- | --- |
| 00405620 | Open read/write, root end, rewind and opened flag |
| 00405690 | Destructor's file close phase |
| 00405750 | Explicit close with byte result |
| 00405820 | Push writer header position and emit six-byte header |
| 00405890 | Leave/read seek or writer size backpatch, optional size output |
| 00405940 | Mode/boundary-gated payload read |
| 004059B0 | Mode-gated payload write |
| 004059F0 | One-shot header/data/backpatch, ignoring operation results |
| 00415C90 | Raw header read and caller WORD output |

## State and lifetime

State has opened byte +00, handle +04, write-mode byte +08, 257 DWORD offsets
at +0C and depth at +410; size 414 hex. Native static assertions check those
offsets independently of pointer size. Index zero is the root end (or FFFFFFFF
for write open). Deeper entries store read chunk ends or writer header starts.
The native owner starts from zeroed fields/stack and invalid handle; arbitrary
original uninitialized placement is not modeled.

Open returns false immediately when already opened. Otherwise set root offset
to FFFFFFFF, call dtaOpen(flags0) or dtaOpenWrite(flags10000 hex), and store the
returned handle. ONLY FFFFFFFF fails; zero and other high-bit handles are valid
to this helper. On success store the actual mode byte, query end for read mode,
rewind to zero and set opened. Both seek results are ignored. Neither open nor
close resets depth. Close in read mode closes directly without unwinding; a
subsequent open retains stale nesting. In write mode it leaves/backpatches every
open chunk before closing. Handle becomes FFFFFFFF and opened becomes zero
AFTER the close callback. Mode and offsets persist. The native destructor uses
the same close phase; explicit close returns false if already closed.

## Chunk and transfer semantics

Enter reads six bytes directly through dtaRead, without payload-read gates.
Require exactly six bytes to calculate `current_position - 6 + total_size`,
push that wrapping 32-bit end and update the caller's WORD id. A short header
can consume bytes but leaves id/depth unchanged. There is no mode gate, minimum
size, parent-end, file-end or failed-position validation in the original.
Sizes below six, wrapping ends and entering in write mode retain that behavior.

Begin writes a little-endian WORD id followed by DWORD zero. It records current
position and increments depth BEFORE writing. A short write therefore returns
false while retaining a live stack entry. It has no write-mode check. Payload
write DOES require open and nonzero write mode, and returns exact transfer-count
equality. The one-shot helper emits the header, writes mode-gated data, then
leaves, ignoring individual operation results.

Payload read requires open/read mode. Only nonzero depth checks a boundary:
take the wrapping end-minus-position word, interpret signed, clamp negative to
zero, and reject a signed requested count larger than that. At depth zero the
root file size does NOT bound the read. Return true when the signed reported
read count is at least the signed requested count. Short reads retain partial
buffer/position effects. No automatic retry, seek rollback or output clearing.

Read leave seeks the current end, does not touch optional size output, and pops
only when depth is nonzero. Writer leave captures position, computes wrapping
size from header start, seeks header+2, writes four little-endian size bytes,
restores captured position and optionally stores size output. All underlying
results are ignored; both leave paths return true whenever opened, even at
depth zero. Writer close repeats that operation until depth zero.

## Concrete DLL and native limits

All file exports use stdcall. Checked RVAs: dtaOpen 12C1, dtaOpenWrite 145B,
dtaSeek 1528, dtaRead 152D, dtaWrite 101E, dtaClose 14F1. The adapter preserves
32-bit argument/result bit patterns. rw_data's search precedence, archive
indexes, handles and codecs remain delegated; this work recovers the client
file object rather than reimplementing that DLL. Module/backend must outlive
the owner. Callbacks must not throw or structurally mutate owner state.

Native guards reject stack pushes past depth 256 before touching the backend,
null nonempty buffers, and read counts above INT32_MAX (unsafe original negative
count cases). Native state/storage replaces original object construction and
raw-pointer ABI. These are explicit differences; all entries remain partial.
This component does not recover full GameData/HumanStore/profile record parsers,
their loaded objects, startup protection or controller wiring.

## Validation and next dependency

Temporary ASan/UBSan audits compare nested writers at depths 0..31 against an
independently generated byte format, including automatic close backpatching.
They cover open modes/flags, stale read depth across reopen, depth-zero reads
and leaves, optional output preservation, malformed sizes/wrapping, high-bit
handles, ignored failed seeks, partial header/payload writes and reads, exact
write vs signed-at-least read returns, read-mode writer calls, one-shot output,
256-depth guard and cleanup. Strict i386 fixture instantiates every owner method
and concrete DTA adapter and links successfully; it is NEVER executed.

Two ignored samples extracted read-only from base Others.dta independently
validate the new reader: HumanStore.def (9,170 bytes, 651 chunks/fields) and
Gamedata00.gdt (22,161 bytes, four verified container/chunk levels). Every
verified payload matches independently decoded raw offsets; reads beyond each
leaf boundary are rejected without consuming bytes. HumanStore contains 65
record chunks 2328 hex inside root 029A; this gives the next application-data
slice a real installed fixture. GameData has root0003 metadata then root0001;
arbitrary embedded payload bytes were not guessed to be chunk containers.

Existing native checks, all 158 Python tooling tests and unchanged host build
pass. Temporary audit sources/executables/extracted samples are removed after
verification. [Evidence](chunk-file-map.json), checked with
`python3 -B tools/validate_chunk_file.py`, validates all three fingerprints,
22 instruction/data anchors covering all nine original body ranges plus
import/export thunks, and 34 direct calls. Seven altered fingerprints/bytes/
calls are rejected by a temporary negative audit. Read-only original exports
are ignored under analysis/work/chunk-file-lifecycle.

Four existing function entries gain original DTA/write coverage; five new
partial client entries bring totals to 167 (162 client/five engine), zero
complete, frozen denominator 18,783. No duplicate credit for portable/native
versions, and no new credit for delegated DLL calls.

Next: port actual HumanStore and GameData record semantics onto this object,
then profile selection and remaining application/state prerequisites. Original
protection-state effects and full executable startup remain unresolved.
