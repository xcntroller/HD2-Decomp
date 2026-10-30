## Autonomous reconstruction workflow

When instructed to continue:

1. Read `HANDOFF.md` and the current milestone in `plan.md`.
2. Select the next meaningful subsystem from `plan.md` and finish one coherent,
   testable slice in the iteration.
3. Read only the relevant research notes and source files.
4. Investigate original behavior before implementing it.
5. Preserve evidence for important behavioral claims, including original function addresses.
6. Implement the smallest testable reconstruction.
7. Run the relevant builds, tests, and sanitizer audits.
8. Update the research notes, plan, function ledger and handoff as applicable.
   Keep `HANDOFF.md` to the latest result and next work; link detailed contracts
   from analysis documents. Keep plan checklist entries short.
9. Stop after one verified milestone or when blocked.

Do not repeatedly investigate findings already marked as confirmed unless new evidence contradicts them.

Do not claim desktop verification unless the actual desktop test has been performed.

Prefer a complete, verified reconstruction of a small subsystem over a broad but speculative implementation.

## Project boundaries

- Rebuild the original launch path to the menu before expanding menu flows.
  Recover actual dependencies; do not add successful placeholder services.
- Preserve original game files and existing user changes. Do not execute the
  original game, DLLs or Proton. Compile/link Windows fixtures without running them.
- Use relevant existing tests and temporary native sanitizer audits. Do not add
  permanent tests under the current project agreement; remove temporary audits.
- Mark source-only recovery separately from integrated, desktop-tested delivery.
  Only credit implemented bodies present in the frozen function inventory;
  delegated engine calls and absent functions receive no new credit.
- Commit when requested. Changes to interactive behavior require user desktop
  confirmation before committing; source-only work needs no invented desktop test.
  Do not push without explicit instruction.
