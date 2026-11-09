# H&D2 script inspector

Independent modding workstream on `feature/modding-tools`, based on local master
`89d3d2f` (latest locally available at start). No game targets are built or changed.
Requires a C++17 compiler and this repository's recovered headers; normal use
needs neither Ghidra, original executables, commercial assets nor Python.

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

## Machine interface (schema_version 1)

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
truncated. No source rewriting, archive extraction, mission-content decoder,
resource existence check, recursive script traversal or runtime simulation exists.
Only frame-name references are relevant to scene content in this milestone;
no new content-format research was needed. Sabre Squadron compatibility is not
established. Global variables and events are syntax, not inferred lifecycle rules.

## Extension boundaries

Keep additions under tools/modding; do not change runtime headers to fit tooling.
Next useful work: source spans and structured diagnostics, explicit numeric event
records, richer verified reference schemas, and a resolver interface whose search
policy is supplied by the caller. Add each reference kind only with documented
original receiver/operand evidence. A future editor can consume versioned JSON;
there is no need to expose compiler pool internals as a stable editor model.

Verification uses temporary synthetic fixtures only, removed after audits under
the project agreement. No permanent tests or commercial examples are added.
