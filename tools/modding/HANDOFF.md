# Modding-tools handoff — 2026-11-09

Latest result: read-only Tk GUI implemented and user desktop-accepted ("Works").
User authorized committing/pushing feature/modding-tools and delegated branch
work, with review at merge with master. Keep changes isolated to this workstream;
do not merge into master. Start with `make -C tools/modding gui` from repository
root. [Usage and boundaries](README.md#desktop-gui).

GUI: background CLI analysis of an unchanged byte snapshot; overview, source,
references, symbols, instructions, searchable 308-entry catalog and JSON tabs;
export to separate JSON with source/alias protection. No extra script semantics,
editing, runtime, networking, archive or include resolution was introduced.

Verification: warning-free backend build, Python compile check, actual desktop
widgets exercised with synthetic source against real CLI. Passed open/reload,
275+33 catalog/filtering, reference display, read-only source, JSON round-trip,
source overwrite protection, stale-result/export clearing, missing file/binary,
rejected syntax/includes, source-size/NUL bounds and Tk callback checks. Inspected
Overview/Command reference screenshots; adjusted wrapping/footer and repeated
that focused GUI check successfully. User subsequently confirmed it works.
No new native implementation changed; existing CLI sanitizer evidence applies.
Temporary audit fixtures are not tracked; no commercial source was used.

CLI checkpoint53f6093; original branch base89d3d2f. CLI synthetic/native
ASan/UBSan/LSan evidence and original addresses: [README](README.md). Main
HANDOFF.md, plan.md, function ledger, recovered runtime and networking remain
untouched. All changes under tools/modding. No original executable/DLL/Proton,
merge or game build was performed. No P2.2/function-recovery credit applies.

Next independent implementation candidates in PLAN.md: source spans/structured
diagnostics, numeric event records and evidence-tagged references. Include
resolution remains unsupported until caller policy and faithful preprocessing
are explicit. Do not reinterpret unknown command semantics for GUI convenience.
