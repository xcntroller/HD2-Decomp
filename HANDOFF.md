# P22-HUM handoff — 2026-11-11

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the full selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Inspect HEAD/status before acting. Transfer
base50f9193 and22 pending Human files were verified; ignored MAIN WORK
`human-return-main-20261111` records before/after hashes. Other LIFE/UI/PACK/research
worktrees and `/tmp/hd2-p22-hum-continue` are separate. Preserve their work and
newer master merges. Latest preceding checkpoint7a2ec6d; external master pulls
have rebased earlier checkpoints: inspect current history.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra must run **sequentially**. No push. Source-only
checkpoints may be committed; interactive acceptance requires the user.
Application mission loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Containing application-packet receipt](analysis/checkpoints/p2.2-human-application-packets-20261111.json)
adds3 client partials/extends5:34 complete/2617 partial (2612 client+5 engine) of
18783, no promotions.0072C0B0 routes application20 BEFORE loaded-World gate through
SAME StartupSession. Presentation00728450:2 client departure log→0069E720;
10 direct SAME0069BC40 objective apply without score/FIFO echo;17 local result4/
clock/HUD hint→late mode gate→scalar0072F2F0 log.17 does NOT retire peer/Human.
0072EC90 forward active08/transport00 lookup locks418, unlocks before name query.
One actual process006C4B40 log route is shared by Human and received messages.

Non7 owned queue drain is now distinct from transport. Authority domain174==1
requires real007239F0 BEFORE queue pop; authority application grammar, history
packet decoder, unknown presentation commands and non7 flush/transport still
throw. Current packet retires on receiver failure, later queue nodes survive.
No new UI/loading/connection success. Native ASan/UBSan/LSan PASS independent
wire, actual scalar FIFO/broadcast/queues/objective/session/peer policy, late mode/
name/vector growth, failures and100 lifetimes; platform/HUD/log/Scene endpoints
are doubles. Strict actual World i386 and launcher/probe compile/link PASS
`human-application-packets-{host,application}-i386.log`; no Windows output ran.

Preceding [cold peer/Human lifecycle](analysis/checkpoints/p2.2-human-peer-lifecycle-20261111.json)
checkpoint7a2ec6d retains actual00612A60 distinct ECX peer/EDX Scene identities,
0059EDC0/0059EB60 live setup/writeback and late3C/A8/aux publication; actual0069E720/
005A7FE0 ordered delayed retirement; Remote7 FC7/8/10 filter; HUD timer/rank/name;
mode2/1 containing composition guard with reset-before-erase map publication.
Six focused native fixtures and strict World/launcher checks PASS there. Positive
00479700 kind2 Human-reference9C and occupied attachment80 remain required.
Native destructor/delayed record clears use Scene identity, not peer slot.

Preceding [metadata/request](analysis/checkpoints/p2.2-human-network-mission-metadata-20261111.json)
and [reaction/authority retirement](analysis/checkpoints/p2.2-human-reaction-authority-retirement-20261111.json)
checkpoints remain valid. Do not re-audit unchanged kernels or conflate source
receivers with admitted network/application success.

## Immediate next work

1. Continue actual normal non7 spawn/entry/transport.00611C90/006123A0/006125A0
   callers still reject before unrecovered tails. Full containing Human frame
   depends on actual peer3C producer/caller lifetime, beyond verified cold factory.
2. Correct old entry wording:0072E890 clears peer3C/stats under418, calls0073F5A0
   to CLEAR registered input records (manager and embedded30 survive), then clears
   actual team stats/scores. It does NOT replace the manager.006C20F0 retires prior
   application state before this branch and constructs new game afterward; exact
   game/global lifetime integration still needs matching source. Recover canonical
   process input ownership; no second manager or invented reverse-map cleanup.
3. Finish full containing Human frame/reaction, failure/re-entry and ordered
   teardown. Application loading/rendering and user desktop acceptance remain
   required. Commit larger verified checkpoints; keep documentation consolidated.

Research indexes: WORK `human-control-next-index.txt`, `human-application-scalar-index.txt`
and `human-entry-input-index.txt`. Reused originals `human-application-reused-originals`
pin research C read-only; raw00699A20 result/hint evidence is retained.
Peer evidence: `human-peer-lifecycle-abi` correct fastcall read-only rollback
exports; `human-peer-lifecycle-reused-originals/provenance.json` pins research
checkout C reused read-only. Static vtable60/FC and raw HUD/override/aux/timer leaves
retain local evidence. No broad unchanged-system audit is needed.

## Commands and evidence

WORK=`analysis/work/mission-world-20261103`. Earlier metadata original reuse:
`human-network-map-reused-originals/provenance.json`; focused Ghidra read-only
rollback exports `human-network-mission-abi` / `human-network-selection-abi`, logs
complete without ERROR. Raw `human-network-*` assembly retains field/retirement
and ordinal evidence. Network parameter globals009820D4/D8/DC are loader-zero
.data tail; focused reference export pins later packet/advertisement/console users.

Client=`analysis/work/hd2.unpacked.exe`, SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
`python3 -B tools/research_index.py find ADDRESS...` before new exports;
`neighbors ADDRESS` has no `--count`. Candidates are not equivalence proof.
`python3 -B tools/function_progress.py --render` / `--check`, `git diff --check`.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
Inspect ERROR even on exit0; never overlap with heavy compilation.
