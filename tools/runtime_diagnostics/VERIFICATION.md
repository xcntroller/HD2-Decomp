# Verification — 2026-11-10

Source baseline `ca321e2`. Offline Python tooling only; no game build, Ghidra,
Windows binary, original DLL or Proton executed. No desktop acceptance added.
The temporary Python audit used the installed jsonschema package to validate
2020-12 schemas; the CLI itself uses only the standard library.

## Existing evidence inspected read-only

Files below are relative to the main checkout's private `analysis/work/`.
They were not copied into Git. Hashes identify the bytes actually analyzed;
old accepted logs do not identify or accept the current executable.

| Evidence | Lines | SHA256 | Diagnostic outcome |
| --- | --- | --- | --- |
| `shutdown-acceptance-20261031/host-runtime.log` | 14 | `631cfdae9503fd904d437298c15b45f6fdc250e664fca4e4583249a9b4d9e89f` | Startup prepare0 at line5; cleanup through line12; teardown result1 line13; fatal catch line14. No confirmed mission stage. |
| `menu-cache-acceptance-20261031/run-host-runtime.log` | 228 | `38fd16572fe6fafbce8f682a0c912be0f558823719f0212b3dcd250be2d4bc6d` | Startup prepare1; five controller snapshots; loop completion and teardown0. No world presentation inferred. |
| `startup-runtime/run/host-runtime.log` | 49 | `df312e712895c8908fb97e93f6d46326d49c2cf026de26cb92161c6aa0198303` | 37 input captured/absent records, explicit controller/game-not-started marker, cleanup; mission stages not applicable. |

## Temporary audit results

53 assertions passed across the main temporary audit (48) and focused additional
checks (5), covering:

- Actual log classification and output schema validation for run/comparison JSON.
- Empty/missing files, malformed JSON and metadata types/unknown fields, invalid
  build identity, unsupported metadata version, invalid UTF-8 and oversized lines.
- Truncated failure, owner without request, owner after teardown, duplicate
  teardown, restart without recorded teardown, conflicting startup outcomes.
- Synthetic negative activation/entry failures; never a successful P2.2 transcript.
- Sanitized output removes private path, free metadata, outcome and screenshot
  references; sanitized capture does not copy raw logs.
- Byte-identical deterministic reports; comparison stage/error deltas, identical
  run counts, reordered cleanup with unchanged event counts, malformed reports.
- Private capture preserves raw bytes and directory mode0700; capture/analysis
  errors return2 and valid diagnostics return0 regardless of game failure.
- 150,000 distinct failure lines: exactly2,000 retained events/errors, counts
  continue to150,000,148,000 omitted events reported. Combined validator/audit
  peak RSS36,588 KiB on final run. Input reading never materializes the whole log.

A sanitized real-fatal report was also generated and manually inspected under
`/tmp/hd2-diag-real-fatal-export` (ephemeral, not required to use the toolkit).
Temporary audit scripts/fixtures were removed after verification; no permanent
test infrastructure or acceptance records were added. `git diff --check` passed.

## Remaining limits

No P2.2 mission-success log exists in the evidence used here. Positive World,
loading/module/activation/camera/frame/present markers remain insufficient;
see INSTRUMENTATION.md. User capture is a post-run copy, not an atomic live-log
snapshot. Wine output is archived only when supplied, not semantically parsed.
Unknown/future records require parser review. Event caps make long-run ordering
comparison partial; full raw logs stay private. Comparison normalizes variable
addresses in error text, not arbitrary application semantics.
