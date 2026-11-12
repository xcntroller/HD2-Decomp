# P22-HUM handoff — 2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[active contracts](analysis/human-ragdoll-source.md) and relevant linked notes.
Continue dependencies until the FULL selected checkbox completes or genuinely blocks.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Check HEAD/status before acting. Transfer
was byte-verified in ignored WORK `human-return-main-20261111`. Preserve other
LIFE/UI/PACK/research worktrees, `/tmp/hd2-p22-hum-continue` and external merges.
Latest preceding checkpointc4895a4; older hashes may differ after rebases.

No original EXE/DLL/Proton or Windows fixture execution. Native own audits only;
Windows compile/link only. Heavy compilation and Ghidra **sequentially**. No push.
Source-only checkpoints may be committed; interactive acceptance requires user.
Application mission loading remains disabled; no mission desktop verification.

## Latest source result

[Normal Mine producer receipt](analysis/checkpoints/p2.2-human-mine-producers-20261112.json)
adds5 partials/extends00477660, zero promotions:34 complete/2685 partial(2680client+
5engine).00478B90/00478D30 actual model/resource/direct pose→runtime create/activate/
bind→factory-reference release→definition then ID-based Human inventory retirement.
Late-mode optional00478CC0 uses ORIGINAL pose and SAME process broadcast streams;
module12 command prefix2+2bits, IDs16/default primary(-1)/axis mask7/direction(-1).
004788A0 authority/004788E0 presentation envelopes now dispatch actual Mine leaves;
other actor leaves remain required errors.004788A0 absent frozen getsZERO credit.
Placed flag1D/kind10 now uses SAME World ray startY+.5/displacement(0,-2,0), masks0/0,
actual hit normal or genuine miss fallback, then creator→actual item removal. Missing
required geometry fails; missing model releases temporary frame and removes placed
entry before containing mandatory-load failure. Created actor survives later bind/
inventory/wire failure. Raw overrides misleading driver-call C aliases.
ASan/UBSan/LSan prior800+400 producer lifetimes/cursor/ownership/failure PASS. Frames,
inventory/Scene2 and ray endpoints explicit doubles. Strict actual full World/Human
inventory/ray/module/Mine i386 and final launcher/probe compile/link PASS
`human-runtime-mine-producers-{host,application}-i386.log`. No Windows execution.

[Runtime Mine receipt](analysis/checkpoints/p2.2-human-runtime-mine-20261112.json)
adds13 partials/extends6, no promotions:34 complete/2680 partial (2675 client+5
engine) of18783. [Full Mine contract](analysis/human-runtime-mine-source.md).
Type2 factory005FDE30/ctor005FDEC0/vtable00812F08 allocates0x120, scene Actor_mine.
Actual005FE010 bind→timer94→sphere42/40000→unregister99→common/base/frame retirement
now uses SAME runtime registry, SceneEC, dynamic buckets and borrowed Human9C.
00479700 keeps forward increment after swap-last: skipped matching owner remains.
Raw arming005FEBB0 takes NO stack args; threshold2000, modelY+.2, radius.4 for71
/.15 otherwise. Unproduced fields, missing grid and invalid rearm fail honestly.

Mine packet leaf0/1/2/3 and effect005FEF50 now borrow SAME ShotMgr/SOUND/module17.
Native payload sound14/particle1C maps to serialized10/18 (no native vtable in
payload bytes), minimum1C.00451A80→00451DB0 negative-material branch preserves
reused D0 fields, SOUND before particles before owner removal. Positive-material
shot effects remain required. Car Mine5C and blast material100/emptyD4 are real
verified leaves; absent frozen entries get ZERO credit. Cold108-table/save
producers are not exposed. ASan/UBSan/LSan800 lifetimes/failure prefixes pass;
Frames/localization/sound/particles/grid geometry and removal predicate adapter
are explicit doubles. Actual full World/Mine/Car/blast i386 and final launcher/
probe compile/link PASS `human-runtime-mine-{host,application}-i386.log`.
Temporary audits remain ignored until full selected checkbox completion.

Earlier da59872 [blast binding](analysis/checkpoints/p2.2-human-containing-blast-20261112.json)
uses typed borrowed Human1C, final hit0 and actual lifetime→005A1C90→B0 queue.
Actual owner-map key guard precedes source dereference. EC+14 word5 is Versions
master, NOT rigid-body force;0043C6A0/0043B000 transitions remain required.
Earlier18196e5 [entry/voice](analysis/checkpoints/p2.2-human-entry-input-voice-20261112.json)
uses SAME pending inputs/ONE selected FIFO. Voice EIGHT initializer args end with
clientConfig,optionalPlaybackInterface: final ABI-v3 only, raw tail overrides bad
C aliases. Connect(audio,clientConfig,1), interface at audio18.0C publication before
init/ignored HRESULT/client-before-server close retained; positive non7 reset still
fails honestly. Attachment80 borrows existing Car/StaticWeapon seat receivers.
Earlier spawn/map/Car/contact/peer checkpoints remain valid; no unchanged solver/
clip/contact audit needed.

## Next work

1. Continue full containing Human frame/dependency boundary. Mine normal producers,
   placed-ground and module12 envelopes now have source bodies; do not reopen unchanged
   kernels. Other runtime packet leaves, save/replay and positive Mine contact-trigger
   005FEFB0 are still required if reached. Versions0043C6A0/0043B000 transitions block
   positive Car/blast frameEC word5; index lookup saved WORK
   `human-containing-versions-transition-index.txt`, existing originals reused, no new
   Versions export yet. Full contract `analysis/human-runtime-mine-source.md`.
2. Full status9/received0 entry remains unbound. Read
   `analysis/multiplayer-world/shared-world.md`.00730A40 needs actual peer6C/client334
   wrappers/worker lifecycle, then0072E890, then00731610(true). Read-only pinned
   originals are WORK `human-entry-connection-originals`. Worker registration
   underB4 writes connection08=workerD4 before append; swap-last unregister does
   not delete. Client00723D20 publishes334 thenport10/register; host007260A0 peer6C
   register AFTER418 unlock. No fake wrappers.
3. Preserve differing game publication/failure order: status9 restores retained
   game BEFORE menu shutdown and uses SAME00698510/006989B0. Received0 manager
   replacement precedes old-game retirement; failed new-game deletion differs
   from status9's explicit shutdown/delete. Non7 zone decisions/frame and full
   containing Human frame/failure/re-entry/ordered teardown remain required.
   Actual application loading/rendering/user desktop acceptance remain separate.
   Commit larger verified checkpoints, continue FULL selected Human checkbox.

## Commands

WORK=`analysis/work/mission-world-20261103`; original
`analysis/work/hd2.unpacked.exe` SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
`python3 -B tools/research_index.py find ADDRESS...` before new exports;
`neighbors ADDRESS` has no `--count`. Candidates are not equivalence proof.
`python3 -B tools/function_progress.py --render` / `--check`; `git diff --check`.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
ExportFunctions refuses existing output directories: choose new variant suffix.
Inspect ERROR even on exit0; never overlap heavy compilation. `objdump -s` ignores
address bounds: use bounded PE-section reads for data.
