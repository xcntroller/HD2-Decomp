# Mission Explorer format/model contracts

The explorer inspects serialized data; it does not instantiate a world or prove
runtime compatibility. Parsers/models remain independent of Tk:
`mission.py`, `actor_fields.py`, `resource_store.py`, `installation.py`, and
`dependency_graph.py`. The existing GUI integrates these modules through
`mission_gui.py` / `explorer_panels.py`. No client/server source was changed.

Installation discovery, precedence, ISD0 codecs and **verified ISD1 raw/LZSS/checksum** limits
are documented in [RESOLUTION.md](RESOLUTION.md). In particular, a winning
unsupported Patch member blocks lower-priority base data; it is not substituted.

## Serialized coverage

| Input | Decoded subset | Original evidence |
| --- | --- | --- |
| scene2.bin | First 4C53 root, version; 4000/4010 objects; kind/subtype, name, position, WXYZ rotation, fallback position, scale; parent/model/sound names; AE20 actors, AE40 tables | 006CC760/006CD940, `recovered/game/mission_scene_reader.h`; existing `tools/inspect_scene2.py` |
| actors.bin / sounds.bin | Same framing/fields; AE20/AE21 actor entries, AE22 type, AE23 frame; supported AE24 property families below | Same reader; no scene version invented for a headerless document |
| mission Scripts.dta / mpscripts.dta | Ordered pairs of bounded frame/script strings in first root; IDs/order/duplicates retained | 00428600, `recovered/game/mission_script_bindings.h`, `analysis/loading-modules.md` |
| check2.bin / AE40 | Two version 7ED00109 blocks, counts, 32-byte records and WORD link pairs, 18-byte records, frame/actor names | 00523C00/0051DE80/0051C7B0/0051C870/0051C910, `recovered/game/mission_table_reader.h` |
| .scr | Compiler-backed schema2 spans, diagnostics, events, symbols, instructions, includes and limited references | `analysis/mission-script-loading.md` / `mission-script-receivers.md` |
| Other files | Inventory/size/provenance and explicit unsupported status | No new payload interpretation |

Unknown chunks preserve file offset, size, kind and SHA256; their payloads are
not recursively guessed to be containers. The loader's ignored trailing spans
remain opaque. Unconsumed bytes are noted. Table prefixes/suffixes/raw words stay
uninterpreted; the native sentinel flag is identified without manufacturing an
extra serialized link. A top-level commercial archive is not confused with a
mission-local binding document; B810 is not treated as script bindings.

Lightmaps, lights/geometry, tracks/effects, collision trees, historical check.bin,
meshes, textures/audio and broader historical formats remain opaque here, even
where separate reconstruction research exists. Scene transforms are serialized
local fields, not evaluated world transforms. Nonfinite supported transforms
reject as an inspection safeguard, not an original-game verdict.

## Actor/object inspection

Entities group serialized object, actor and binding records with stable source
identity, names, offsets and recognized type labels. Multiple serialized type
declarations are retained/warned about rather than collapsed into a guessed
single runtime type. Empty/unspecified creation kinds do not acquire fake types.

The verified actor property reader contracts are reused as **serialized field
subsets**, with every field's offset/size and evidence address. Thirteen families:

| Type / family | Property contract |
| --- | --- |
| 3 Human (versions 11–15) | 0059E800 |
| 6 Door | 00601AC0 |
| 8 Ladder (versions 1–3) | 005EBBD0 |
| 9 SupplyBox | 005FC200 |
| 10 SpawnPoint (chunk version 3) | 006119B0 |
| 13 Switch | 00613E10 |
| 21 StaticWeapon, 33 NullActor (original no-op readers) | 00604CE0 / 005E2BC0 |
| 22 ExplosivePlace | 0060CA30 |
| 25 Animation | 005B5AF0 |
| 28 Pike | 005B9C80 |
| 29 FowlGenerator (chunk version 1) | 005BB7E0 |
| 30 Dog | 005D6630 |

Source: `recovered/game/actor_properties.h`, with the family/type map corroborated
by `analysis/actor-loading-ownership.md` and recovered constructors. Type 4 Car
is recognized but its property decoder remains unsupported. Unknown numeric
families remain unknown; no successful placeholder is supplied.

