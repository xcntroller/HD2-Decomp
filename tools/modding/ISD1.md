# Verified ISD1 raw/LZSS and checksum milestone

The resource backend now decodes actual selected Patch mission content. It does
not substitute an older ISD0 copy, run rw_data.dll, or require Ghidra at runtime.
`isd1.py` owns bounded container parsing/integrity/decoding; the GUI consumes its
outcomes through `resource_store.py` and `installation.py`.

## Evidence and scope

Existing read-only exports establish archive setup10003763 and member open1000C110.
Targeted **static objdump** inspection of the already fingerprinted original
rw_data body1000513B establishes the distinct ISD1 block-read path. Checksum
10009E97 and XOR10009BF3/10009D24 were inspected statically; decoder thunk100013CF
points to1000EA20. No original executable, DLL body or copied original machine
code was executed. No Ghidra process was required.

[isd1-evidence.json](isd1-evidence.json) preserves the original binary SHA256,
address/size/code-range hashes, input archive fingerprints and independent
capture fingerprints. No original bytes or raw exports are committed. The
existing client/server reconstruction and function inventories were not changed.

Supported: raw tags0/2..7 and LZSS tag1, encrypted or clear stored payloads,
multiple32KiB decoded blocks, short final blocks and the empty one-block raw
members present in Patch. Tags>=8 enter original sound handling and remain
**unavailable**, including32. ISD0's existing bounded codec32 support does not
imply support for the ISD1 sound envelope. No new sound/asset family is decoded.

## Integrity algorithm (10009E97)

The game's so-called CRC is a custom16-bit additive/XOR checksum, not a standard
polynomial CRC16. For each byte b, starting from seed00FF unless specified:

```
s = ((s + b) & 0xFFFF) ^ (b ^ 0xFF)
```

Chaining uses the previous WORD result as the next seed. It is deliberately
faithful and weak: collisions, including some bit changes, exist. Passing it
is not cryptographic authentication. SHA256 comparisons against existing captures
provide an additional, independent output check in this milestone.

The native program sometimes checks only when its error callback is installed
and can invoke that callback before continuing. Inspection always rejects a
mismatch and publishes no partial decoded member. It does not emulate that
callback/error ABI or attempt recovery from corrupt selected content.

## Actual storage rules

All integral fields below are little endian, except LZSS's established token/flag
encoding handled by the reused primitive.

| Structure / offset | Meaning and verification |
| --- | --- |
| Archive0..3 | Literal ISD1 magic |
| Archive4..23 | Separately XOR-decoded five DWORDs: count, index offset, index bytes, index checksum, header checksum |
| Header checksum | Check plaintext magic + decoded20-byte header with its final DWORD zeroed; seedFF |
| Index | count×28 bytes; checksum **stored ciphertext before XOR**; then decode the index |
| Index entry0/4/8 | Uppercase name length/sum lookup hash, member header offset, payload offset; remaining16 bytes retained only through source/index provenance |
| Member header | Separately XOR-decoded32 bytes; decoded length+10, block count+14, DWORD header checksum+18, name-length/encryption WORD+1C, metadata checksum WORD+1E |
| Member header checksum | Decoded32 bytes with DWORD+18 zeroed; seedFF |
| Name | Low15 bits of+1C give length; separately XOR-decoded; high bit is payload encryption |
| Block descriptors | block_count plain DWORDs after name: low WORD stored length, high WORD stored-block checksum |
| Codec tags | block_count ciphertext bytes after descriptors; separately XOR-decoded |
| Metadata checksum | Chain **plaintext name**, plain stored descriptor table, **ciphertext codec tags**, startingFF; compare member+1E |
| Payload | Starts immediately after codec table, matching the index payload offset; blocks are contiguous with no ISD0 four-byte block headers |
| Block checksum | Check each stored block before any payload XOR, seedFF, against its descriptor's high WORD |
| Payload XOR | If member encryption flag set, restart repeating eight-byte key at each block; metadata segments also restart individually |

1000C110 splits descriptor DWORDs into length and checksum arrays. 1000513B
seeks by cumulative lengths and reads ISD1 bytes after a reserved tag slot.
10005F06..100060AE verifies stored bytes before conditional XOR. At100060AF the
separately decoded tag is prepended for dispatch. Thus tag1 invokes the **same**
1000EA20 primitive as ISD0, justifying reuse of `decompress_lzss`; this is not an
assumption that archive framing matches ISD0. Other tags below8 use byte copy;
sound tags>=8 branch elsewhere and are explicitly rejected by this decoder.

Decoded block sizes are min(32768, remaining decoded bytes). Stored length is
independent of decoded size. Every decoded block and the final member length
must match; malformed LZSS never returns a truncated success. Zero-length members
with a single raw/zero-stored block are verified, not confused with a missing
file. Arbitrary historical zero-block open behavior is not reconstructed.

## Bounds and failure handling

Index/header/member lengths, name hashes, metadata and physical nonoverlap are
validated before payload allocation. Counts must fit index size and decoded
block count; source/payload spans cannot cross the index or another member.
The selected archive's stat snapshot must still agree at read time.

