## Autonomous reconstruction workflow

When instructed to continue:

1. Read `HANDOFF.md` and the current milestone in `plan.md`.
2. Select the next unchecked checkbox from `plan.md`. Research its entire
   completion boundary and dependencies before implementation, then implement
   the complete checkbox in one sweep. Do not stop at a smaller source slice
   unless blocked or the user explicitly requests that narrower scope.
3. Read only the relevant research notes and source files.
4. Investigate original behavior before implementing it.
5. Preserve evidence for important behavioral claims, including original function addresses.
6. Implement the smallest testable reconstruction that completes that checkbox.
7. Run the relevant builds, tests, and sanitizer audits.
8. Update the research notes, plan, function ledger and handoff as applicable.
   Keep `HANDOFF.md` to the latest result and next work; link detailed contracts
   from analysis documents. Keep plan checklist entries short.
9. Continue after dependencies/checkpoints until the selected full checkbox is done or blocked.

## Audit and Verification Policy

1. Prioritize implementation progress while maintaining correctness and fidelity to the original game.
2. Perform lightweight verification during implementation: compile checks, relevant call signatures, ownership, known invariants, and focused tests.
3. Perform targeted audits when completing a major subsystem or encountering a concrete discrepancy.
4. Reserve comprehensive audits for major phase milestones, significant architectural changes, or evidence of systemic problems.
5. Do not repeatedly audit unchanged, previously verified code without a specific reason.
6. Do not initiate broad audits automatically after every implementation batch.
7. When an audit identifies issues, prioritize actionable defects affecting the current execution path.
8. Maintain a concise record of unresolved risks and assumptions rather than repeatedly investigating them without new evidence.
9. Preserve faithful reconstruction of original functions and behavior. Reduced audit frequency must not justify speculative implementations or bypassing required functionality.
10. Continue implementation after successful verification rather than starting another audit cycle.
11. Do not repeatedly investigate findings already marked as confirmed unless new evidence contradicts them.
12. Do not claim desktop verification unless the actual desktop test has been performed.
13. Prefer a complete, verified reconstruction of a small subsystem over a broad but speculative implementation.

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
- Commit at larger verified subsystem checkpoints while continuing the full plan
  checkbox; consolidate documentation/tally updates at those checkpoints.
  Changes to interactive behavior require user desktop
  confirmation before committing; source-only work needs no invented desktop test.
  Do not push without explicit instruction.
