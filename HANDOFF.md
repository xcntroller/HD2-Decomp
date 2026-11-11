# P22-HUM handoff — 2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the FULL selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Check HEAD/status before acting. Transfer
base50f9193/22 pending Human files was byte-verified in ignored WORK
`human-return-main-20261111`. Other LIFE/UI/PACK/research worktrees and
`/tmp/hd2-p22-hum-continue` are separate; preserve them and external master merges.
Latest preceding checkpoint e77c833; earlier checkpoints were externally rebased.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra **sequentially**. No push. Source-only checkpoints
may be committed; interactive acceptance requires the user. Application mission
loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Car/contact/protection receipt](analysis/checkpoints/p2.2-human-car-contact-20261112.json)
adds ONLY004DE800 and extends0059F2C0/0049DE50:34 complete/2621 partial (2616
client+5 engine) of18783; no promotions. Detailed original contracts are in the
active Human analysis, including retained C/raw evidence and exact exclusions.

004DE800 uses actual seat0 and local peer3C, both-null equality, repeated virtual78,
kind19C and late mode. Ragdoll007CBF90/007CC740 directly call004F2790 after that
policy in both modes; unsupported blanket non7 impulse guard removed. SAME actual
Car body receives impulse; wider Car hit58/non7 event publication still required.
Human Car callback now binds through SAME loading_lifetime→existing005A07C0→B0 C8.
Post-damage B0/current mode are reread; native unordered speed also enters damage.
Missing B0/lifetime/damage failure remains honest.005A07C0/0059C2D0/C8 leaves absent
frozen: zero new credit. Existing uncredited collision/impulse kernels reused,
no new credit here.0059F2C0 protection prefix now reads config250/rule10 (NOT
respawn23C/rule5) and reaches SAME injury TextState before local initialization.

Focused native ASan/UBSan/LSan PASS ownership matrix/impulse100x4, contact100,
protection HUD100, and owner retirement modes1/2/3/7x100 after interface update.
Engine/policy/damage/lifetime/HUD endpoints are explicit doubles, not a full World
run. Strict actual World/Car/Human i386 and latest launcher/probe compile/link PASS
`human-ragdoll-car-authority-{host,application}-i386.log`. No Windows output ran.
Temporary native fixtures stay ignored for this ongoing full-checkbox task.

Earlier verified checkpoints remain valid:
[process input](analysis/checkpoints/p2.2-human-process-input-20261112.json) e77c833,
[application messages](analysis/checkpoints/p2.2-human-application-packets-20261111.json) b4e9617,
[peer lifecycle](analysis/checkpoints/p2.2-human-peer-lifecycle-20261111.json)7a2ec6d,
[network metadata](analysis/checkpoints/p2.2-human-network-mission-metadata-20261111.json)05dbdee.
ONE canonical process input manager/profiles/callback bundle; World/B0 borrow a
stable source handle.0072E890 clears records; received-start00723000 REPLACES
manager before old-game retirement. Do not merge these protocols. Old Player
close after its input record retires remains required failure. Presentation
application2 retires peer/Human;17 sets result/hint/log, not retirement.10 applies
objective status without score/FIFO echo. Queues outlive World; no false transport.

## Immediate next work

1. Continue actual non7 spawn callers00611C90/006123A0/006125A0, entry and transport.
   Full caller boundaries remain required despite verified00612A60 factory/setup/
   publication and matching delayed retirement. Normal rules, match phase, camera/
   selection/HUD/name, generation/stat decode and roster changes need actual owners.
2. Full entry: status9 needs00730A40/00731610 around implemented0072E890; received0
   needs00724110 optional service/current-state/map/load/sync around canonical
   replacement. Cold World manager acquisition is explicitly a source-only lazy
   adapter, not native control-start timing. Positive00479700 kind2/9C and occupied
   attachment80 remain required. Preserve Scene-id versus peer-slot clear quirks.
3. Finish full containing reaction/frame, failure/re-entry and ordered teardown.
   Keep actual application loading/rendering/user desktop acceptance separate.
   Commit larger verified checkpoints and consolidate docs; do not stop at one slice.

Research indexes: WORK `human-control-next-index.txt`, `human-application-scalar-index.txt`,
`human-entry-input-index.txt`, `human-entry-input-replacement-index.txt`,
`human-reaction-network-leaves-index.txt`, `human-actor-contact-index.txt`,
`human-respawn-activation-index.txt`, `human-spawn-protection-hud-index.txt`.
Prior reused-original provenance remains pinned; research checkout is read-only.

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
