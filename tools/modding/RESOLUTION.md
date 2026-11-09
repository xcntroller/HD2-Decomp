# Installation discovery and resource resolution

This is a bounded read-only implementation of the established **base startup
profile**, not a capture of a running game. No DLL/executable, engine, Ghidra,
private cache registry or archive extraction directory is used during operation.

## Verified policy reused

[Original research](../../analysis/startup-archives.md),
[lookup evidence](../../analysis/startup-archives-map.json), and
`recovered/io/startup_archives.h` establish:

- rw_data 10004B8C: flags' low byte and global override 100478DC both zero →
  loose first, archives second. Otherwise archives first, loose second.
  Fallback occurs only on -1, not on an unsupported payload or decoding failure.
- 1000C110: archives are searched in registration order, not newest-first.
- 006DB6E0: selected language, English creation fallback, Patch, Models, Missions,
  Maps, Sounds, Scripts, Others, Maps_C, Maps_U. Existing documented keys are
  reused. Missing archives do not stop later registrations.
- 006DBE60 resource-directory/cache registration is distinct from dtaOpen path
  lookup. Filename candidates for cache-backed resources are **not** represented
  as proven engine cache resolutions.

The GUI exposes language and loose-first/archives-first choices before indexing.
The exported profile records the chosen low-byte flags (0 or 1) and override 0;
this covers the equivalent established search orders without claiming the live
settings are known. Changing the controls requires reindexing. The runtime's
opaque protection setup, full file-error ABI, language selection from registry,
resource-cache enumeration and expansion/PatchX01 registrations are excluded.
Language archive fallback is based on local availability; an unindexable present
archive is conservatively a barrier, not an excuse to choose base content.

## Archive coverage

`resource_store.py` indexes loose files and only the established archive names.
For ISD0, it **reuses** `tools/extract_dta_member.py` and its existing raw/LZSS/
bounded codec-32 support rather than duplicating decompressors. Each payload is
read on demand into bounded memory; there is no asset extraction output.

ISD1 **metadata only** is supported. Previously exported rw_data 10003763 proves
its XOR-decoded 20-byte header fields and 28-byte index entries. The existing
1000C110 export proves the 32-byte member header, name flags at +1C (length low
15 bits), separately XOR-decoded name and uppercase length/sum lookup hash.
The inspector checks bounds, names, duplicate paths and name/index hashes. This
extends index inspection only: original CRC/checksum verification and ISD1 block
payload reconstruction are **not implemented**. Header/member offsets and source
sizes remain visible. No function-body completion credit is claimed.

The available original metadata passed all 4,560 LangEnglish and 1,059 Patch
name/index checks, agreeing with the earlier archive census. Seven ISD0 indexes
retain their documented 53,612 members. This does not establish general ISD1
compatibility, CRC equivalence or support for other installations/expansions.

If an ISD1 member wins, resolution is `selected_payload_unsupported`, and the
file is `resolution_blocked`. Its lower-priority copies remain visible under
`shadowed`, but are **never parsed as though selected**. A corrupt/unindexable
higher-priority archive similarly blocks archive-dependent lookups that could
reach it. A valid loose-first hit still wins before any archive. Missing paths,
case-colliding loose names and changed archive indexes remain explicit.

## Discovery and dependencies

Mission names come from virtual `missions/<name>/` entries containing known
Scene2/actor/table/binding filenames. Discovery does not imply those files can
all be decoded or that the directory is a playable mission. The available input
set produced 58 names; that is not a universal game mission count.

An installation session inspects selected recognized mission files. It indexes
all sources under `scripts/<mission>/`, analyzes literal binding/ScriptAssign
dependencies within explicit limits, and adds matching model/sound resource
metadata. Unbound sources remain `indexed` until selected. Includes and dynamic
script assignment remain unsupported; dependency expansion is not an execution
trace or complete closure. Conditional/unreached references are still serialized
references, not asserted reachable behavior.

Script paths use the established 00427730 `Scripts/<mission>/<name>` and exact
`.scr` / `.SCR` substring rule. These virtual-path selections can be labelled
`resolver_selected`; that proves the selected file under the chosen profile,
not that its source compiles or executes. A selected unsupported payload remains
unavailable. Model cache keys (extension-stripped) and sound filename matches
remain `name_candidate`, even when the candidate file's own archive provenance
is known. No model joint hierarchy, directory-priority cache search or scene-path
engine query is simulated.

## Bounds, identity and installation protection

Virtual paths use slash-separated, case-normalized safe relative names. Record
IDs are `resource/<virtual-path>@0x<member-relative-offset>`; script PCs append
`#pc=`. IDs do not depend on which archive or loose file supplies the path;
content hashes/source provenance identify the current bytes. Offsets are within
the decompressed member, separate from archive header/data offsets. A changed
layout can naturally produce changed record identities. Loose-folder sessions
retain their earlier tagged relative identities; cross-backend equivalence is
not asserted.

The index caps total entries at 200,000, each recognized payload at 64 MiB,
dependencies at 2,048 files and automatic script analyses at 512. Session models
retain at most 200,000 records and 100,000 script instructions. Limits are
reported, not treated as complete success. Byte buffers are discarded after
parsing; unknown chunks keep span/hash metadata rather than payload copies.
JSON is streamed from shallow dataclass views, avoiding a deep copy/string of
the entire mission. UI tables page 250 rows and the graph draws at most four
nearby edges. No performance claim is made for every mission or hostile input.

All reads use read-only handles and recheck containment/symlinks. Archive
size/mtime changes require reindexing. No hostile concurrent replacement sandbox
or byte-identical live snapshot across an entire installation is claimed.
Mission exports require new paths outside inspected roots; source snapshots also
protect the entire originating installation. Existing files/aliases cannot be
overwritten. No editing or binary serialization is present.
