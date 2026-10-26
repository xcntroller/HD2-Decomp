# Readable reconstructions

This directory contains our readable reconstructions of small, understood
behaviors. It is **not original developer source**, a dump of Ghidra output,
or a rebuilt game.

The first component is the read-only
[chunk reader](io/chunk_reader.cpp), with its [public interface](io/chunk_reader.h).
Its names and API are ours; its supported behavior is based on the observed
base-game chunk helpers. The [evidence map](../analysis/reconstruction-map.json)
records addresses, scope, and intentional differences.

## Chunk reader

The reader:

- measures the input length and rewinds to its beginning;
- reads six-byte little-endian headers: a two-byte ID and a four-byte total
  length, including the header;
- keeps a stack of active chunk ends;
- bounds payload reads by the current chunk;
- skips unread payload when leaving a chunk;
- treats file length as the boundary outside any chunk.

It accepts a caller-owned, seekable `std::istream`. Keep that stream alive and
do not move its cursor independently while using the reader. Destroying the
reader does not close the stream. `enter_chunk()` returns `std::nullopt` at the
current boundary; `leave_chunk()` at depth zero seeks to the file end, matching
the observed read-mode helper's depth-zero behavior.

### Supported subset and differences

This is a portable C++17 implementation of the **read-only, valid-input subset**,
not the game's original binary layout or archive interface:

- `std::istream` replaces `dtaRead`/`dtaSeek`. DTA opening, archive precedence,
  ownership/closing, write mode, and chunk-size backpatching are not reconstructed.
- The original helpers generally return Boolean status. This API throws
  `ChunkReadError` for validation and ordinary stream failures; a stream configured
  to throw I/O exceptions may also propagate its own exception.
- Truncated headers, lengths below six, chunks crossing their parent/file end,
  failed seeks, and reads crossing the file boundary are rejected explicitly.
  The observed game code does not consistently enforce these checks.
- Nesting is capped at 256, consistent with the inferred end-stack capacity.
  This is an explicit safety guard, not evidence of an original runtime check.
- Files above signed 32-bit offset range are unsupported. Arithmetic uses wider
  internal offsets instead of reproducing potential x86 overflow.
- No gameplay meaning is attached to a chunk ID. No world transforms, object
  creation, or runtime scene state are simulated.

After an I/O failure, discard the reader rather than assume cursor rollback.
Malformed headers may have been consumed before their lengths were rejected.
The reader never silently substitutes data or reports a failed seek as success.

## Build and test

From the repository root, with a C++17 compiler, GNU Make, and Python 3.11+:

```sh
make -C recovered test
```

This builds with strict warnings and runs:

- 16 C++ tests for headers, payloads, nesting, skipping, bounds, short reads,
  failed seeks, stream positioning, and the signed offset limit;
- 37 deterministic synthetic scene comparisons against the independent Python
  inspector;
- 9 malformed-scene cases that both scanners must reject explicitly.

No game files are needed for those tests. Build products go into ignored
`recovered/build/`.

To cross-check an already extracted local mission:

```sh
python3 -B recovered/tests/compare_scene_scan.py \
  --scanner recovered/build/scan_scene_chunks \
  --scene extracted/alps1-base/scene2.bin
```

This compares file size, top-level/container/record counts, every immediate
record-field count, and unknown record IDs. For the recorded ALPS1 sample, the
reader matches 15 top-level chunks and all 4,008 records. It does not decode
payload semantics or check names; use the existing Python inspector for that.

An address/undefined-behavior sanitizer build can stay in ignored analysis output:

```sh
make -C recovered BUILD_DIR=../analysis/work/recovered-sanitized \
  CXXFLAGS="-std=c++17 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer" \
  test
```

The [scene scanner](tools/scan_scene_chunks.cpp) is a test driver for the reader,
not recovered mission-loader logic. It follows only the established root,
object-container, and object-record levels and emits aggregate JSON without
asset contents. Additional unknown payloads are skipped, not guessed.

## Evidence standard

Each further reconstruction should identify its input binary and static
evidence, use descriptive names, state the supported behavior and deviations,
and include tests. Original addresses belong in the evidence documentation,
not in normal reconstructed logic.

These tests compare reconstructions and file observations, **not execution of
the original game helpers**. Runtime equivalence, ABI compatibility, binary
matching, and a complete source reconstruction have not been established.
