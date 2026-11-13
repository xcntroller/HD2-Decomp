# Local research lookup

Use `research_index.py` before starting another Ghidra export or searching a
large research document. It joins existing evidence by original address and
searches short passages by words, symbols or phrases. Python 3.10+ with SQLite
FTS5 is sufficient; no packages, network, Ghidra session or original execution
are needed.

## Main-agent workflow

From a checkout containing this tool and the local research evidence:

```sh
python3 -B tools/research_index.py find 00751480 00755500 00799bb0
python3 -B tools/research_index.py neighbors 00751480 --limit 30
python3 -B tools/research_index.py search 'Car seat' --phrase --kind note
python3 -B tools/research_index.py search 'begin_human_model_exit' --kind source
python3 -B tools/research_index.py search 'seat' --path human-frame-source
python3 -B tools/research_index.py read analysis/human-frame-source.md --line 1300 --count 60
```

Batch the related addresses in one `find`. It returns:

- Frozen inventory membership, name and body size; containing entries when the
  query is an interior instruction address rather than a function entry.
- Current ledger coverage, exclusions and evidence paths. No ledger claim does
  not mean no implementation: untracked/uncommitted source is indexed too.
- All available C content variants, including custom ABI exports without JSON.
  Identical copies collapse; changed bodies remain separate. Modification time
  orders candidates, **not correctness**. The signature excerpt is a locator,
  not a recovered ABI claim.
- Fingerprint-qualified export metadata with signature, convention, stack purge,
  decompilation result and paired C path. Different ABI metadata remains separate
  even when C bytes match. Failed exports never offer a paired C body as success.
- Bounded source, contract, receipt, assembly and export references with line
  numbers. Source comments beginning with the queried address rank before
  incidental references. Notes prefer recently modified files and later sections.

Read the relevant source/contract section and compare differing exports before
deciding what still needs recovery. `neighbors` lists only caller/callee entries
reported in existing Ghidra metadata, with evidence paths, current ledger status,
metadata-export counts and available C-variant counts. It does not infer calls
from prose, C text, shared files, receipts or names. Virtual/indirect dependencies
still require original evidence. Reported edges from different exports are kept
with their provenance, not promoted to a complete or current call graph.

## Use immediately from a separate tooling worktree

The tool can read a different checkout, including its ignored exports and dirty
source. **The default cache belongs to the tool's checkout, never `--root`.**
There is no need to copy assets/exports, create symlinks or change the main branch.
For the isolated implementation worktree, run from the main checkout:

```sh
python3 -B /tmp/hd2-research-index/tools/research_index.py --root "$PWD" find 00751480 00755500
python3 -B /tmp/hd2-research-index/tools/research_index.py --root "$PWD" search 'Car seat' --phrase --kind note
```

That `/tmp` path is session-local. After integration, use the relative commands
above. `--root`, `--db`, `--json` and `--no-refresh` precede the subcommand.
`--program client` is the default; use `--program engine` for LS3DF or
`find --program unknown` to inspect export metadata with other/missing hashes.
Generic text references are always labeled program-unqualified. Neighbor graphs
are restricted to the two fingerprint-qualified programs.

## Index lifecycle and output size

```sh
python3 -B tools/research_index.py index
python3 -B tools/research_index.py status
python3 -B tools/research_index.py --json find FUN_00751480 0x00755500 --limit 3
python3 -B tools/research_index.py --no-refresh search 'mounted equipment' --kind note
python3 -B tools/research_index.py index --force
```

Every normal query incrementally refreshes first. Unchanged size/mtime/ctime
avoids rereading; changed files are hashed, and only changed content is reparsed.
Removed, invalid and excluded files lose their old hits. Refreshes are SQLite
transactions, and writers serialize. A file observed changing during its read
is omitted with a diagnostic and retried on the next refresh. The source tree
itself is not an atomic snapshot; the tool is a locator, not a validation receipt.

`--no-refresh` opens an existing cache read-only and explicitly marks the result
as potentially stale. `read` reads the live file and reports any digest difference
from the cache. `index --force` rehashes/reparses everything when necessary.
Cache root/schema mismatches fail rather than mixing evidence; choose a new
`--db` path for an incompatible cache. The default uses a root-specific filename
under ignored `analysis/work/research-index/` in the tool checkout.

`find --limit` bounds locations per category and C/metadata variants per address;
the total counts remain visible. `search --limit` bounds 70-line passages, with
heading, line interval and a short matched excerpt. Search terms are literal
AND terms; `--phrase` matches their sequence. `--path` is a literal, case-sensitive
path substring. JSON output retains full ledger claims, variant copy paths,
fingerprints, file digests and edge provenance for scripted consumers. Errors
use a nonzero exit code and stderr. No reconstruction counts are changed.

## Coverage and limits

The allowlist includes root instructions/handoffs, `recovered/`, `analysis/`,
`tools/` and `docs/`. In ignored `analysis/work/` and `analysis/ghidra/`, only
`.c`, `.asm`, `.json`, `.md` and `.java` are candidates. Other scoped text types
are `.h`, `.cpp` and `.py`. Downloaded tooling, builds, assets, installations,
captures, binaries, logs and temporary native `.cpp` fixtures are excluded.
Symlink files/directories are not followed. Files over 8 MiB and unreadable,
invalid-JSON or non-UTF-8 text are omitted with diagnostics; queries report the
omitted count and `status` lists paths/reasons. A clean source checkout works
without the ignored evidence, with correspondingly narrower coverage.

Only matching binary SHA-256 values qualify inventory, ledger and export
identities as `client` or `engine`. Text matches and C-only exports do not prove
program identity. Existing exports do not consistently record Ghidra type/analysis
revisions or decompiler options: **this is a reuse-candidate index, not an automatic
safe-export cache**. The tool neither declares an export unnecessary nor launches
Ghidra, changes types, executes binaries, grants function credit or certifies
fidelity/integration/desktop delivery.

Generated SQLite caches contain local research material. Keep them ignored;
do not publish or commit them. Sources and original evidence are only read.

## Implementation checkpoint

Isolated branch: `tooling/research-index`, based on `31b01f9`. The main checkout
was only read; gameplay source, root handoff, plan, ledger and its in-progress
Human work were not changed. The cache already indexes that live checkout and
can be used with the absolute commands above before integrating the branch.

On the inspected local corpus (about 11,600 candidate files), the initial build
took 25.1 seconds, a refresh-and-lookup took about 0.66 seconds, and a cached
two-address lookup took a median 0.068 seconds over five runs. The full cache
uses about 392 MB. These are local CLI measurements, not claimed reductions in
agent research time. Four historical aggregate JSON files over 8 MiB were
excluded explicitly; ordinary function exports and frozen inventories fit.
A source-only checkout without ignored exports also indexed successfully.

Verification and measured results for the initial implementation are recorded
in [the tooling checkpoint](../analysis/checkpoints/research-index-20261111.json).
This tool does not add permanent tests or change gameplay behavior.