Limits:100,000 members/archive,4096 bytes/name,16MiB cumulative names,262,144
block descriptors/archive and64MiB decoded payload/read. These are explicit tool
limits, not original format maxima. Metadata/name buffers are discarded after
indexing; member bytes are decoded on demand and discarded after interpretation.
Unknown codecs still have all their stored block checksums verified when read,
then raise unavailable without publishing output. An integrity failure takes
precedence over that unsupported-codec result.

Exception categories distinguish checksum, bounds, unsupported compression,
inspection limits and ordinary unavailability. A failing winning archive/member
never silently selects a shadowed lower-priority copy. Invalid metadata forms a
conservative resolution barrier because affected path absence cannot be trusted.
Loose-first and archive-first profiles and registration order are unchanged.

## Export and GUI states

Mission schema2 gains additive `FileReport.archive_decoding` and selected-source
codec/support/integrity metadata. The script CLI schema remains1. Reports remain
deterministic for the same inspected state, with no times or private root paths.

The file browser has separate **Content status** and **Archive** columns; the
selection heading and details expose the winning origin. For example, a patched
ALPS1 scene has content `decoded_subset` but archive `decoded_verified`. Its
checksums and bytes are verified while its internal scene interpretation remains
partial. This is not full mission compatibility.

- `decoded_verified`: supported ISD1 payload fully decoded, all container/stored
  checksums passed; the file SHA256 describes the resulting member bytes.
- `decoded_size_checked`: existing ISD0 decoder, without invented CRC protection.
- `invalid_resource`: checksum mismatch or malformed archive/payload bounds;
  records, script results and output hash are cleared.
- `unavailable_resource`: unsupported codec or unavailable selected input;
  codec/metadata/stored-checksum status is explicit. Limits remain `limited`.
- `unsupported`: content format not interpreted. Its container can still be
  decoded/verified independently on explicit inspection.

Graph edges/validation preserve exact selected-file provenance. Source selection
is separate from script syntax, scene parsing and inferred engine/name matches.
`coverage.complete_format_support` remains false. No editing or serialization
back to mission binaries is implemented.

## Verification performed

A consolidated original Patch sweep checked all1059 members: **1053 raw/LZSS
members decoded,259,360,081 output bytes**, while six sound members had stored
checksums checked and returned unavailable. All4560 LangEnglish and1059 Patch
member/header/index/metadata checks passed. This is a verified subset on these
archives, not support for every ISD1 variant or sound codec.

The existing user-produced capture manifest supplied32 present resources and
five absences. All32 selected outputs matched **byte-for-byte** and by their
recorded sizes/SHA256; all five absences agreed. Key actual Patch selections:

| Resource | Bytes | SHA256 |
| --- | ---: | --- |
| missions/ALPS1/scene2.bin |6727085|ba91c84be678571d5567594a1373a6197731f5e866c6bc0a95b1d1830ecae274|
| missions/ALPS1/actors.bin |41052|2523c437d5391fdedeb39b3094488fe1a490dc5cad44ad7f2b4397a5c2d8675b|

Full installation sessions now inspect Tutorial275 files/2139 entities/5345 graph
edges and ALPS1246 files/4452 entities/8586 edges on this dataset. Selected ALPS1
Scene2/actors parse to their existing documented subsets; no invalid/blocked
selected resources occurred in those sessions. These counts are observations,
not general compatibility criteria. Structured schema2 exports were deterministic.

Independent synthetic construction (split-byte reference checksum, not the
production routine) covers every checksum layer, clear/encrypted raw and LZSS,
multiblock key resets, empty/short blocks, all small-file truncated prefixes,
invalid offsets/counts, malformed LZSS, unsupported sound and changed payloads.
Corrupt winning Patch data remains invalid despite an available loose fallback
in archive-first mode. Nonempty supported block lengths are checked exactly.
Known checksum collisions are not incorrectly asserted detectable.

Actual native GUI checks opened both patched ALPS1 documents, confirmed their
provenance/hash/archive column, and distinguished unavailable real sound data
from invalid synthetic CRC data in validation. The rendered window was inspected.
This is agent desktop verification, not new user acceptance or original-game
execution. Python compilation and the existing small inspector build passed;
no new native code changed, so no new sanitizer run is claimed for Python code.
All fixtures, raw static disassembly, screenshots and extracted audit material
were temporary under /tmp, removed after verification; no permanent tests or
commercial assets were added. Audit identities are in the workstream handoff.

Temporary audit source SHA256 identities (removed after verification):

| Audit | SHA256 |
| --- | --- |
| Independent checksum/storage fixtures and failure states |d439be047790b05a447c2772d2574f8046d96469e3d7a7be1036617b92f4bb43|
| Original capture/Patch sweep/mission exports |f744fe25aeb7176d9c047f7b0a20961eb0c793e824e07ec00c871ff038b4ade1|
| Native GUI flows |6c7d0139180eec699ad0883e661abd95f54e36f1e32ba219602583af01f6be8e|
| Initial independent metadata-layout census |67959b243a46e34080bd86035a27d315087973e8a5dc4cd818b90d34c1446e33|
