# Modding-tools handoff — 2026-11-09

Latest result: useful read-only mission explorer implemented and verified for a
local commit. Prior user delegated this branch and review at merge; newest
instruction explicitly prohibits pushing/merging without approval. No push or
merge in this work. All changes remain under tools/modding on
feature/modding-tools. Main game/server code, shared handoffs, plan and ledger
are untouched. No original executable, DLL, Proton or Ghidra was run.

Launch `make -C tools/modding gui`, then **Mission explorer…**. Open a loose
mission directory and optionally add loose reference folders. The browser exposes
files, lazy records, fields with source offsets/identities/evidence, incoming and
outgoing references, original script analysis, and deterministic JSON export.
Read-only parsers/client are independent of Tk. Exports cannot overwrite existing
files; mission reports must be outside all inspected roots.

Coverage: first-root Scene2/actor/sound document subsets, object transforms and
names, AE20 actor entries (properties opaque), scene/table/resource references,
script binding pairs and both version9 check2 table sets. Resource files and
unknown bounded chunks stay opaque. Local candidates are not engine resolution;
includes remain unexpanded. See [formats/contracts](FORMATS.md), [actual validation
and limitations](VALIDATION.md), [usage](README.md) and [backlog](PLAN.md).

Verification: 293 original base-archive scripts, 289 accepted/14289 instructions;
three includes explicitly unavailable and the known Weather negative-expression
rejection reproduced. Six original scene/object files agree with the previous
inspector on supported offsets/fields. Tutorial/ALPS1 check2 primary counts
863/56 and5113/100; binding pairs114/148. Native GUI navigated actual binding →
source → Scene2 field and incoming references, lazy tables and export; no new
user desktop acceptance is claimed. Existing initial GUI was previously accepted.
Python compilation, synthetic truncation/count/version/name checks, unknown
retention, deterministic JSON, candidate ambiguities and source/alias/symlink
protections pass. No native bodies changed; no new sanitizer run is claimed.

Temporary audit harnesses, extracted data, screenshots and reports were under
/tmp/hd2-toolkit-audit and removed after verification; nothing commercial is
committed. Final native GUI audit SHA256:
cdbe0724c49daa9d188a5f7a3c0471604d0c12c8f2831848ace3da7c730952e4.
Other audit identities are in VALIDATION.md. No permanent tests were added.

Next independent candidates: source spans/structured script diagnostics, explicit
numeric events and further verified formats. Preserve unknown semantics and
read-only operation; do not start binary editing before this bounded explorer
has been reviewed. No P2.2 delivery or frozen-function credit applies.
