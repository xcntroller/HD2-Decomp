# H&D2 modding toolkit

Independent modding workstream on `feature/modding-tools`, based on local master
`89d3d2f` (latest locally available at start). No game targets are built or changed.
The compiled script CLI builds with C++17 and the recovered headers and runs
without Python. The GUI and mission backends require Python with Tk for the GUI.
Normal operation needs neither Ghidra nor original executables; installation
inspection reads the user's own local assets.

```
make -C tools/modding
/tmp/hd2-inspect my-script.scr
/tmp/hd2-inspect --json my-script.scr
/tmp/hd2-inspect --catalog > command-reference.json
```

Default output describes accepted instruction constructs, symbols, lexical
command/predicate mentions and references. Exit codes: 0 supported syntax
accepted, 1 malformed/unsupported/unexpanded source, 2 CLI or file-open error.
Input is read-only. Supply your own source; no commercial scripts are included.

## Desktop GUI

From the repository root:

```
make -C tools/modding gui
```

This builds the small inspector sequentially if needed, then launches the local
Tk desktop application. Requires Python 3 with Tkinter and a graphical desktop;
there are no pip dependencies, web servers or network requests. On systems where
Tkinter is packaged separately, install the matching Python Tk package first.
The default native binary lives in `/tmp` and is rebuilt after a reboot. With an
already compiled binary you can also run:

```
python3 tools/modding/gui.py --inspector /path/to/hd2-inspect
```

Use **Open script…** (Ctrl+O) to inspect a local script. The tabs provide:

- **Overview:** syntax status, diagnostic, construct counts, event indicators,
  limitations and lexical command/predicate mentions.
- **Source:** read-only preview of the same bounded byte snapshot used for analysis.
- **References / Symbols / Instructions:** structured tables from the CLI report.
- **Command reference:** searchable command/predicate catalog, available without
  opening a script. Argument kinds are displayed by known type names.
- **JSON:** the complete inspection report; **Export JSON…** saves it to a new file.

**Mission explorer…** opens the integrated read-only mission browser in this application. Choose a local
mission directory containing loose original files, then optionally add reference
folders containing loose scripts/models/sounds. Browse files and expand documented
records; select fields to see offsets, IDs, provenance and references. The
**References** tab includes outgoing and incoming local candidates. Double-click
a candidate to follow it; use **Show all file references** to reset subtree scope.
**View script** opens the selected source in the existing script inspector.
Unknown formats/fields stay explicitly unsupported/opaque. No mission file is edited.

Use **Open installation…** to discover missions directly from the original loose
files and established archive registration profile. Set the language and choose
loose-first or archives-first, then select a discovered mission and **Inspect
mission**. **Refresh** reindexes changed inputs. Winning ISD1/Patch payloads are
explicitly unavailable; base copies are never substituted. Archive provenance
and shadowed copies are shown in file/entity details.

The new **Actors / objects**, **Dependency graph** and **Validation** tabs provide
searchable/paged results. Actor details show supported serialized properties;
unknown fields/types remain labelled. Select a graph row to visualize its local
relationships; double-click nodes or use **Inspect source/target** to navigate.
Solid/dashed relationships distinguish serialized/selected-resource evidence from
candidates. Validation includes unresolved/dynamic references, malformed supported
records, unsupported payloads and partial coverage. This is not a map editor.

Headless mission inspection and deterministic export work independently of Tk:

```
python3 tools/modding/mission.py /path/to/Missions/Tutorial \
  --reference-root /path/to/loose-assets \
  --inspector /tmp/hd2-inspect --output /path/outside-inputs/tutorial.json
```

`--inspector` is optional; without it, scripts are indexed. Reference folders are
indexed and can be inspected on demand in the GUI. Mission JSON exports require
a new file outside all inspected folders. Source snapshots protect their originating installation.
The headless command exits 0 when a report is produced, even if files are malformed/unsupported; inspect per-file
status and diagnostics. Invocation/directory/export errors exit 2.

Installation discovery/headless use (no asset extraction directory):

```
python3 tools/modding/installation.py /path/to/game
python3 tools/modding/installation.py /path/to/game --mission tutorial \
  --inspector /tmp/hd2-inspect --output /path/outside-game/tutorial.json
```

Add `--archives-first` / `--language English` to choose a different explicit
profile. It is not a capture of live global settings. The browser reuses existing
ISD0 codecs; ISD1 supports index/name metadata only. A successful report command
can contain blocked/malformed/unsupported entries; inspect validation and coverage.

Exact coverage/schema **2** contracts: [FORMATS.md](FORMATS.md).
Lookup policy and archive limits: [RESOLUTION.md](RESOLUTION.md).
Latest real-input verification: [INTEGRATED-VALIDATION.md](INTEGRATED-VALIDATION.md).
Earlier script/content validation: [VALIDATION.md](VALIDATION.md).

**Inspect again** reads a fresh snapshot. Analysis runs in a background worker;
old results and export availability clear before a new inspection. Failed syntax
still provides a diagnostic and lexical mentions, with no partial parse products.
Export refuses the selected source path, including symlink/hardlink aliases.

Preview is limited to 65,535 bytes, displayed using a reversible Latin-1 byte
mapping rather than guessed Windows encoding. Line breaks are normalized for
viewing only, and non-whitespace control bytes are displayed as hex escapes.
Original bytes are passed to the CLI unchanged. Files beyond its source limit
remain rejected. The GUI is an inspection frontend, with no script editing,
execution, include expansion or added semantics. JSON exports are reports, not
bytecode or mission files.

