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
newer master merges. Latest preceding checkpoint05dbdee; external master pulls
have rebased earlier checkpoints: inspect current history.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra must run **sequentially**. No push. Source-only
checkpoints may be committed; interactive acceptance requires the user.
Application mission loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Cold peer/Human lifecycle receipt](analysis/checkpoints/p2.2-human-peer-lifecycle-20261111.json)
adds7 client partials/extends7; totals34 complete/2614 partial (2609 client+5 engine)
of18783, no complete promotions. [Detailed contract](analysis/human-ragdoll-source.md)
records original evidence, field/ABI boundaries and focused verification.

Actual00612A60 has distinct ECX peer/EDX Scene identity, live non7 clothing and
0059EDC0 setup, fixed slot order and0059EB60 signed WORD identity writeback,
then late3C/timeGetTime A8/auxiliary publication under418.00730140/00730460 use
SAME records.0069E720 has ordered peer/HUD/override/spectator/auxiliary/runtime/
delayed Scene2 retirement; peer3C/stats clear only after return.005A7FE0 retires
actual observation borrowers/DD0 collisions/late B0 FC, without deleting Human.
State7 Remote00592220 forwards7/8/10 only; absent frozen inventory, zero credit.
HUD refresh retains hidden primary borrower; actual timer geometry and rank/name
receivers now use SAME source owners.0045E8F0 includes camera/FPV rebind.

Containing-owner adaptation now follows actual current mode2 then1; client2
retires the external composition before Human destruction. Keep owner-map key
published through reset-before-erase for SAME loading_lifetime callbacks; reject
re-entered retirement/new activation. No second Human/input/peer owner graph.
00479700 positive kind2 Human-reference9C and occupied attachment80 still require
actual original receivers and throw. Native destructor/delayed record clears use
Scene identity, not peer slot: preserve that quirk and recover containing entry.

Six focused temporary native ASan/UBSan/LSan fixtures PASS, including preparation,
peer delayed lifecycle, Remote/Scene retirement, HUD timers, authority packet
regression and same-owner mode guards. Most have100 lifetimes; guard modes1/2/3/7
x100. Engine/component/platform endpoints are doubles, not a full World run.
Strict actual World/factory/HUD/retirement i386 and latest launcher/startup-probe
compile/link PASS (`human-peer-lifecycle-{host,application}-i386.log`). No Windows
output ran. Temporary fixtures remain ignored for the ongoing full checkbox.

Preceding [metadata/request](analysis/checkpoints/p2.2-human-network-mission-metadata-20261111.json)
and [reaction/authority retirement](analysis/checkpoints/p2.2-human-reaction-authority-retirement-20261111.json)
checkpoints remain valid. Do not re-audit unchanged kernels or conflate source
receivers with admitted network/application success.

## Immediate next work

1. Continue actual normal non7 spawn/application20 packet/entry/transport chain.
  00611C90/006123A0/006125A0 callers still reject before unrecovered tails.
   SessionLocalPackets still rejects application20 and non7 transport. Original
  00728450 reads application commands BEFORE loaded-World gate; received2/10/17
   are distinct from module array dispatch. Scalar17 publishes/logs, not removal.
2.0072E890 clears old peer borrowers and replaces canonical process input manager
   BEFORE old World shutdown. Adapt SAME ownership; no invented reverse map or
   fixture-only borrower cleanup. Positive00479700 kind2/9C needs original research.
3. Finish full containing Human frame/reaction, failure/re-entry and ordered
   teardown. Application loading/rendering and user desktop acceptance remain
   required. Commit larger verified checkpoints; keep documentation consolidated.

Research index run for next seeds in WORK `human-control-next-index.txt`.
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
