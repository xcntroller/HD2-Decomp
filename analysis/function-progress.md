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

The current totals are34 complete client functions and2,754 partial entries
(2,749 client and five engine), through the [Car contact request checkpoint](checkpoints/p2.2-human-car-contact-request-20261112.json). The [Human damage/scoring checkpoint](checkpoints/p2.2-human-damage-presentation-20261111.json)
adds32 frozen client partials and extends four existing partials, preserving the
preceding GUARD additions. The mounted SOUND checkpoint adds three client
partials and extends two existing entries. The [Player objective/session checkpoint](checkpoints/p2.2-human-death-objective-session-20261111.json)
adds nine client partials and extends three. The [peer/console source checkpoint](checkpoints/p2.2-human-peer-console-20261111.json)
adds ten client partials and extends four. The [reaction/authority retirement checkpoint](checkpoints/p2.2-human-reaction-authority-retirement-20261111.json)
adds one client partial and extends four. No complete count or denominator changes. The full
reactions/damage/ragdoll checkbox, P22-HUM and application/desktop acceptance
remain open. The [ledger](function-status.json) and generated README are authoritative.

The [P22-LIFE source checkpoint](p22-life-implementation.md) extends five existing
partials with concrete prefix rollback/finish/file retirement and partial-VM
lifetime guards. It adds zero entries or complete promotions; totals and frozen
denominator are unchanged. Application and desktop acceptance remain pending.

An earlier checkpoint had34 complete client functions and1,833 partial entries
(1,828 client/five engine), including [Car activation](car-activation-loading.md)
and [Human/world composition](human-world-composition.md), followed by22
[initial Human/VM/view control subsets](mission-script-initial-controls.md). The [P2.2 source reconciliation](function-ledger-p2.2-reconciliation.md)
adds881 implemented client subsets and extends six existing partial rows, with
zero complete promotions or denominator changes. The [weapon swap checkpoint](human-equipment-loading.md#same-human-weapon-exchange--2026-11-09) adds eight further partials; [active reselection](human-state-loading.md#active-weapon-reselection--2026-11-09) adds four, and [formation loading](human-state-loading.md#formation-loading-owner--2026-11-09) adds seventeen. [Detached-camera references](human-state-loading.md#detached-camera-human-references--2026-11-09) add six. The later camera/Players checkpoint adds another fifteen,
[activation/prepared spawning](session-activation-loading.md) adds39 and
[local dispatch/first Human camera](camera-local-dispatch-loading.md) adds28. Source-only recovery and live
world/desktop delivery remain separate. The [first completion audit](function-completion-audit.md)
reviews all564 preexisting entries, corrects stale integration notes and closes
five original-ABI button queries. Its0 exact instruction matches are recorded
separately from full-body reconstruction. The subsequent
[delta/scroll group](delta-scroll-completion.md) closes four more entries,
including two exact instruction matches. The
[scalar/input batch](scalar-input-batch.md) reviews41 candidates and closes11
more bodies with one consolidated sweep, including one further exact match.
The [input-owner batch](input-owner-batch.md) then closes14 of17 candidates through
source-only raw ABI entries, separately from the accepted desktop host adapters;
three specific producer/SEH/startup tasks remain. No new exact match is claimed.
The [first Phase2 owner batch](game-owner-start.md) adds12 native source partials
with explicit backend/integration exclusions and no new complete-function credit.
The [owner integration batch](game-owner-integration.md) supplies concrete unloaded
backends and controller receiving/failure paths, extends10 partials and adds17
more. Desktop acceptance and real level loading remain open; no new whole-body credit.
The standalone [completion queue](function-completion-queue.md) is postponed by
the user2026-11-03. Required function completion now accompanies each
[Phase2 delivery milestone](phase2-delivery-plan.md), with no queue-exhaustion gate.
The [ledger](function-status.json) records each supported subset and
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

## Multiplayer source checkpoint — 2026-11-01

[Function audit](multiplayer-function-audit.md) adds33 partial frozen client entries
and extends existing006C20F0. Current total559 partial (554client/five engine),
zero complete /18,783 frozen. Native/strict-build verification is not desktop or
live-network acceptance. No new credit for absent callbacks, engine delegates,
unimplemented Connect/Host, auxiliary services or the Phase2 game receiver.

## P22-UI presentation checkpoint — 2026-11-11

[Source implementation](p22-ui-implementation.md) adds four verified frozen
client partials (004AF980,004AFDB0,004B01E0,00491E30), with no complete promotions
or denominator changes. Totals:34 complete,2,371 partial (2,366 client/five engine).
P22-UI remains open; full HUD/FPV processing and command geometry are still required.

The HUD animation continuation adds four further client partials:004ABE60,
004AD350,004AD380,004AD440. Totals:34 complete,2,375 partial (2,370 client/five engine).

FPV completion adds00491C60 as one further client partial:34 complete,2,376 partial
(2,371 client/five engine). Full P22-UI and desktop acceptance remain open.

FPV frame/status sweep adds00491660,00491E90 and004AAD30 as three client partials.
Totals:34 complete,2,379 partial (2,374 client/five engine). P22-UI remains open.

Crosshair continuation adds004AA840 and005A8100 as two client partials:34 complete,
2,381 partial (2,376 client/five engine). P22-UI remains open.

Hands/icon/selector continuation adds six client partials and extends two existing
rows:34 complete,2,387 partial (2,382 client/five engine). P22-UI remains open.

Command input/modal continuation adds31 frozen client partials:34 complete,
2,596 partial (2,591 client/five engine). The [receipt](checkpoints/p22-ui-command-controls-20261111.json)
records source-only verification. General HUD controls, actual Human producers
and P22-UI closure remain open.

General HUD control continuation adds15 frozen client partials:34 complete,
2,611 partial (2,606 client/five engine). [Receipt](checkpoints/p22-ui-general-controls-20261111.json).
Conditional presentation, actual Human producers and P22-UI closure remain open.

H4 C1 adds five frozen client partials0060D6F0/00588AA0/005A7770/005A7810/005A9560;
source bindings/track coverage extend existing rows, no promotions or inventory
changes.34 complete/3,016 partial (3,011client/fiveengine). Full H4 remains open.
[Contract](p22-hum-h4-implementation.md).
### Human H3 seat interactions — 2026-11-12

[H3 source receipt](checkpoints/p2.2-human-seat-interaction-20261112.json) adds17 frozen
client partials and extends8 existing entries:34 complete/3025 partial
(3020 client+5 engine), no promotions or denominator changes.
Car/StaticWeapon entry/exit and mounted aim/reset/casing use existing owners;
full occupied Car frame/non7 inputs, Human frame and desktop delivery stay open.
004D99D0/004D9A40/004D9BE0/00604630/006034D0/004C1990/004DA1D0/00604DA0 and
B0 direction setter wrappers are absent from the frozen inventory: no new credit.

### H4 common outputs — source boundary, 2026-11-12

[Source closure](human-h4-source-closure.md) and
[receipt](checkpoints/p22-hum-h4-source-20261112.json) consolidate H4 on existing
actor owners. Completed-HUM keyed integration preserves both branches' credit;
Active AI grenade response added3 partials; vehicle source adds15 and extends3.
Current34 complete/3324 partial (3319client+5engine), unchanged18783 baseline and
no complete promotions. Full checker still fails on inherited unavailable ignored
evidence; exact baseline/schema/address/status/totals/available evidence, changed
rows and README checked separately. H6/H8/absent actor/application/desktop remain
required; no delegated engine/helper bodies or external actor credit added.
