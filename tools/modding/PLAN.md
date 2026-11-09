# Independent modding-tools plan

- [x] Inspect established compiler/lexer/tables and receiver contracts.
- [x] Deliver standalone syntax inspection, catalog and verified references.
- [x] Version JSON output and document unsupported semantics/compatibility.
- [x] Verify synthetic success/failure cases and native sanitizers; commit locally.
- [x] Prepare local read-only GUI and verify real CLI/widget flows.
- [x] Obtain user desktop confirmation for the GUI.
- [x] Authorize GUI commit and branch push; review at merge with master.
- [ ] Add source spans and structured diagnostics without changing game code.
- [ ] Expose explicit numeric event records and verified reference provenance.
- [ ] Design caller-supplied include resolution with faithful preprocessing evidence.

This plan does not advance P2.2 or the function ledger. No runtime integration,
networking or asset redistribution belongs here. User authorized commits and
pushes on feature/modding-tools on 2026-11-09 and delegated this branch; merge
review remains with the user. Do not merge into master from this workstream.
