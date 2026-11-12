# P22-HUM H2 handoff —2026-11-12

Active worktree **`/home/control/Documents/Projects/HD2-Decomp-p22-hum-h2`**,
branch **`feature/p22-hum-h2`**, created at user request frome742f84;
fb89d76 records the move. Older HUM checkout is preserved. Check current
HEAD/status before acting. Default environment cwd may still point at LIFE:
use this H2 worktree explicitly. Other worktrees advance independently; no blind
reset/merge and no edits to their uncommitted source. Ignored WORK is the local
copy-on-write research snapshot; .tools/ghidra reuse installed tools.

Read AGENTS.md, Phase2 section2 plan.md, [Human plan](analysis/p22-hum-plan.md),
[ragdoll contracts](analysis/human-ragdoll-source.md). **H2 remains OPEN**.
S01–S14, D0–D3, D4a/D4b and now D6a are bounded source checkpoints. No whole
frame, live transport or desktop claim. No agents/push. Never run original
EXE/DLL/Proton or Windows outputs. Heavy compilation and Ghidra sequential;
check other worktrees' heavy jobs before ours. Temporary audits stay until D12.

## Latest verified source — D6a cold connection and entry registry

[Contract and exact remaining boundary](analysis/human-entry-connection-source.md),
[receipt](analysis/checkpoints/p2.2-human-entry-connections-20261112.json).
Ten NEW frozen partials00744ED0/00745350/007458D0/00744FC0/00745410/
00745D20/00745E40/00746B50/00746D20/00730A40. **34 complete/3021 partial
(3016client+5engine)**; no promotions. D6 full remains OPEN; D6b is now explicit.

- Actual00745D20 owns cold unreliable/reliable/ordered queues (ten independent
  sentinel owners), two FRESH clock reads (first44, second3C/40), scalar fields
  and OS20 critical section AFTER all queue allocations. NULL outer queue14/18/
  1C allocations are retained and construction continues.00745E40 retires lock
  BEFORE ascending queues. Cold queue retirement is implemented; no populated
  packet producer/release or protocol success is invented.
-00746B50 uses SAME B4 lock, writes workerD4 to wrapper08 BEFORE vector growth
  and preserves duplicates.00746D20 removes first match via swap-last, without
  deletion/08 clearing. Cold workerD4 stays UNPRODUCED: only actual native start
  writes0; the audit explicitly seeds prior workerD4, not a fake worker start.
- Actual C4 peer6C and process334 are typed borrowers of SAME connection08.
  Copy preserves identity; native record init clears6C without deleting it.
  00730A40 authority locks418 and processes named stage74==2 records forwards;
  client toggles localAC without418. Full-word XOR1 precedes required wrapper
  dereference; failure retains that AC and earlier-row mutations. Source RAII
  unlocks on checked errors; native fatal fault recovery is not claimed.
- Actual StartupSession has constructor/clock/Win32 lock and entry routes. These
  do not enable host/join/application entry. Source lifetime uses STL/RAII and
  typed pointers, not native layout/allocator/SEH ABI.

Native100 families PASS ASan/UBSan/LSan using actual connection/queue/record/
registry/entry owners, with OS clock/lock/allocation doubles. Includes all three
NULL queue cases, failed registry allocation after08 publication, duplicates,
first-match swap-last/no deletion, modes1/2/3/7, missing wrapper mutation prefixes,
record copy/reset, repeated entry and lock-before-queue retirement. Strict actual
StartupSession i386 PASS; final launcher/probe rebuild PASS. No Windows execution.
Receipt freezes6 source files/10 artifacts/56 evidence files. Ledger --check and
README render pass. New exports/raw/index files use `human-h2-*` names in WORK.

## Next work and external dependency ownership

Continue selected H2 through dependencies, preserving known blockers honestly.
Do not stop merely to repeat D11, and do not reopen previously verified kernels.

1. **D6b actual worker/socket/thread/caller lifetime:** expanded original exports
   are already available; index before more exports.007466E0 constructs inline
   socket007474E0 then B4; D4 is unproduced.007467B0 createsB0, starts00747690,
   resets end, writesA4/AC/D4 and CC/D0, then creates007466D0→00746EB0 worker.
   Recover reached callbacks, socket buffers/events and exact00746840/00747750
   stop ordering; native1000ms waits do not establish safe source joining.
   Preserve00723D20 client334 publication BEFORE port/register and007260A0 peer6C
   publication under418, unlock BEFORE registry append. Unregister does not delete;
   caller disconnect/delete/6C clear and00746890 reverse delete are separate.
2. **D5 mounted/occupied/non7** is being edited in sibling H3 worktree: its
   uncommitted mounted_weapon_control(_loading), car_loading_activation,
   human_state_base_loading, human_ai_state_loading, world_actor_loading_consumers
   and host/game_world_actors overlap these exact receivers. Leave those changes
   untouched; integrate its verified checkpoint when available. H4 likewise has
   uncommitted common action-completion/AI/world-owner changes. Do not copy them.
   Prior D5 research/raw is preserved;004C6620/004C6DD0 evidence needs no new export.
3. **D7/D8** actual status9/received0 owners still depend on D6b; see shared entry.
   SAME00698510/006989B0, restore retained game BEFORE menu shutdown, actual input
   replacement BEFORE old World retirement and distinct failure deletion remain
   required. OWN worktree has advanced application lifecycle independently; review
   its committed dependencies before integration, no blind merge.
4. **D10** actual unmounted0077DE30→0074D510→007B0910→007A7470 composition can
   advance independently of transport; existing audits still use separate control/
   selection/geometry endpoints and do not close this real containing-owner task.

**D11 genuine closure blockers unchanged:** fresh part004CB2CD reads/toggles
unproduced5F9 BEFORE motion; fresh wheel005FDD10 reads unproduced695 at <=500ms/
expiry after body/lifetime mutation. EXPL record8/smoke14 also have no established
producer. No defaults, warm fixtures as fresh success, or repeated unchanged
producer audits. [Exact triggers](analysis/human-reaction-containing-boundary.md#source-closure-blockers).
D12 consolidated containing audits/ledger credit/temporary audit cleanup remain
open; actual application/render integration and user desktop acceptance remain P2.2.

## Commands

WORK=`analysis/work/mission-world-20261103`; original=`analysis/work/hd2.unpacked.exe`,
SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Index BEFORE exports: `python3 -B tools/research_index.py find ADDRESS...`;
candidates/absence are not proof. Never mutate frozen ignored receipt artifacts;
use new variants. Native g++17/O0/warnings-as-errors/ASan/UBSan/frame pointers,
runtime `.tools/hd2-sanitizer-runtime/usr/lib64`. i386 compiler
`.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
Ghidra JAVA_HOME=/home/control/.sdkman/candidates/java/25.0.0.0-oracle, project
/home/control/Documents/Projects/HD2ResearchProject/HD2Research, process
hd2.unpacked.exe -readOnly -noanalysis; ONE local WORK scriptPath, rollback only.
Check ERROR even exit0; no overwrite dirs. Bounded objdump-d/PE reads only;
objdump-s ignores bounds. No Windows execution or application acceptance.
