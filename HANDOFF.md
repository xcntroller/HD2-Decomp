# Session handoff — 2026-11-03

## Latest result

The scalar-owner/input-foundation batch is complete **source-only**, following the
user's larger-batch/one-final-sweep policy. [Contract and verification](analysis/scalar-input-batch.md),
[all41 candidate dispositions](analysis/scalar-input-batch.json),
[original bytes/calls](analysis/scalar-input-batch-map.json).
All41 candidates reviewed:11 complete bodies,30 specific remaining recovery tasks.
Do not describe those30 as externally blocked or defer them solely for native ABI.

Closed: status0067EA10; raw inhibition0066A860; reply clear006691A0; line scan006E5300;
x87 limits006C6C90; reserved scans006A8230; ordinal006A8290; edit006A83C0;
conflicts006A8480; unassigned006A85B0; settings clear006A8740. Normal compiled
callers use these bodies with original i386 field/call/return contracts. Binding
edit checks full DWORD arguments before truncation; native old/new fixture proves
the earlier error. ECX-only no-stack methods match the original receiver ABI.
Constructor006A81C0 is NOT complete: its header byte is now confirmed as high byte
of saved this (PUSH ECX, MOV CL,[ESP+3]), not an unwritten value. It also returns
EAX=this; native header0/construction ABI still differs. The ledger is corrected.

Ledger: **20 complete client,544 partial (539 client/five engine)**, denominator
18,783 unchanged; no new entries/engine credit. Status0067EA10 matches exact
instructions; ten other new bodies differ as documented. Earlier two exact matches
remain separate. The comparison parser now consumes every raw byte on long
instructions, verified against independent COFF sections for all11 candidates.

## Verification and preservation

One consolidated native ASan/UBSan fixture passes40000 editor/config transactions,
131072 reserved scans,20000 line scans/parser integration,144000 x87 triples in12
valid rounding/precision modes,raw field/queue/write/alias/capacity checks. Strict
application/preview/startup i386 builds, Windows fixture compile/link, existing
native regressions,158 Python tests, original map and ledger pass. Windows fixture
needed only an explicit void cast for intentional vtable byte inspection; no app
source fix required another full sweep. Temporary fixtures removed; logs and
source/artifact/preservation hashes: analysis/work/scalar-input-batch-20261103.
No original game/DLL/server/Proton/Windows fixture executed. No new desktop claim.
No staging/packaging/commit/push requested or performed. Preserve the user's staged
and unstaged edits, original assets, configs/profiles and existing dist artifacts.
The user handles packaging; test actual desktop only through their Python launch.

## Next work

The expanded pre-Phase2 completion checkbox remains **OPEN**: complete all currently
feasible functions before moving on. [Policy/current queue](analysis/function-completion-queue.md),
[machine queue](analysis/function-completion-queue.json).559 entries at expansion,
15 since closed (the first five button functions predate that queue),544 open.

Next batch has17 related candidates: original input-manager/configuration raw
construction/vector lifetime, binding installation/applicators/defaults, producer
and startup. Their exact remaining work is in the41-entry batch index. Research
its full dependency boundary before editing. Current typed vector layout is
proven only for the nonallocating editor methods; do not infer allocator, SEH,
header-padding, original-global manager008AEA48 or device lifetime closure.
The original first header byte is pointer-derived; retain that confirmed finding.
Screen callbacks still need focus+6C/raw children+08/virtual input ABI; queue/common
routes need segmented storage/intrusive payload ownership. Do not repeat confirmed
investigations. Never substitute successful placeholder services.

Use quick affected-source compiles during implementation, then one consolidated
sanitizer/build/regression/evidence sweep and one ledger/queue/documentation update.
AGENTS.md requires stopping after one verified milestone, now a subsystem batch.
Do not mark the overall pass exhausted while rows merely await body/dependency
research. Phase2 planning/implementation waits. No subagents or permanent new tests.
Commit only when requested; interactive changes need user desktop acceptance first.
No push. Loadouts remain COMPLETELY UNTESTED until accessible; broader save metadata,
actual game/save/results and ready-server UI retain their documented deferrals.
Those screen deferrals are not blanket source-recovery blockers.

Build: make -C recovered application-launch engine-window startup-probe
WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
User launch: python3 -B tools/run_application_launch.py
(optionally --fullscreen --width 2560 --height 1440 --language Czech).
Agent staging only with --stage-only if needed; never run Proton. Checks:
make -C recovered test; python3 -B -m unittest discover -s tools -p 'test_*.py';
python3 -B tools/function_progress.py --check.
