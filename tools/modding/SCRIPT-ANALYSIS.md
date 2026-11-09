# Structured script analysis and includes — 2026-11-09

This checkpoint completes the two script checkboxes. It is a read-only scripting
inspection environment, not a text editor, debugger or runtime. Syntax acceptance
means acceptance by the recovered bounded front end, not original-game or full
mission compatibility. No original executables/DLLs are executed.

## Evidence and observation boundary

The existing [compiler contract](../../analysis/mission-script-loading.md),
[receiver contract](../../analysis/mission-script-receivers.md) and recovered
headers remain authoritative. `prepare_trace.py` generates inspection-only
copies of lexer/program/compiler headers in `/tmp/hd2-inspect-trace/game`.
`trace-contract.json` pins their SHA256 values and anchors; a changed contract
fails the build pending review. The overlay adds observations at actual token
reads, statement dispatch, ordinary argument consumption and validation failures.
It does not replace expressions, overload signatures or modify runtime headers.
Building now needs Python; the compiled prepared-source inspector runs alone.

| Evidence | Exposed interpretation |
| --- | --- |
| 0042CC00/0042CDD0/0042CF60 | Exact consumed token byte spans; ASCII names, original strings/comments and token limits; expect mismatches |
| 0042C850/0042C0B0 | Statement-to-instruction intervals, compile outcome and final validation sites |
| 0042ECD0/0042E160 | Integer/String/Frame declarations and compiled assignment pool identities; unsupported declaration branches remain unsupported |
| 0042EC20/0042DD40, 0042D890/0042D920 | Ordinary command argument spans, expected kind and consumed typed-pool index; includes minus plus numeric token |
| 0042D170/00412760 | Existing original expression collection/grouping, including unusual NOT/folding behavior; no replacement grammar |
| 0042F0A0/0042F2E0/0042FD10 | Label declarations and actual patched GoTo/Gosub target PCs |
| 0042E410/0042E730 | ScriptStartPoint and simple event definitions; simple event slots retain the last registration |
| 0042E460/0042E550/0042E640 | Numeric events retain the first duplicate identity mapping, while all markers remain instructions |
| 0042E7C0/0042F530/0042F920 | Whenever records, expression index, entry PC and actual defaults (enabled, period500); existing forward-reference restrictions retained |
| 00416E10 | Only the three established ScriptAssign/FRM_FindFrame/FRM_FindChildFrame receiver references (string operand1) |

An event records whether its entry is registered, its marker statement, actual
entry PC, and a brace interval only if that entry is an opening-brace instruction
with the recovered backpatch. This is not a claim about dispatch, reachability or
lifecycle. Lexical command/predicate mentions remain separate from instructions.
Only ordinary command argument uses are currently linked to variable symbols;
expression/predicate symbol uses, predicate argument spans and expression trees
remain unexposed. Special constructs have dedicated grammar, not ordinary-call
signatures. Unknown commands acquire no invented semantics.

## Protocol and provenance

Native and Python script reports now use schema_version **2**; mission reports
remain schema2 with additive script/reference/graph fields. Check versions and
`syntax_accepted`. Native legacy fields remain, with tokens, statements,
arguments, structured diagnostics, numeric event tables and Whenever records.
The Python client adds events, declarations, labels, controls, symbol_references,
includes, source metadata and source_map. Instructions/reference operands retain
their original raw meaning; no runtime values are evaluated.

Spans are half-open byte ranges. `span_space=prepared_source` addresses the final
expanded compiler input. `locations` gives authoritative original source IDs,
byte ranges and 1-based line/column positions, normalizing CRLF/CR/LF for viewing.
These differ intentionally from the native lexer's zero-based CR-only counters.
Multiple locations are retained if a construct crosses inserted sources.
Preprocessor spans use `span_space=preprocessing_step` and a step number: they
refer to the working buffer at that step, not the final expanded offsets.
Source metadata retains size, SHA256 and selected/shadowed provenance; source
bodies are not embedded as additional snapshots. Prepared bytes have a size/hash.

