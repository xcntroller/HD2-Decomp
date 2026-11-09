# Modding-tools handoff — 2026-11-09

Latest: integrated read-only Mission Explorer is independently verified for a
local checkpoint. User's branch delegation permits local commits for merge
review; **do not push or merge without renewed approval**. All work is under
tools/modding. Main/client/server handoffs, source, plan and ledger are untouched.

Launch `make -C tools/modding gui` → **Mission explorer…** → **Open installation…**.
Choose explicit language/loose-first/archive-first profile, select a discovered
mission and inspect. Existing loose-folder mode remains. New actor/object,
searchable dependency graph (interactive ego view) and structured validation tabs
share the current application. Files/entities expose offsets, IDs, property
subsets, confidence, selected/shadowed provenance and deterministic schema2 JSON.

Contracts and boundaries: [FORMATS](FORMATS.md), [resolution](RESOLUTION.md),
[latest verification](INTEGRATED-VALIDATION.md), [usage](README.md).
ISD0 reuses existing codecs. ISD1 metadata/name indexing is supported, but CRCs/
payloads are NOT: winning Patch members block fallback. Do not claim full ALPS1,
expansion, engine cache/joint, include or runtime compatibility. No editing.

Actual dataset:58 discovered names; Tutorial275 files/2139 entities/5315 edges,
ALPS1174/178/1464 with10 selected payloads blocked. Tutorial Scene2 selects base
Missions; ALPS1 Scene2/actors select Patch and remain unavailable. Original base
ALPS1 was separately checked, not substituted. Thirteen actor property contracts
retain written serialized fields;205 original payload consumption boundaries
agree with the actual recovered C++ readers, native ASan/UBSan/LSan clean. Car
properties remain unsupported. Actor callbacks/defaults/runtime effects excluded.

Focused original/synthetic, headless JSON and actual native GUI checks pass;
rendered graph/validation windows inspected. No new user desktop acceptance is
claimed. No original executable/DLL/Proton or Ghidra execution, no heavy game
build, no permanent tests or commercial assets committed. /tmp/hd2-integrated-audit
held temporary fixtures/slices/reports/screenshots, removed after verification;
audit hashes are in INTEGRATED-VALIDATION.md. Source-only tooling, no P2.2 or
frozen-function credit.

Next: verified ISD1 CRC/payload decoding (before claiming patched coverage),
structured script spans/events and faithful include resolution. Preserve scoped
lookup evidence, explicit unknowns, bounded memory and read-only installation.
