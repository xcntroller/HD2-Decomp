# P22-RENDER handoff —2026-11-11

Work only in `/home/control/Documents/Projects/HD2-Decomp-p22-render`, branch
feature/p22-render, implementation base/fetched447e073ed90f9fc6fb0fdd305fe6d725115d1786.
No original EXE/DLL/Proton or Windows output execution; heavy compilation/Ghidra
sequential. No push/merge or other worktree edits.

[Plan](analysis/p22-render-plan.md), [implementation](analysis/p22-render-implementation.md),
[strict history proof](analysis/p22-render-history.md),
[latest receipt](analysis/checkpoints/p22-render-history-20261111.json).
Procedural/resources/debug/concrete callback services remain verified atc1bbd09.
Latest continuation binds actual render operation/camera/volume/history diagnostics
through SAME DATA store, without extra frame reads or changed render policy.
Native ASan/UBSan/LSan, concrete i386 composition and application link pass.
No new function credit; original full render/desktop acceptance unobserved.

**User decision: preserve original behavior.** No first-visible sample, invented
angle, zero delta, extra random draw or field2C bypass is authorized.
00482BB8 allocates48 via ordinary new007E520F→007E81D9→007E8205. Small-block
reuse/HeapAlloc(flag0) do not guarantee cleared payload.00481320/read/refresh
leave per-volume3C/40 unwritten;0048284E consumes them before writing current
history. Actual original heap bytes remain unknown. Owner18/1C reset(1,0) is
separate. Current reached failure is an explicit reconstruction guard, not a
claimed original error or successful rendering. Keep the first containing
checkbox OPEN. Do not repeat this confirmed allocator/constructor investigation
without new evidence. A faithful successful read requires actual original-state
provenance or an evidenced producer; do not substitute fixture scalar history.

UI now19f049b was inspected via Git only; general HUD controls advanced, RENDER
seam unchanged, relevant Human/order/occupied actor/FPV/console producers remain.
Master observed83e77da, not imported. OWN installs/activates; FRAME schedules and
updates camera; VM/configuration supplies actual GEffectsB8 calls where reached.
No source closure claim for those external owners. Prior tracker generation is
blocked by existing missing ignored rule-refresh.asm evidence; ledger unchanged.
