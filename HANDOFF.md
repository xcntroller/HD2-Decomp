# P22-FRAME handoff — 2026-11-17

Actual worktree `/home/control/Documents/Projects/HD2-Decomp-p22-frame`, branch
feature/p22-frame. User priority: clear HUM, FRAME, then OWN on its branch toward
actual game entry. H824c7310 was fast-forwarded here; it already reconciles FRAME
with current Human/World/Items/physics and source input/fixed/update/lifecycle.
No conflicts, original/Windows execution or push. Preserve other agents' source.

Latest: [actual HUD input](analysis/p22-frame-live-input-source.md),
[receipt](analysis/checkpoints/p22-frame-live-input-20261117.json).
0049FC10/0049F090/0049D300 now bind actual roll/signals, input manager snapshots,
map108FC (not center), command/inventory/message owners. Native4175 ASan/UBSan/
leaks PASS; actual World/UI/session all entry points i386 link PASS; forced
launcher/probe PASS. Temps fingerprinted/removed. Three frozen partials only.

HUM/FRAME not globally resolved: selected H8.4 and full containing/application
execution remain. Old FRAME2026-11-13 review of HUM51082a2 is stale for Active,
timed action/motion, physics and authorized D11 phase0. Reuse current H8 closures;
do not repeat broad unchanged audits or expand unselected Dog variants.

Next: actual Human order display queries005A5ED0/5F00/5F50 and Player input
005A3DE0→0053BF90; adapt actual same-owner APIs for OWN. Use research index before
new exports. Query in progress/rootH8 evidence recorded in WORK/frame-live-human-
bridge-index.txt. Then integrate into OWN's branch, preserving frame retirement
before callback/UI/World close and source-only versus desktop gates. OWN939fb70
has actual activation/controller/render but deliberately fails missing_frames.
Its remaining UI producers/cutscene requirements are documented in OWN plan.
No false ready flags or successful required-service placeholders.

Tools/data live in H8's ignored .tools/analysis/work. This FRAME worktree has only
needed evidence/inventories copied so far; use absolute tool paths or copy existing
indexed files, preserving provenance. Heavy compile and Ghidra sequential. No
permanent tests. Commit at verified source checkpoints; push only when requested.