Native desktop smoke checks exercised real widgets, catalog filtering, file
loading/reloading, read-only previews, export/source protection and failure
states against the actual CLI. User desktop acceptance was received on
2026-11-09 ("Works"), with authorization to commit and push feature/modding-tools.
The user delegated branch work and will review changes when merging with master.

## Evidence and supported boundary

The tool consumes the verified header-only lexer/parser and tables **read-only**;
it is a separate executable with no VM/context, loader, game application or engine
linkage. It does not introduce a competing parser or change runtime behavior.
See [compiler contract](../../analysis/mission-script-loading.md) and
[receiver contract](../../analysis/mission-script-receivers.md).

- Lexer: 0042CC00/0042CDD0, ASCII names, case-insensitive name lookup, CR-based
  original diagnostic lines, quoted text without invented escape decoding,
  `//` comments and 1023-character token bounds.
- Tables: 00835B68 (275 ordered command records), 008338A8 (246 signatures),
  00832D60 (33 predicates). Reverse command lookup 0042DE80 and forward predicate
  lookup 0042E0E0 are reused. The catalog preserves aliases and raw masks.
- Prepared-source parser: 0042C850/0042C0B0; Integer, String, Frame declarations,
  assignments and established expressions; signature-driven calls, labels,
  GoTo/Gosub, If/Else, braces, Block/Return, ScriptStartPoint, numeric/simple
  events and Whenever/SetWhenever/SetWheneverTime. Recognized table entries
  need not be supported declarations or implemented runtime commands.
- Variables: kind 2 integer, 4 string, 5 frame; compiler symbols also include
  kind 8 Whenever identities. Bool/Enum/Real declarations remain unsupported.
- References: original 00416E10 receivers for opcode 1 ScriptAssign,
  5E FRM_FindFrame and 66 FRM_FindChildFrame consume string operand 1.
  Literal strings are reported as script, scene_frame or child_frame references.
  String-variable operands are marked dynamic with a null literal, even when
  initialized from a constant. No data-flow or path-existence claim is made.
  Child names require a parent frame; they are not file paths.

## Script CLI machine interface (schema_version 1)

`--json FILE` emits one JSON object. Fields include syntax_accepted, diagnostic,
limitations, variables (name/kind/pool_index), start_pc, eight event_slots,
whenever_count, instructions (pc/name/opcode/operands), references
(pc/kind/dynamic/literal), lexical_mentions (name/category), and
include_directive_present. All parse products are omitted/empty on failure;
lexical mentions remain available but are explicitly **not calls**. Names can
also occur as symbols. Internal instructions can have `<internal>` names.
Operands are raw typed-pool/PC indices, not evaluated values or portable VM
bytecode. Event slots follow the recovered program's DC..F8 order; numeric
signal/cutscene event identities remain visible in instruction operands.

`--catalog` emits command records with name, opcode, raw_mask and argument_kinds,
and predicate records with name, opcode and argument_kinds. Kinds stop at the
first zero. A null signature denotes a special parser construct or absent table
signature; an empty array denotes zero ordinary arguments. Special constructs
have dedicated grammar, so their table signature is not a call template.
Consumers must check schema_version and syntax_accepted before using products.
Original source bytes >=128 are JSON-escaped as byte-valued Unicode code points;
this is reversible byte preservation, not a Windows code-page conversion.

## Compatibility and unknowns

Acceptance means the bounded reconstructed front end accepts prepared source.
It does not establish execution, original-game acceptance or mission delivery.
Expression quirks and explicit safety rejections are inherited, including the
known negative-expression fold boundary documented in the compiler contract.
There is no speculative command description, actor applicability, unit inference
or predicate evaluation. Raw command masks are not interpreted as capabilities.

Any raw `#include` occurrence blocks validation, even inside a comment/string:
original 0042C580 searches raw bytes and has shared-cursor behavior. This tool
flags presence but does not claim a dependency filename or expand includes.
Expansion requires the actual Files resolver with resource-specific then shared
include paths; inventing filesystem/archive precedence would mislead modders.
Use explicitly prepared source if needed, keeping provenance separately.
Input over 0xFFFE bytes and embedded NUL are rejected rather than silently
truncated. The script CLI has no source rewriting, archive extraction, resource existence
check, recursive script traversal or runtime simulation. The separate mission
explorer decodes only the documented subsets in [FORMATS.md](FORMATS.md) and
provides local reference candidates; it does not establish runtime resolution. Sabre Squadron compatibility is not
established. Global variables and events are syntax, not inferred lifecycle rules.

## Extension boundaries

Keep additions under tools/modding; do not change runtime headers to fit tooling.
The parsers/client are independent of Tk; typed models and deterministic mission
JSON support future frontends. Next useful work: source spans, structured script
diagnostics, explicit numeric events, additional verified format subsets, and a
resolver interface whose search policy is supplied by the caller. Add each reference kind only with documented
original receiver/operand evidence. A future editor can consume versioned JSON;
there is no need to expose compiler pool internals as a stable editor model.

Verification uses temporary synthetic fixtures only, removed after audits under
the project agreement. No permanent tests or commercial examples are added.