Neutral field names (`field19c`, raw word arrays, etc.) are intentional. Runtime
constructor defaults, sentinel normalization, copied state, clamping, callbacks,
portal/animation changes and gameplay semantics are not fabricated. Strings with
unproven consumers are stored fields, not invented resource dependencies. Empty
AE24 payloads are explicitly marked skipped by original dispatch. Supported
malformed properties publish no partial interpreted fields but retain their
opaque span/hash and diagnostic; other records remain inspectable. Unknown
bounded property chunks are retained as partial coverage, not original acceptance.

## Relationships and validation

Node IDs use file path plus serialized offset, or script PC; they do not depend
on a UI row or graph layout. File hashes/provenance identify the input version.
An offset identity is not a claim of persistent gameplay identity after editing.
Loose-folder IDs retain their `mission/` / `referenceN/` tags; installation IDs
use `resource/<virtual-path>` independently of selected archive origin.

The graph deduplicates logical relationships; file Reference records preserve
all serialized occurrences and original source offsets/PCs. Validation deduplicates
identical messages at the same source. Edges have deterministic IDs, source/target,
relation, status, evidence and a
separate **certainty**:

- `serialized`: documented containment/PC relationship, not execution/reachability.
- `resolver_selected`: exact script virtual-path selection under the recorded
  profile. Payload/syntax availability remains separate in status/validation.
- `name_candidate`: exact serialized object-name or resource filename/cache-key
  association. It is not a live engine frame, imported joint or cache lookup.
- `serialized_unresolved`: a recorded reference whose target is unknown/dynamic.

Incoming and outgoing edges are searchable by names, IDs, categories, type,
relation, confidence, status and evidence. The interactive ego view uses solid
edges for serialized/selected-resource relationships and dashed candidates or
unresolved edges. It is a relationship diagram, not a map or world preview.

FRM_FindFrame / ScriptAssign / FRM_FindChildFrame retain established operand
positions. Child lookup requires its runtime parent; dynamic strings have no
invented target. Frame identifiers match verbatim and are not filesystem paths.
Ambiguity/empty names, unresolved references, blocked resources, malformed known
payloads, opaque chunks/files and unsupported types/versions remain explicit.
Unresolved does not mean a missing original asset: an imported model joint or
runtime path may be absent from the serialized subset. Both binding files can
be inspected without guessing which mode activates one.

## Deterministic JSON and safety

Mission schema **2** extends the earlier file/record/reference model with typed
entities, graph nodes/edges, structured validation issues, coverage and optional
resolution profile. Each file can carry selected/shadowed source provenance and `archive_decoding`
integrity/availability separately from content-parse status.
The script CLI protocol is now version2; [SCRIPT-ANALYSIS.md](SCRIPT-ANALYSIS.md) describes spans, events, includes and provenance. Consumers must check the schema and
per-file/coverage status. `coverage.complete_format_support` is always false.

Sorted keys/files/IDs and ordered serialized records produce deterministic
exports for the same indexed state/profile. There are no timestamps or ordinary
absolute root paths; OS error text may be host-specific. Reference-folder sources
are analyzed on selection; installation binding/assignment dependencies are
bounded and automatic, while unbound sources remain indexed. Inspection state
can therefore deliberately change export contents. JSON is streamed with shallow
dataclass views rather than deep-copying a large mission.

Names use a reversible Latin-1 byte mapping; no Windows code page is inferred.
Unknown payloads retain span/hash metadata, not complete byte dumps. Local reports
may contain proprietary identifiers and must not be committed as fixtures.

Bounds: 64 MiB per recognized file, 200,000 records per file/session, 100,000
retained script constructs (tokens/statements/arguments/instructions/diagnostics), 20,000 loose-session inventory files, 200,000 installation
index entries, 2,048 dependencies and 512 automatic script analyses. UI result
pages contain at most 250 rows. Budget exhaustion is explicitly incomplete.
Bad chunk lengths, missing terminators and table count/capacity errors reject
transactionally. Symlinks and containment are checked; this is not a hostile
concurrent-filesystem sandbox. Original files are read-only. Exports create new
files outside protected inputs/installation; overwrites/aliases are refused.
No binary editing, serialization back to game formats or original-code execution
is available.

Script source statement nodes use `#statement=` and compiler instruction nodes
use `#pc=`. `contains_syntax` / `compiles_to` relationships describe observation
and compilation, not execution. Include edges use the caller's installation
resolver; reference locations retain source byte spans. See [script contract](SCRIPT-ANALYSIS.md).