Diagnostics retain severity, category, message, evidence, position and origin.
Origins distinguish recovered-front-end rejection, original missing-include
behavior, resource failures and tool restrictions. Failure-token spans identify
what the recovered parser observed, not always the earliest causal mistake.
Postparse errors use the relevant statement or EOF; unclosed blocks point at EOF.
Compiler failures expose partial statement observations and declarations with
`compiled=false`, but no compiled instructions, variables, PCs or events.
Preprocessing failures withhold statement/compiled products altogether. Lexical
recognition is still not evidence of execution.

Mission graph edges `contains_syntax` and `compiles_to` link source statements to
compiler products. Existing receiver links keep `resolver_selected` or
`name_candidate` certainty. Include edges carry selected resource identities,
source directive locations and provenance; a shared include still uses its
caller's mission namespace for receiver references. Included fragments are
indexed as dependencies without claiming they compile as standalone programs.

## Faithful include resolution and explicit restrictions

`InstallationIncludes` is supplied by the caller with an existing ResourceStore
and explicit mission resource namespace. Mission Explorer supplies its actual
language/loose/archive-first profile automatically. The standalone GUI's Include
context selector uses the default installation profile; use Mission Explorer for
alternative profiles. Loose-folder sessions without an explicit context reject
includes rather than guessing a search root.

Recovered `mission_script_source.h` 0042C850/0042C580/0042C4B0 establishes:

1. Case-sensitive raw substring search for `#include`, without lexical filtering.
2. Blank its eight bytes and the following byte, then read one token with the
   **saved shared lexer cursor**, without seeking to the directive.
3. For a quoted token, blank the inclusive range from after the directive through
   that token's length. Preserve this exact behavior even with extra spaces.
4. Try `Scripts/<resource>/<name>` then `Scripts/include/<name>` only on absence.
   No implicit extension is appended. Missing includes are blanked and ignored,
   now surfaced as warnings.
5. Insert included bytes at the directive, then search again from that position.
   Do not shift the saved cursor. Nested includes can consequently select an
   unrelated token. This is not conventional C-style preprocessing.

The tool rejects non-whitespace prefixes, malformed/unquoted/empty filenames,
shared-cursor/token aliasing, paths leaving the resource namespace, changed
repeated sources, recursion, depth>32, >128 expansions, source reads>0xFFFE,
expanded buffers>65535 or embedded NUL. Native prefix reads/NUL truncation and
unsafe raw-substring cases are **not** silently reproduced. These are explicit
defensive restrictions, not claims that the original rejects such source.
Original final bare-directive/nonquoted cases are also conservatively rejected.
A conventional nested include may be rejected even when simple nesting works.
The shared cursor is tested with the actual lexer through `--lex-one`.

Selected invalid/unavailable archive resources stop preprocessing. They do not
fall back to shared includes, shadowed base archives or loose alternatives. The
existing ResourceStore retains all precedence and checksum/codec barriers.
Source provenance follows insertion and directive blanking without copying any
commercial assets into the repository.

## UI and bounds

The existing application gains an Analysis tab with debounced search, category
filters and 250-row pages. Double-click diagnostic/event/reference/statement rows
to navigate to original source. References, variables and instructions also
navigate on double-click. The Source tab selects root/included sources, applies
lexer highlighting and diagnostic underlines, and highlights exact byte ranges
through normalized line breaks/control-byte rendering. Reopened included sources
must match the analyzed SHA256. Follow mission reference uses existing Explorer
navigation; candidates remain labelled as candidates in report details.

Analysis/resource reads run on the existing single background workers. Syntax
highlighting/rendering stays on Tk's thread. Legacy result tables show the first
2000 rows; Analysis searches all constructs. JSON preview stops at256KiB, while
export streams the full report. Source files remain strictly read-only, and
exports are new files outside originating sources/installations.

