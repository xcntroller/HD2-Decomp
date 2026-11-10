# Runtime diagnostics (offline, Python 3.10+, standard library)

From the repository root, one command analyzes a real reconstructed-game log:

```sh
python3 tools/runtime_diagnostics/cli.py analyze /path/to/host-runtime.log --out /tmp/hd2-attempt-report
```

Read `report.md`; `report.json` contains the same evidence with source SHA256,
1-based log lines, event counts, last confirmed stage, last confirmed mission
stage, errors and uncertainties. Output directories must be new. No Windows
code, DLL, Proton or launcher is executed by this toolkit. No game code changes.

## User-operated capture

Run the **existing** Python launcher in a desktop terminal in your implementation
checkout, using its existing arguments and workspace. Immediately save its exit
code and capture the direct host log before another run overwrites it:

```sh
python3 tools/run_application_launch.py --work-dir /path/to/isolated-workspace --windowed
run_exit=$?
python3 /path/to/tooling-worktree/tools/runtime_diagnostics/cli.py capture \
  /path/to/isolated-workspace/run/host-runtime.log \
  --exit-code "$run_exit" --scenario 'Tutorial attempt; wait at least 4 seconds' \
  --metadata /path/to/run-metadata.json --out /path/to/private-captures/attempt-01
```

The launcher prints the actual `Runtime log:` location; use that location if it
differs. `--metadata` and `--scenario` are optional. Capture archives the direct
log and reports with private file modes (directory 0700, files 0600). Do not run
capture while the game is still writing. Capture is post-run and is not an
atomic snapshot of a changing log. Complete local logs remain private by default.

Use `--wine-output /path/to/explicitly-saved-output.log` to archive explicit
Wine/Proton diagnostics. The tool records availability; it does not classify
Wine output or enable Wine debug settings. The existing launcher merges relayed
Proton output and appends the direct host log to `host-output.log`; prefer
`host-runtime.log` for analysis to avoid duplicated events and unrelated output.
The recorded exit code belongs to the launcher process; staging failures also
produce nonzero codes. It does not independently prove the Windows process ran.

Metadata example (all fields optional; see `test-run.schema.json`):

```json
{
  "schema_version": 1,
  "scenario": "Tutorial first mission attempt",
  "mission_expected": true,
  "launch_arguments": ["--windowed", "--width", "800", "--height", "600"],
  "conditions": "User desktop, windowed, waited 6 seconds after loading",
  "outcome": "User reports black window; not accepted",
  "screenshots": ["private-screenshots/attempt-01.png"]
}
```

`build_commit` accepts a full 40-digit commit SHA only when provenance of the
**executed build** is known; `build_sha256` accepts its 64-digit executable hash.
Neither repository HEAD nor a current local binary is assumed to identify the
executed build. Hash the executed/staged file yourself when reliably available.
No environment variables, home paths, hostnames or system inventory are collected.
Screenshots are references only and are never opened or copied. Outcome is a
user report, not an acceptance decision.

## Comparison and export

```sh
python3 tools/runtime_diagnostics/cli.py compare /private/old/report.json /private/new/report.json --out /tmp/hd2-comparison
python3 tools/runtime_diagnostics/cli.py analyze /private/attempt/host-runtime.log --sanitized --out /tmp/hd2-export
```

Comparisons show newly/no-longer observed stages, status changes, error counts,
event counts and metadata differences (including cleanup/shutdown). Raw event
text is not compared. Address-like tokens and paths in private error signatures
are normalized; numeric error codes remain. Timestamps in unknown text do not
create semantic events. Different reporting conditions can invalidate comparisons.
Do not interpret equal reports as behavioral equivalence.

Sanitized exports use an allowlist: omit all log text, paths, free-text metadata,
scenario, arguments, outcomes and screenshot references. Keep numeric controller
fields, event IDs, line references, counts, source hash and explicit build IDs.
Errors become marker categories: compare two sanitized runs for a public
comparison, with reduced error detail. Sanitized capture copies no raw logs or
Wine output. Review exported identifiers before sharing. Nothing is published.
Private Markdown also contains local evidence and must not be posted as an export.

## Interpretation and limits

- `observed_success`: an explicit marker confirms only the named operation.
- `observed_failure`: a known failure marker or recorded failure convention.
- `not_observed`: a marker exists but was not seen in the supplied log.
- `not_instrumented`: this baseline has no reliable positive marker for the stage.
- `not_applicable`: explicit capture-only mode, or user reports a nonmission test
  without contrary mission evidence. Unspecified scenario remains unknown.
- `indeterminate`: conflicting results, unclassified warning or insufficient detail.

A construction marker reports an owner/scene, **loaded=0**, not a loaded World.
The kind7 preparation request is distinct from type1 game-state observation.
The controller only prints type/sequence changes, after frame advancement: a
failed transient game state may never appear as type1. Status/phase fields are
snapshots and are not a complete transition history. Missing preparation before
construction is a missing-order observation, not proof of a game bug. Repeat
mission attempts and menu navigation are legal. Shutdown/restart anomalies are
flagged only for constrained markers. Multiple launches are numbered by startup
headers, but aggregate conflicting stage outcomes remain indeterminate. Supply
one direct host log per run for the clearest result.

No success follows from absence of errors. Application frame counts may count
movies and menus. Compiled render callbacks do not prove presentation. Resource
capture does not prove loading. Source/native fixtures do not prove desktop
execution. A black screenshot may reflect the original 3–4 second fade; render
can precede update/fixed. This tool never grants P2.2 acceptance.

Input streams are read in bounded 64 KiB chunks; oversized lines are skipped and
counted. Up to 2,000 events/issues/distinct error signatures and 32 evidence
lines per stage are retained; event text/signatures are limited to 2,048 characters.
Counts continue across the whole file. UTF-8 replacement is recorded. Unknown
lines are counted, with warning-like lines retained as indeterminate rather than
invented success/failure. Future log markers require parser updates. JSON metadata
is limited to 64 KiB; comparison inputs to 16 MiB each. Exit 0 means analysis
completed, even if the game failed; exit 2 means invalid input/tool failure.

Schemas: `test-run.schema.json`, `diagnostic.schema.json`, `comparison.schema.json`
(JSON Schema 2020-12). Output is deterministic for identical input bytes,
metadata and source path; timestamps are not generated. Renaming the input changes
private source location. Use sanitized mode to compare location-independent output.
See [model and instrumentation](INSTRUMENTATION.md), [verification](VERIFICATION.md)
and the independent [handoff](HANDOFF.md).
