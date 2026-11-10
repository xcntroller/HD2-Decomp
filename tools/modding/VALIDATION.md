# Toolkit validation — 2026-11-09

This records agent verification, not original-game execution or new user desktop
acceptance. Original executables, DLLs, Proton and Ghidra were not used. Native
compilation was limited to the existing small standalone inspector. Temporary
harnesses, extracted inputs, screenshots and local reports lived under /tmp,
never in the repository; no permanent tests were added.

## Review of the initial inspector / GUI

The features are implemented rather than successful placeholders: the native
inspector calls the recovered lexer/compiler, exports actual program/symbol
containers, and the GUI consumes that JSON. The catalog contains the actual
275 command and 33 predicate records. Script bytes are passed unchanged to the
compiler; display normalization is independent of analysis. GUI open/reload,
reference tables and JSON export were exercised with actual backend results.

Limits identified during review:

- Acceptance is the bounded recovered prepared-source parser's result, not full
  syntax validation, original-executable equivalence or runtime compatibility.
  Native parser quirks remain, including ignored comparisons at some grammar
  sites; they are not replaced with a new language grammar.
- Bool/Enum/Real declarations remain unsupported; command/predicate table presence
  does not mean runtime semantics or actor applicability are known.
- Includes are detected and validation is unavailable, not expanded. Raw search
  also detects directives within comments/strings, matching the known preprocessing
  hazard. Filenames are not claimed as resolved dependencies.
- If/Else, events and Whenever are real parsed constructs, but event output remains
  low level: eight slots and numeric marker operands, not an editor event model.
- Lexical mentions are not confirmed calls; symbol names can coincide with table
  names. AND/OR/NOT recognized by expression parsing are not catalog mentions.
  Lexical reporting can stop at a lexer bounds error. There are no source spans.
- Only ScriptAssign / FRM_FindFrame / FRM_FindChildFrame receiver references are
  extracted. String-variable references are conservatively dynamic; literals can
  share pools, so no data-flow or evaluation claim is made.
- Source over 0xFFFE bytes, embedded NUL and known unsafe expression forms reject.
  Original diagnostic lines count CR only; LF-only input can report CR line 0.
- Initial script export could overwrite an existing non-source file after a save
  dialog. It now uses exclusive creation. GUI/client parsing is now independent
  of Tk through script_client.py.

## Real source corpus

Read the available **base ISD0 Scripts.dta**, SHA256
`9c7d5b429cce158fa01b08feb4b9c012db6b89dadaecaa1e0ac4af67c803bc81`,
using the repository's existing archive reader in a temporary audit, without
changing originals. Its 3,181 script members were scanned to locate include
inputs. This is not the runtime Patch/loose resolver or Sabre Squadron selection.

Tested **293 original scripts**: 146 Tutorial, 144 ALPS1, plus all three other
members containing raw #include. **289 accepted**, producing **14,289 instructions**;
four explicitly unavailable/rejected. This extends the prior 145/146 Tutorial
compiler census and agrees with its known failure.

The corpus exercised complex actual scripts: catalog mentions in 168 sources
included Whenever, 110 If, 72 OnSignal, 106 OnDeath and five Gosub. These counts
are lexical observations, not executions or reachable-event counts.

- `TUTORIAL/T_EE_Weather.scr`: rejects at byte 1443, the documented negative
  expression fold boundary. The original executable was not tested and is not
  claimed to reject identically.
- `AFRICA3/AF3a_14.scr`, `AFRICA3/AF3a_15.scr`,
  `CZECH1/transformator_1.scr`: include validation explicitly unavailable.

No original source text is reproduced in these notes. The old GUI was also
exercised on an original source, and the new explorer analyzed an original
binding's source on demand and followed its frame reference. File-open success
was not used as the compatibility criterion.

## Mission content

Read the existing original Tutorial/ALPS1 research samples and base mission
binding records, arranging temporary normal mission directories for explorer
checks. These inputs are **base research samples**, not proof of the runtime-
selected overlays. More than one captured version exists; fingerprints from
other runtime captures must not be conflated with these samples.

| Original document | Tutorial | ALPS1 |
| --- | ---: | ---: |
| scene2.bin object records | 1,890 | 4,008 |
| scene2.bin actor entries | 27 | 1 |
| actors.bin object / actor entries | 53 / 53 | 133 / 133 |
| sounds.bin object records | 2 | 30 |
| check2.bin primary record counts, set 0 / set 1 | 863 / 56 | 5,113 / 100 |
| check2.bin 18-byte records, set 0 / set 1 | 160 / 23 | 304 / 35 |
| check2.bin actor-name references | 35 | 6 |
| Scripts.dta pairs | 114 | 148 |

Tutorial's empty mpscripts.dta root also decodes. Across all six scene/object
files, record counts, offsets, sizes, names, model names, creation kinds and
supported transforms were independently compared with `tools/inspect_scene2.py`.
They agree. Table/binding counts agree with the recovered research. This does
not audit unknown payloads, instantiate actors or verify geometry.

Native GUI checks passed for real mission inventory, binding → source → Scene2
field navigation, incoming references, lazy table expansion, actual source analysis,
read-only display and mission JSON export. Single-window handling, pending-close
callbacks, source overwrite/alias protection and stale results were checked.
Rendered explorer layout was inspected. No new user desktop acceptance is claimed.
The earlier branch delegation permits local verified commits for review at merge;
the latest instruction prohibits pushing/merging without approval.

## Malformed/unsupported inputs

Temporary synthetic audits cover every truncated prefix of small representative
scene and two-table documents, undersized/oversized chunk framing, missing NUL,
nonfinite transforms, unsupported versions, impossible counts/link capacity,
odd binding pairs and the 999-byte name limit. Unknown chunks and trailing spans
remain opaque and identifiable. Deterministic JSON, source-root export refusal,
existing-file/hardlink protection, symlink exclusion and changed-link rejection
pass. Synthetic model/script/frame candidates, dynamic strings, ambiguous roots
and unsafe names were also verified. A mission CLI export was parsed independently
as JSON.

New parsing/client/UI code is Python; no recovered native bodies changed, so the
previous inspector ASan/UBSan/LSan evidence is retained rather than claimed as a
new sanitizer run. Python compile checks and focused parser/native-GUI checks pass.
No comprehensive game build, desktop game compatibility or map editor is claimed.

Temporary audit source SHA256 identities (removed after verification):

| Audit | SHA256 |
| --- | --- |
| script corpus | 205316ff04662c311832e7b59d4039588c60643d18038ebecc8d1a12439ba699 |
| real mission census | b55929180d85b03a5dc5529f43fe9d68e5134bd47d721ef4c81ca3d828ca0238 |
| existing decoder comparison | 7e575bc34ded762953a5485e5ead5cac125559aa2a0f7b508a935740486178db |
| synthetic bounds/export | 73e9481ebf8e7ab8e370c0a2b53d62ed8c63a2fe41f692db3dc29d6f8ca73aec |

The GUI audit received focused lifecycle/selection checks after layout review;
its final fingerprint is recorded in the handoff, not an intermediate value here.
