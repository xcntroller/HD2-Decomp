# Independent runtime diagnostics handoff

Branch/worktree: `tooling/runtime-diagnostics`, based on available local master
`ca321e2`. Existing branch/worktree was already correctly isolated. No fetch,
merge or push; research/future-dependencies and other worktrees preserved.

Delivered `cli.py` with offline analyze/capture/compare commands, deterministic
JSON and Markdown, bounded event storage, source line/hash evidence, explicit
six-state uncertainty model and sanitized export. Three version1 JSON schemas,
user-operated existing-launcher capture instructions and a focused instrumentation
proposal are alongside it. See README.md and VERIFICATION.md for commands and
53 verification assertions against actual existing logs and synthetic negatives.

Game source, launcher, root handoff/plan, function ledger and milestone acceptance
were not changed. No desktop runtime executed. Current host declines activation;
resource-input capture and menu frame counts do not demonstrate a first world
frame. Diagnostics are useful now for failure evidence; positive intermediate
stages need real markers when the primary implementation integrates them.

Next: user runs the existing launcher on their desktop, saves the direct host log
and captures it with explicit exit/build/outcome metadata. Agent #1 may adopt the
small marker proposal if useful; update parser rules against actual new producer
strings before claiming those stages instrumented. Acceptance remains a user
desktop decision. P2.3–P2.9 can reuse run metadata, stable events and comparison;
additional lifecycle markers should be versioned and evidence-backed.
