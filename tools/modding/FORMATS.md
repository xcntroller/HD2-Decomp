# Read-only mission explorer contracts

The explorer inspects serialized data. It neither loads a mission nor validates
that a game, engine or mod can run it. All parsers and typed models live in
`mission.py`, without Tk imports; `script_client.py` isolates the CLI protocol.
`mission_gui.py` renders models. No recovered runtime files were changed.

## Supported formats

| Input | Decoded subset | Original evidence |
| --- | --- | --- |
| scene2.bin | First 4C53 root; version header; 4000/4010 objects; creation kind/subtype; names, position, WXYZ rotation, fallback position, scale; parent/model/sound references; embedded AE20 actors and AE40 tables | 006CC760/006CD940, `recovered/game/mission_scene_reader.h`; existing `tools/inspect_scene2.py` |
| actors.bin / sounds.bin | Same documented document framing; object fields and AE20/AE21 actor entries, AE22 type, AE23 frame name; AE24 property payload remains opaque | Same reader; no assumed independent scene version when header is absent |
| Scripts.dta / mpscripts.dta **inside a mission** | First root's ordered pairs of bounded frame and script strings; chunk IDs retained without requiring specific values | 00428600, `recovered/game/mission_script_bindings.h`, `analysis/loading-modules.md` |
| check2.bin | Two version 7ED00109 blocks; headers/counts; 32-byte records with variable link pairs; 18-byte records; frame/actor name references | 00523C00/0051DE80/0051C7B0/0051C870/0051C910, `recovered/game/mission_table_reader.h`; containing `recovered/host/game_mission_scene.h` |
| .scr | Existing inspector protocol; supported syntax, instructions, symbols, limited literal/dynamic references | `analysis/mission-script-loading.md`, `analysis/mission-script-receivers.md` |
| Other files | Inventory, byte size, explicit unsupported status; no payload interpretation | No new semantic claim |

Scene documents expose only the **first** root, retaining trailing bytes as an
opaque span because the loader ignores them. Unknown chunks are spans with IDs,
offsets, sizes and SHA256; they are not recursively guessed to be containers.
Unconsumed named-field suffix sizes are retained. Transforms are serialized local
values, never claimed as resolved world geometry. Nonfinite decoded transforms
are an explicit inspection rejection; this is not an original-game verdict.

Actor property blobs, lightmaps, light configuration, geometry, effects, tracks,
collision trees, historical check.bin, archives, meshes, textures and audio are
not decoded by this milestone. B810 is not treated as a script-binding list.
A top-level commercial Scripts.dta archive is not a mission binding file.

Table records retain uninterpreted prefix/suffix bytes, serialized flags, ordered
WORD/WORD link pairs and the native sentinel flag. Synthesized sentinel storage
is identified, not invented as an extra serialized link. The 18-byte record's
four DWORDs remain raw words; the tool assigns no coordinate or gameplay meaning.
Table offsets are file-relative, including when embedded in AE40. Unsupported
versions reject; no conversion to version 9 is attempted.

Bindings retain **all serialized pairs**, duplicates and order. The original's
live driver frame filter and mode-dependent choice of binding file are not run.
Both files are inspectable; listing one does not mean it is active. Names follow
the native bounded 999-byte NUL search. Trailing data outside the first root is
retained as an opaque span.

## Reference graph

References carry their source record/PC, kind, verbatim byte-mapped name,
evidence and zero or more tagged local targets. Both outgoing and incoming
references are browsable; record selection scopes links to its subtree. Targets
are **candidates**, never a reimplementation of runtime lookup.

- Object, actor, parent, script frame and table references match exact serialized
  object names, preserving ambiguous duplicates. This does not prove that the
  engine creates that frame, that an actor is bound to it, or that a frame-path
  query resolves. Imported model joints/hierarchies are not indexed.
- Script bindings and opcode 1 ScriptAssign match direct local script paths or
  `Scripts/<mission>/<name>` / `<mission>/<name>` in supplied folders, ignoring
  filename case for candidate lookup. The original exact `.scr`/`.SCR` substring
  extension rule is retained. There is no archive search or shared include search.
- Opcode 5E FRM_FindFrame yields scene-frame candidates. Opcode 66
  FRM_FindChildFrame remains dependent on its runtime parent; it is not globally
  matched. String-variable references remain dynamic without data-flow guesses.
- Model and sound references match explicit local path suffixes. Model `.4ds`
  filename candidates additionally use the established extension-stripped key.
  This is local filename evidence, not engine cache selection or asset decoding.
- Empty/unsafe path-like names, unresolved names and ambiguous matches remain
  explicit. Unresolved means not found in supplied folders, not absent in the game.

Optional reference folders are indexed, with sources analyzed only when selected.
Mission-local recognized files are parsed on scan. Resource files remain opaque.
The graph updates when a referenced source is inspected. JSON exports describe
the current inspected state, including indexed/unsupported/unresolved records;
they are not a complete transitive dependency closure.

## Models, safety and export

Dataclasses: MissionReport, FileReport, Record and Reference. Identity is a tagged
relative file path plus file offset (or script PC). JSON schema_version 1 has
sorted object keys, stable file ordering, ordered records and deterministic
candidate lists. It excludes machine-specific absolute root paths in ordinary
successful reports. Each read binary/source has a SHA256 fingerprint; inventory-
only entries have null hashes. Error diagnostics may contain OS-specific paths.
There are no timestamps. This is a report schema, not a binary editing schema.

Byte names use Latin-1 code points with JSON escapes for reversible preservation;
no original Windows code page is asserted. Unknown record bytes are not dumped,
but their source span and hash allow independent identification. Parsed data and
local reports may contain proprietary names: they are for the user's local use,
not fixtures to commit or redistribute.

Reads are bounded to 64 MiB per recognized file, 200,000 records per file and
20,000 inventory files per session. Script analysis retains its 0xFFFE-byte
acceptance limit. Directory symlinks/file symlinks are skipped, and selected-file
reads recheck containment/links. Truncation, bad chunk sizes, unterminated strings,
invalid table counts/capacities, invalid versions and nonfinite transforms fail
without publishing partial records or references. Unknown bounded payloads are
supported as opaque; malformed known structures are not silently skipped.
File-budget exhaustion declares the inventory incomplete. No filesystem sandbox
against hostile concurrent file replacement is claimed.

All inspection reads originals without writes. Mission exports require a **new**
file outside every supplied folder, protecting existing files and aliases. Script
exports now also require a new file. No save/edit/patch operation is available.
