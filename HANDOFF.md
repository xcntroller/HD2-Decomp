# P22-HUM handoff — 2026-11-11

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the full selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Inspect HEAD/status before acting. Transfer
base50f9193 and22 pending Human files were verified; ignored MAIN WORK
`human-return-main-20261111` records before/after hashes. Earlier LIFE/UI/PACK
worktrees and `/tmp/hd2-p22-hum-continue` are separate, inactive for this task.
Preserve their work and newer master merges. Latest preceding checkpoint83e77da;
external master pulls have rebased earlier checkpoints: inspect current history.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra must run **sequentially**. No push. Source-only
checkpoints may be committed; interactive acceptance requires the user.
Application mission loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Network mission metadata/request receipt](analysis/checkpoints/p2.2-human-network-mission-metadata-20261111.json)
adds14 client partials/extends006B8700/006B9190; totals34 complete/2607 partial
(2602 client+5 engine) of18783, no complete promotions.006B5890/006B5960/006B83B0
are implemented but absent frozen: zero credit. Do not re-audit unchanged kernels.

Actual map/rules codecs now use SAME application configuration and process
parameters. Destructive reads preserve partial prefixes; count>255 writers reject
explicitly. ApplicationStartupData constructs ONE stable MP mission13 owner
(adapting native inline+14) before configuration; preparation reload graph creates
no second MP owner.006B8700 clears/rebuilds SAME mission, actual kind12 objectives,
ordered scalars, retained name/metadata, resource at GameData38/mission24 and
status4 finalization. Native table destination copies a fourth UNWRITTEN DWORD;
source keeps it undefined.006B7F50 clears end cursor, **not allocation**.

MissionRequest distinguishes owned offline descriptor from borrowed actual MP
mission. Valid non7 process/config04 can have empty campaign/live squad; offline
still requires both squads. Borrow actual process/config/route refs. Existing
source route is the adapted standalone-mode authority; its precedence rejects
this network factory before MP lookup. Full Saved descriptor remains required.
SessionSelection uses actual application GameData for network request;006B8EC0
returnsFFFFFFFF for absent campaign selection, with root check before standalone.
No new positive UI/connection/status9/activation path is exposed.

Native ASan/UBSan/LSan PASS `human-network-mission-native.log`: independent wire,
signed fields/rules416, same-owner map→mission→Player objectives, copy/packet/map
retirement, destructive failure, actual loaded selection, offline-empty rejection,
SessionOwner failure prefixes and100 constructions/retirements. Backend/engine/OS
endpoints are doubles. Strict actual World i386 and launcher/startup-probe compile/
link PASS `human-network-mission-host-i386.log` and
`human-network-mission-application-i386.log`. Neither Windows output was run.

Preceding checkpoints remain valid: [reaction/authority retirement](analysis/checkpoints/p2.2-human-reaction-authority-retirement-20261111.json),
[peer/console](analysis/checkpoints/p2.2-human-peer-console-20261111.json),
[objective/session](analysis/checkpoints/p2.2-human-death-objective-session-20261111.json).
0074D510 receives DD0+AA0, not Look. Selected retirement uses SAME owner refs and
late sound pool lookup. Authority17/damage and10/drop reach actual effects;
Remote nonreserved C4 is genuine empty00583220; input0 copies C4/1C4 BEFORE queue
insertion. Motion group priority3 clamps2; finish motion before ordinary0, with
native authority FIRST stream alias preserved. Other PACK effects retain failures.
PACK drop writer multiplier15 is confirmed across PUSH20AB: retain
`(speed-1)*.25*15`;005B1F10 x87 and005A8320 name fallback remain preserved.
0072F2F0 publishes scalar17 BEFORE logging; it does not remove the peer/Human.
Scalar application FIFO uses actual application2C, not418/430/peerB4; non7 flush
still throws before envelope and retains node. Console is one actual lazy Win32/
GDI owner, never desktop-tested; console parser/frame and kind12/210 remain open.

## Immediate next work

1. Finish ordinary non7 peer3C creation/configuration/storage **with matching
   borrower teardown**.00612A60 ECX peer slot differs from EDX Scene identity;
   late3C/A8 write and remote-AI auxiliary publication occur under418 AFTER
   actual0059EDC0.0059EB60 still rejects non7 identity writeback; recover full
   reached setup and lifecycle before removing it. Never publish a dangling peer.
2. Research index already run for00730140/00730460/0069E340/00722180/00746B50/
   00746D20; WORK `human-network-peer-lifecycle-index.txt`. Reuse ignored original
   C in research checkout `analysis/work/future-dependencies/p28-*` read-only;
   0069E720 additionally needs its own lookup before any new export. Actual MAIN
   C00612A60/0059EB60/0059EDC0 are under WORK `offline-squad-storage-and-input`
   and `offline-squad-human-equipment`. Current source still rejects positive
   non7 spawn/configuration/decoder branches. Full entry/transport remains required.
3. Continue full containing Human frame/damage/reaction, partial failure/re-entry
   and ordered teardown. Temporary native fixtures stay ignored for this ongoing
   full checkbox; no permanent tests. Commit larger verified checkpoints and
   consolidate notes/ledger. Full selected checkbox, P22-HUM and P2.2 stay OPEN.

## Commands and evidence

WORK=`analysis/work/mission-world-20261103`. Latest metadata original reuse:
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