Native guards bound nesting to128 and operator chains to256 between statement
separators, in addition to original token/source limits. These prevent unsafe
recursive grouping/destruction; they are tool limits. Installation models retain
at most100000 combined tokens/statements/arguments/instructions/diagnostics,
200000 binary records,2048 dependencies and512 automatic script analyses.
Exhaustion is incomplete, never success. Expanded source tracking has at most128
insertions. These bounds do not promise constant memory or hostile concurrent
filesystem isolation.

## Verification performed

Temporary native/Python fixtures were used, not committed. No Ghidra, game build,
original DLL/EXE/Proton, push or merge was used.

- Selected Tutorial146 scripts:145 accepted,24369 tokens,4511 instructions,
  317 event definitions; two selected from Patch. ALPS1144:144 accepted,
  48881 tokens,9797 instructions,462 events; eight selected from Patch.
  All accepted compiler products (instructions/operands, symbols, slots, start,
  Whenever counts and receiver references) matched an independently built
  uninstrumented prior inspector. Token byte slices and instruction-to-statement
  intervals were checked. Tutorial `t_ee_weather.scr` still rejects at byte1443
  in the recovered expression parser; this is a known coverage limitation.
- All three original include scripts (AFRICA3 af3a_14/15 and CZECH1
  transformator_1) expand and compile. Their expanded bytes, lookup attempts and
  compiler products match an independent native fixture using the unmodified
  recovered MissionScriptSource/Compiler. Source token origins were verified.
  Expanded SHA256 values respectively:
  `fc71332b917e6ef29e6a20753498392c89bdf0255d9a7faefdaea56dd7a0befb`,
  `92d13e01b9b35c4c72c9727ea4b0e6368117c169cab95641ef57b6cfddc7f542`,
  `17c56f8ece7367654733dc259e91d8f9d80041c52a910ebacb83b68749c93ee6`.
- Fifteen include fixtures cover caller/shared precedence, missing includes,
  adjacent directives, successful aligned nesting, cursor-unsafe nesting,
  recursion, malformed/comment directives, unavailable selection without
  fallback, NUL and expansion limits. Supported bytes and callback order match
  the recovered source contract.
- Sixteen failure fixtures cover duplicate symbols/labels/start, unresolved
  labels, unsupported declarations/expressions, event-block/Else checks, token,
  source, NUL, depth and operator bounds. Positive fixtures verify duplicate event
  registrations, patched labels, Whenever defaults and original expression quirks.
- ASan/UBSan/LeakSanitizer: all290 selected Tutorial/ALPS1 scripts and focused
  negative/positive native fixtures passed. Leak checks ran outside sandbox
  ptrace supervision. All15 include fixtures also passed with the sanitizer-built native lexer.
  A zero-instruction statement regression check excludes comments/labels from
  attribution to subsequent assignments; actual AFRICA3 include graph edges were checked.
- Integrated Tutorial275 files/2139 entities/12777 graph nodes/14217 edges;
  ALPS1246/4452/27348/29377. Retained script units35592/76338; deterministic
  streaming exports approximately57/118MB. Sequential two-session streaming
  verification peaked at252012KiB RSS. Binary format coverage remains partial.
- Actual native Tk tests exercised included-source selection/navigation, event
  filtering, source hash rejection, diagnostic positions/underlines, syntax
  highlighting, stale-table clearing and the integrated Explorer window. Actual
  Patch-selected Tutorial/ALPS1 scripts rendered through the same GUI. This is
  an automated desktop test, not new human desktop acceptance.

Still excluded: execution/debugging, editing, bytecode/binary serialization,
expression/data-flow analysis, full compiler/game compatibility, expansion
profiles, new asset families and full mission parsing. Historical validation
notes describe earlier checkpoints and do not supersede this coverage.
