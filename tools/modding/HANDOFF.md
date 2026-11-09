# Modding-tools handoff — 2026-11-09

Latest checkpoint: both remaining script milestones are complete. Only
`tools/modding` changed; client/server/shared handoffs, plan and ledger untouched.
Earlier branch delegation permits verified local commits for merge review; no
push/merge without renewed approval. No editing/runtime/debugger work added.

`make -C tools/modding gui` launches the existing integrated application. New
Analysis search/filter/pages navigate diagnostics, events, declarations, labels,
controls, arguments and references to exact source bytes. Source has highlighting,
diagnostic underlines, included-file selection and SHA256 checks. Open a script
from Mission Explorer to retain its actual installation profile/include context;
standalone Include context selects the default profile explicitly. Headless:
`python3 tools/modding/analyze_script.py --help`.

Native schema2 uses checked inspection-only lexer/compiler overlays; recovered
runtime headers remain untouched. Python adds provenance, actual event mappings,
control/variable views and faithful shared-cursor includes via ResourceStore.
Selected invalid/unavailable resources never fall back. Defensive restrictions
are distinguished from recovered behavior. Mission graph links statements to
compiler instructions, receiver candidates/selected resources and includes.
Contracts, addresses, coverage, limits and verification:
[SCRIPT-ANALYSIS](SCRIPT-ANALYSIS.md), [README](README.md).

Verified all290 selected Tutorial/ALPS1 sources (10 Patch):289 accepted, prior
compiler products unchanged; Tutorial weather retains its known expression
rejection at byte1443. All three real include scripts now compile; expanded bytes,
lookup order and compiler products match unmodified recovered native fixtures.
Fifteen include and sixteen malformed/safety fixtures plus event/label/Whenever
checks passed. Native ASan/UBSan/LeakSanitizer passed all290 scripts and focused
negative/positive fixtures. Actual native Tk checks cover navigation, included
sources, diagnostics/highlights, search, stale clearing, hash rejection and real
Patch-selected Tutorial/ALPS1 displays; no new human acceptance is claimed.

Integrated Tutorial275 files/2139 entities/12777 nodes/14217 edges;
ALPS1246/4452/27348/29377. Deterministic streaming exports checked; combined script
retention35592/76338 under100000. Sequential streaming audit peak252012KiB RSS.
Binary format support remains partial. Retention now counts tokens, statements,
arguments, instructions and diagnostics. UI pages250, legacy tables2000,
JSON preview256KiB. Exports stream; originals remain strictly read-only.

No original DLL/EXE/Proton/Ghidra or heavy game builds used; no commercial assets,
permanent tests or raw reports committed. Temporary audit inputs are outside Git
and removed after verification. ISD1 raw/LZSS remains the earlier verified milestone
([ISD1](ISD1.md)); sound tags>=8 unavailable. The independent plan has no remaining
unchecked items in the requested scope. Next work requires a separately scoped
objective. Expression/data-flow views, full compiler/runtime compatibility,
expansion profiles, new formats, editing and binary serialization remain excluded.
