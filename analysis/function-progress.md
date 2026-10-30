# Function progress: scope, rules and refresh

The bar in [README](../README.md) counts original **base Hidden & Dangerous 2**
client functions and original **LS3DF.dll** functions only. Sabre Squadron,
other DLLs, assets, source components and milestones do not enter this count.
The exact two SHA-256 values are guarded in both tools and in the
[status ledger](function-status.json). No original binary is executed or patched.

## What the count means

- The frozen denominator is Ghidra's identified **internal non-thunk function
  entries**, including unidentified, library and runtime code. It is an estimate
  of original functions, not a recovered original source-function total.
  Analysis can miss functions, split/merge them, or include mistaken boundaries.
- External function entries are counted separately. Internal thunks are also
  separate; neither enters the completion denominator. External thunks, if any,
  count only as external, not twice.
- Every internal non-thunk is currently **unclassified** semantically. Names,
  namespaces, name provenance and tags are exported as clues, not proof of
  library/game provenance. `FUN_` names do not imply library code. The tracker
  does not silently exclude static libraries or equate unclassified with game
  code. There is no claimed verified library/game split.
- **Complete** requires the whole original function reconstructed end-to-end,
  with evidence and verification, not just a useful approximation or selected
  branch. This is manually reviewed evidence, not something tests or the census
  alone can prove. The tool checks consistency; it cannot certify equivalence.
- **Partial** means a supported behavioral subset is reconstructed, with
  explicit exclusions or differences. It earns zero complete-function credit.
  Omitted entries have **no completion claim**; they are not necessarily
  unresearched. Located/named/research-map entries are not “fully understood.”
- Body sizes and optional covered instruction intervals are diagnostics, never
  fractional completion credit or an effort estimate. Equal function weights
  do not imply equal complexity. No percentage estimates time or playability.

The current totals are304 partial entries (299 client/five engine), zero
complete. The [ledger](function-status.json) records each supported subset and
its exclusions; the generated [README bar](../README.md) is the current count.
Historical research notes retain their checkpoint totals. Integrated desktop
launch is tracked separately in [plan.md](../plan.md) and does not turn partial
source recovery into complete-function credit. Fragments of the same original
entry still count only once, and delegated engine calls receive no new credit.

## Tracked schemas (version 1)

[function-baseline.json](function-baseline.json) stores scope, denominator rule,
`estimated: true`, and a `programs` object keyed exactly `client` and `engine`.
Each program has `sha256`, `ghidra_version`, `image_base`, `counts`, and
`structure_sha256`. Counts contain
`internal_non_thunk`, `internal_thunk`, `external`, `identified_total`, and
`unclassified_internal_non_thunk`. Structural SHA-256 is over canonical JSON
(ASCII, sorted keys, no insignificant spaces) of rows sorted by entry, keeping
only entry, external/thunk flags, body size/ranges and classification.
Renaming a function or changing a tag alone does not alter the denominator or
this digest. Entries, bodies or classifications changing triggers a failure.

[function-status.json](function-status.json) has exactly:

- `schema_version: 1`
- `programs`: exactly the two guarded program-ID/fingerprint pairs
- `functions`: one row per unique `(program, entry)` having a reconstruction claim

Each row has `program` (`client` or `engine`), `entry` (lowercase eight-digit
hex original entry, **not** an interior fragment address), `status` (`partial`
or `complete`), `coverage` (nonempty plain-English supported behavior),
`limitations` (nonempty for partial, empty string for complete), and `evidence`
(nonempty unique list of existing repository-relative file paths).

Optional `intervals` is a list of inclusive `[start, end]` address-string pairs,
strictly increasing and nonoverlapping, entirely inside this original function's
inventoried body. These describe instruction coverage only, not data references
or out-of-function callees. Omit them unless the boundaries are established.
A complete row additionally requires `end_to_end: true` and nonempty
`verification` text, and any intervals must equal the full body. A partial row
may omit `end_to_end` or set it to `false`. Unknown keys/statuses, duplicates,
wrong fingerprints, missing entries, thunk credit and malformed intervals fail.

To add a new partial: append one row with the six required fields above, using
the **original function entry** from its program inventory. If another subset
of an already listed function is reconstructed, extend its existing coverage,
limitations and evidence; do not append a second row. Run `--render`, then
`--check` and the synthetic tests. No baseline reset is needed for ledger edits.

## Reproduce the full local inventories

Run from the repository root. Ghidra must not have this research project open
elsewhere. Never remove project locks. This uses **only the independent research
project**, never the user's baseline project, sequentially with `-readOnly` and
`-noanalysis`. Adjust the research project location only if it has moved.

```sh
mkdir -p analysis/work/function-progress
ghidra/support/analyzeHeadless \
  /home/control/Documents/Projects/HD2ResearchProject HD2Research \
  -process hd2.unpacked.exe -readOnly -noanalysis -scriptPath "$PWD/tools" \
  -postScript ExportFunctionInventory.java "$PWD/analysis/work/function-progress/client.json" \
  -log "$PWD/analysis/work/function-progress/client.log"
ghidra/support/analyzeHeadless \
  /home/control/Documents/Projects/HD2ResearchProject HD2Research \
  -process LS3DF.dll -readOnly -noanalysis -scriptPath "$PWD/tools" \
  -postScript ExportFunctionInventory.java "$PWD/analysis/work/function-progress/engine.json" \
  -log "$PWD/analysis/work/function-progress/engine.log"
python tools/function_progress.py --check
python -m unittest discover -s tools -p 'test_function_progress.py'
```

`ExportFunctionInventory.java` exports all identified entries from both the
internal and external function iterators, deduplicated by entry. No decompiler
or auto-analysis is run and no functions are created. Local JSON includes
`schema_version`, `program`, `program_name`, guarded `sha256`, `ghidra_version`,
`image_base`, and `functions`. Each function has `entry`, `name`, `namespace`,
`name_source`, `external`, `thunk`, `body_bytes`, `body_ranges` (inclusive),
`classification` (`external`, `internal_thunk`, or `unclassified`), `tags`,
and optionally `thunk_target`. External body intervals are empty. Category clues
and generated names remain only in ignored `analysis/work/`; no pseudocode,
assets or commercial files are tracked. Output is restricted beneath that
workspace-local ignored directory. Inspect the headless output for errors:
Ghidra may exit successfully even if a script failed.

Routine updates:

```sh
python tools/function_progress.py --render  # replace only the marked README block
python tools/function_progress.py --check   # read-only; fails on any stale result
```

Both commands validate the entire local inventories, fingerprints, frozen
structural counts, all status entries and evidence. A clean checkout needs
inventory regeneration first; the compact tracked summary is not a substitute
for a full local inventory when checking membership.

If later analysis discovers more functions, the tracked denominator **does not
silently change**. Preserve the old ignored inventory if still using its frozen
baseline, or deliberately review the new census, then:

```sh
python tools/function_progress.py --freeze-baseline
python tools/function_progress.py --render
python tools/function_progress.py --check
```

Review the baseline/README diff and explain any revised estimate. The explicit
freeze operation validates the ledger first, so a split, removed function or
changed entry cannot silently strand credited progress. A baseline revision may
change the percentage even when no reconstruction work has changed.
