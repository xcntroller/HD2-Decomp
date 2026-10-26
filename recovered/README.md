# Readable reconstructions

This directory contains our readable reconstructions of small, understood
behaviors. It is **not original developer source**, a dump of Ghidra output,
or a rebuilt game.

The first component is the read-only
[chunk reader](io/chunk_reader.cpp), with its [public interface](io/chunk_reader.h).
Its names and API are ours; its supported behavior is based on the observed
base-game chunk helpers. The [evidence map](../analysis/reconstruction-map.json)
records addresses, scope, and intentional differences.

The second component is the
[model resource key helper](scene/model_resource_key.cpp), with its
[public interface](scene/model_resource_key.h). This is a small string
transformation, not a recovered model loader.

The [frame-kind lookup](scene/frame_kind.h) names the 16 engine types supported
by the DLL's type-name query. The
[model-load policy](scene/model_load_policy.h) classifies candidate results.
Neither component creates frames or loads models.

The [model-file header reader](scene/model_file_header.h) implements only the
initial 16-byte checks performed by the engine's model-file loader.

## Experimental engine-window host

[The graphics-only host](host/engine_window.cpp) is independently written
integration glue, **not another recovered original function or a running menu**.
Its [56-byte settings layout](host/graphics_settings.h) is statically supported.
The user has now observed the original-engine windows and blue clear-color
surface in a desktop Proton run, followed by shutdown.

It is an optional i386 Windows target:

```sh
make -C recovered engine-window
```

