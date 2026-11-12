# P22-HUM H3 handoff — 2026-11-12

Worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h3`, branch
`feature/p22-hum-h3`, based on `feature/p22-hum` b99f088 at the user's request.
Check HEAD/status; preserve unrelated worktrees and changes. No agents or push.
No original EXE/DLL/Proton or Windows output execution. Heavy builds/Ghidra
sequential. Ignored WORK is a local copy-on-write research snapshot; tools link
existing installations. Source-only checkpoint; no desktop test or acceptance.

**H3 source is complete.** [Contract](analysis/human-seat-interaction-source.md),
[receipt](analysis/checkpoints/p2.2-human-seat-interaction-20261112.json),
[Human plan](analysis/p22-hum-plan.md). Positive offline Car/StaticWeapon seat
entry/exit, driver controls, existing mass/cameras/seat/B0 owners, mounted
assignment/reset/aim and actual S-marker casing/Physics receiver are bound.
004C1AD0 produces154=0; aim118/11C remain disengaged until actual writes.
Previous-seat temporary in00751480 is initialized-1; stdcall stack movement
caused the misleading decompilation. Car exit search and StaticWeapon FIRST-seat
pose remain distinct. Full occupied/non7 Car simulation remains **D5**, shooting
producers H4/H6, full Human frame H8, application/desktop delivery OWN/FRAME/P2.2.
Do not claim those from this checkpoint. H2 remains blocked independently below.

Four temporary native ASan/UBSan/LSan audit executables pass100 families each:
action timers/modes/races/failures; actual StaticWeapon seat/control/resource
lifecycle and aim/casing; SAME Car seat kernels with actual mass/physics/control/
SOUND/exclusions and no seeded occupants; Human-action→actual StaticWeapon
assignment/update/exit/teardown. Explicit adjacent Human/Car/engine/query/Items/
camera/B0 endpoints and the composition limits are recorded in the contract.
Actual nested World/Human/Car/Items/Physics i386 instantiation and application
launcher/startup probe build pass, compilation/link only. Temporary H3 audits
are fingerprinted in the receipt then removed; ignored research/logs retained.
A targeted additional100 engine-admission families verify F6C feature ownership
(actual solver.substeps.flags) independently of5B0 lighting flags, correcting a
final ownership-check finding before the checkpoint.
**34 complete/3025 partial (3020 client+5 engine)**:17 new frozen partials,
8 extensions, no promotions. Absent functions and delegated engine calls receive
no new credit. Previous D4a receipt remains historical and unchanged.

## Next work

The requested H3 boundary is finished. Next independent Human milestone is H4;
read its entire completion boundary and relevant existing evidence before coding.
H2 D4b/D5 and D10 remain separate open dependencies in the Human plan. H3 resolves
mounted004C6620 reset, but Car's containing update still explicitly rejects
occupied/non7 simulation; do not remove that gate without D5 reconstruction.

**Confirmed H2/D11 blockers:** fresh detached part004CB2CD reads unproduced5F9;
fresh wheel005FDD10 reads unproduced695 after body/lifetime mutation. EXPL8 and
smoke14 also remain unproduced. Do not invent defaults or repeat confirmed
searches. [Exact triggers](analysis/human-reaction-containing-boundary.md#source-closure-blockers).
D4b00586200 subtype14 additionally ignores004DEC60's ray-miss Boolean while its
XYZ outputs remain unwritten. See the existing D4a contract's caller section.

## Evidence and commands

WORK=`analysis/work/mission-world-20261103`; original client SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research index before new exports: `python3 -B tools/research_index.py find ADDRESS...`.
Local H3 index was `/tmp/hd2-h3-local.sqlite`; indexed mentions do not establish
fidelity. Contract lists ABI variants/raw evidence; Ghidra scripts roll back.

`python3 -B tools/function_progress.py --render` / `--check`; `git diff --check`.
i386 compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Native sanitizer runtime `.tools/hd2-sanitizer-runtime/usr/lib64`; no permanent tests.
