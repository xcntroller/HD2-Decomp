# P22-HUM handoff — 2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the FULL selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Check HEAD/status before acting. Transfer
was byte-verified in ignored WORK `human-return-main-20261111`. Other LIFE/UI/PACK/
research worktrees and `/tmp/hd2-p22-hum-continue` are separate; preserve them and
external master merges. Latest preceding checkpoint60df3cc; older externally
rebased checkpoint hashes may differ from earlier handoffs.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra **sequentially**. No push. Source-only checkpoints
may be committed; interactive acceptance requires the user. Application mission
loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Presentation spawn receipt](analysis/checkpoints/p2.2-human-presentation-spawn-20261112.json)
adds14 partials and extends3:34 complete/2635 partial (2630 client+5 engine) of18783;
no promotions. Detailed contracts/evidence/exclusions are consolidated in the
active Human analysis.006123A0 client creates through SAME factory, authority
looks up Scene id/offlineA8; camera before roster, Scene-id side setter quirk,
actual Player/Remote flags and local match/HUD/voice.006125A0 client generation-1
BEFORE soldier decode, exact twelve-stat widths/permutation and late mode gates.
Match B8 is distinct from result88. HUD phases use SAME inline components,
flags before calls, objectives twice, actual message/dialog false bodies. Offline
pause keeps required propagation failure, positive menu/input bodies excluded.
00724570 uses actual process descriptor4E00.flag88 and wrapper0C; live peer count/
side/transport under418 then unlock→late COM2C. Native16-target overflow fails
explicitly. Camera0/2 uses actual scene virtual7C/flags4 and exact x87→DOUBLE→
floor random spectator selection, including RAND_MAX last-candidate boundary.
Prepared factory is now dependency-templated borrowed adapter, no second owners.

Focused native ASan/UBSan/LSan PASS spawn/match/packet/voice/HUD and prior unchanged
camera cases. Engine/Scene/factory/HUD endpoint doubles are explicit, not a full
World/render/transport run. Strict actual World/Human/factory/HUD i386 and latest
launcher/probe compile/link PASS `human-normal-spawn-{host,application}-i386.log`.
Temporary audits stay ignored until full checkbox completion. No Windows output ran.

Earlier checkpoints remain valid: [Car/contact](analysis/checkpoints/p2.2-human-car-contact-20261112.json),
[process input](analysis/checkpoints/p2.2-human-process-input-20261112.json),
[application messages](analysis/checkpoints/p2.2-human-application-packets-20261111.json),
[peer lifecycle](analysis/checkpoints/p2.2-human-peer-lifecycle-20261111.json).
One process input owner;0072E890 record CLEAR differs from received-start
00723000 manager REPLACEMENT. Input link stays published through B0 retirement.
Presentation2 retires peer/Human;17 sets result/hint/log, not retirement.

## Immediate next work

1. Finish00611C90 normal authority positioning/publication. Next indexed functions:
   005ED700/0069E0A0/00475840/0069D6A0. WORK
   `human-authority-spawn-position-index.txt`; focused Ghidra
   `ExportHumanSpawnPositionABI.java`/`human-authority-spawn-position-abi` and log
   may be active at checkpoint; check process/log before starting another heavy
   tool. Signatures provisional;0069D6A0 needs raw register/stack inspection.
   Raw occupancy/position assembly retained.00611C90 v4 decomp tail aliases still
   unreliable: use raw for Scene identity/pose locals, not SpawnPoint pointer.
   Writer non7 stats come from original ARG2, not copied Human properties; direct
   presentation passes NULL stats. Existing offline producer is unchanged.
2. Full entry: status9 needs00730A40/00731610 around implemented0072E890; received0
   needs00724110 optional service/current-state/map/load/sync around canonical
   replacement. Cold World manager acquisition remains a source-only lazy adapter.
   Positive00479700 kind2/9C and occupied attachment80 remain required.
3. Finish full containing reaction/frame, failure/re-entry and ordered teardown.
   Application loading/rendering/user desktop acceptance remain separate.
   Commit larger verified checkpoints and consolidate docs; do not stop at one slice.

## Commands and evidence

WORK=`analysis/work/mission-world-20261103`. Source originals/indexes and hashes
are pinned in receipts; research checkout reused read-only. Client
`analysis/work/hd2.unpacked.exe` SHA256
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