Set `WINDOWS_CXX` to an i386 MinGW-compatible compiler path when needed; see
[the technical build/launch instructions](../RESEARCH.md#graphics-only-host).
Normal `make test` remains native and does not require a Windows compiler.
The launcher fingerprints the original DLL and uses its own ignored Proton
prefix/working directory. Launch inside the agent environment fails before host
entry; launch from the user's desktop terminal displayed the test surface.
Exact frame count, exit status and full cleanup remain unconfirmed.

The intended first test clears/presents an engine-owned window and exits through
`WM_CLOSE` after a bounded frame count. There is no replacement menu artwork,
archive setup, menu text, original UI input system or logo playback in this host.
Neither the original executable nor the user's game prefix is patched.

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

## Model resource keys

The object loader's `0x2012` handler reads a model resource name, and a later
step scans backward for the last dot. If present, it terminates the string
there. The recovered helper reproduces that **string-content transformation**:

| Input | Key |
| --- | --- |
| `crate` | `crate` |
| `crate.4ds` | `crate` |
| `crate.variant.4ds` | `crate.variant` |
| `.hidden` | empty |
| `folder.name/object` | `folder` |

The last example is intentional: the observed scan is not aware of directory
components. Do not replace it with filesystem extension handling. Case,
whitespace, and path separators are otherwise preserved.

The input is already-decoded name content without its serialized NUL terminator.
Embedded NUL bytes are rejected explicitly. `std::string` replaces the game's
copy-on-write storage and bookkeeping. Resource lookup directories, extension
addition, actual frame creation, cache behavior, and successful loading remain
outside this helper's scope.

## Frame kinds and model-load decisions

The frame-kind enum uses the engine's numeric kinds and `FRAME_*` names.
Unknown numeric kinds return an empty optional; the original engine's formatted
fallback is deliberately not recreated. The scene inspector retains the
serialized numbers alongside any recognized labels.

`classify_model_load_result()` accepts the raw 32-bit result bits. Nonnegative
results select `accept`; `0x80000002` and `0x8000000A` select `stop_failure`.
Other negative results select `try_next_candidate`, with separate diagnostics
for `0x80000013` (bad file version) and `0x80000014` (bad file time).
The caller must consume both action and diagnostic. Trying another candidate
is conditional on one remaining; accepting a result is not a guarantee that
the complete load succeeds.

This pure classifier reproduces inline branch decisions, not the retry loop,
logging, file search, engine method, or cache. Unsigned result bits avoid a
platform-dependent conversion to signed integers. Evidence and original
addresses live in the [engine map](../analysis/engine-model-map.json) and
[policy map](../analysis/model-load-policy.json), not in reconstructed logic.

## Model-file headers

`read_model_file_header()` reads a 16-byte prefix from the stream's current
position: the exact `4DS` plus NUL signature, little-endian version `0x29`,
eight opaque metadata bytes, and a little-endian first-table entry count.
The count limit is **inclusive**: 6,000 is accepted; 6,001 is rejected.
The first table's complete meaning is not yet established, so its neutral name
does not claim that its entries are soldiers, models, or materials.

The reader leaves subsequent data unread and supports nonseekable input.
`ModelHeaderError` exposes a typed reason for incomplete reads, invalid signature,
unsupported version or excessive count. A stream configured to throw I/O
exceptions may propagate those exceptions. After failure, do not assume the
input cursor was rolled back.

Unlike the original archive calls, short reads are explicitly rejected. The
source does not open/close archives, modify engine globals, interpret the eight
unknown bytes or construct objects. **An accepted prefix does not validate the
complete model file.** It is another partial reconstruction, not a complete
original function for the progress bar.

## First model-file table

The [first-table reader](scene/model_first_table.h) reads a specified number of
records from the stream's current position. Use the preceding header's count;
both components share the 6,000-entry limit. The reader leaves the following
frame section untouched.

It preserves serialized flags, 72- or 76-byte opaque payloads, raw length-prefixed
name fields, and optional sequence counts/parameters. Optional names distinguish
absence from an encoded zero-length name. Embedded NUL and non-ASCII bytes are
preserved; the original serialized length, not C-string contents, controls
whether a secondary field exists.

This is only the file-reading subset of two engine routines. There are no engine
allocations, renderer options, resource requests, generated sequence filenames,
playback or opaque-field interpretation. Runtime object-flag changes are not
recreated; the evidence explains why they do not change the serialized read
layout. `ModelTableReadError` reports incomplete fields and excessive counts.
Configured stream exceptions may propagate. No cursor rollback is promised.

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
- 9 malformed-scene cases that both scanners must reject explicitly;
- 14 C++ model-key cases, including an embedded-NUL rejection;
- 71 deterministic model-key comparisons against Python;
- frame-kind checks for all 16 supported values and unknown values;
- 17 model-load-result cases covering success, retry, stop, and diagnostics;
- 39 model-header cases, including exact count boundaries, all prefix truncations,
  opaque-byte preservation, stopping positions and stream behavior;
- 30 synthetic model-header comparisons and 27 malformed headers both readers
  must reject;
- 1,002 native first-table cases, including all 128 flag/primary-presence
  combinations and every truncation of a maximal record;
- four first-table comparison fixtures (including mixed records and 6,000
  entries), plus 107 malformed tables both readers must reject.

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

To compare model keys used by the real sample:

```sh
python3 -B recovered/tests/compare_model_resource_keys.py \
  --normalizer recovered/build/model_resource_key \
  --scene extracted/alps1-base/scene2.bin
```

The ALPS1 sample has 1,157 model-resource references using 61 distinct names.
All of those names are extensionless, so the suffix-removal edge cases are
established by assembly inspection and synthetic tests rather than that sample.
The comparison driver does not print the real asset names.

To compare the already extracted local model-file prefixes:

```sh
python3 -B recovered/tests/compare_model_headers.py \
  --reader recovered/build/inspect_model_header \
  --model extracted/alps1-base/scene.4ds \
  --model extracted/alps1-base/loader.4ds
```

Their first-table counts are 86 and 13 respectively. The mission evidence
validator checks the exact prefixes against the fingerprinted samples.

To compare all first-table fields and record boundaries in those same files:

```sh
python3 -B recovered/tests/compare_model_tables.py \
  --scanner recovered/build/scan_model_first_table \
  --model extracted/alps1-base/scene.4ds \
  --model extracted/alps1-base/loader.4ds
```

The aggregate scanners print no asset names. Detailed `--records` output from
the native scanner contains local raw fields; do not publish it as source.

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
