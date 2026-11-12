# P22-HUM handoff — 2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, Phase2 section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[Human contracts](analysis/human-ragdoll-source.md) and relevant linked notes.
Continue dependencies until the FULL selected checkbox completes or genuinely blocks.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here; transfer byte-verified in ignored WORK
`human-return-main-20261111`. Check HEAD/status before acting. Latest preceding
checkpoint4e96476; older hashes may differ after rebases. Preserve other LIFE/UI/
PACK/research worktrees, `/tmp/hd2-p22-hum-continue` and external merges.

No original EXE/DLL/Proton or Windows fixture execution. Native own audits only;
Windows compile/link only. Heavy compilation and Ghidra **sequentially**. No push.
Source-only checkpoints may be committed; interactive acceptance requires user.
Application mission loading remains disabled; no mission desktop verification.

## Latest verified source

[Armed Mine receipt](analysis/checkpoints/p2.2-human-mine-contact-20261112.json)
adds4 partials/extends2:34 complete/2701 partial(2696client+5engine), no promotions.
[Full contact/explosion contract](analysis/human-mine-contact-source.md). Actual
005FEFB0 RET8 gates source/family, clears98/unregisters99, source9C/Human/Car24AC,
catalogue pointer→wire→late payload read→retained baseY+.2 explosion→late removal.
Actual table70/71 is type2/40bytes: radii5/8,power2500/25000,sound2.005FEEE0 disarm
wire1 does not read binding or unregister until removal. Human0059FFD0 uses SAME
radius.5/FFFBFFFF query and ONLY first candidate. Car and World borrow actual runtime/
GameBlastWaves; constructor failure and retirement unbind links in owner order.
00451990 performs actual positive wave initial100ms BEFORE late offline reused D0
SOUND(flags22/source/group6); later Mine visual effect retains its lower Y centre.
Human9C retirement during initial explosion may delete the Mine: local runtime
borrower preserves native global access without retaining/delaying actor lifetime.
Native800 contact lifetimes and separately800 actual producer→wave→Human/B0 queue
PASS with explicit endpoint doubles; strict actual World i386/final launcher/probe
PASS `human-mine-contact-*` /`human-mine-explosion-damage-native.*`. No Windows run.

[Versions receipt](analysis/checkpoints/p2.2-human-versions-transitions-20261112.json)
adds12 partials/extends8, no promotions:34 complete/2697 partial(2692client+5engine).
[Full contract](analysis/version-damage-transitions.md). Actual frameEC5/Car/blast
0043C6A0→0043B000 now borrows SAME module7 Master/Part/metadata/buckets. Lazy d+model
resource uses existing category5 cache open10; four stage arrays borrow its model.
Actual visual/PROJ/LMAP replacement precedes packet/effects; Script death/hit,
existing Physics type4/material10 (NOT18), SOUND/particles/session are shared.
Terminal Part/Master self-retirement preserves global NULL-slot recheck. Frame
release precedes array free; collision/deferred owners retire before reverse
borrowed model reloads.0043F1E0 signed16/8/8/late-authority grammar has source,
absent frozen ZERO credit. Save/replay suppression has no admitted producer.
Native400 lifecycle/cache/packet/debris sanitizer checks, strict actual full World
i386 and final launcher/probe PASS `human-containing-versions-*`. Native frame/
resource/engine/script/effect/Physics endpoints are explicit doubles. No Windows run.

Preceding59fee6f [Mine producers](analysis/checkpoints/p2.2-human-mine-producers-20261112.json)
adds5 partials; [full Mine contract](analysis/human-runtime-mine-source.md).
00478B90/00478D30 actual model/resource/direct pose→create/activate/bind→factory
reference release→definition then ID-based Human inventory retirement. Late-mode
optional00478CC0 uses ORIGINAL pose/SAME process streams: module12 prefix2+2bits,
IDs16/default primary(-1)/axis mask7/direction(-1).004788A0/E0 envelopes admit only
actual known leaves; other actor leaves fail.004788A0 absent frozen ZERO credit.
Placed flag1D/kind10 uses SAME World ray startY+.5/displacement(0,-2,0), masks0/0,
actual hit normal or genuine miss, then creator→item removal. Missing geometry
fails; failed required model removes placed entry before mandatory-load error.
Created actors survive later bind/inventory/wire failure. Native prior800+400
producer lifetimes, strict actual World/Human inventory/ray/module i386 and final
launcher/probe PASS, endpoint doubles explicit. Raw overrides bad C stack aliases.

Earlierc4895a4 [Mine owner](analysis/checkpoints/p2.2-human-runtime-mine-20261112.json)
uses actual0x120 type2/vtable00812F08, Human9C, SceneEC, runtime/buckets/ShotMgr/
SOUND/module17.005FEBB0 takes NO args; timer2000, modelY+.2, radius.4 for71/.15
otherwise.00479700 retains native forward increment after swap-last (may skip).
Payload native sound14/particle1C maps to serialized10/18, minimum1C.00451A80 C
tail is misleading: raw451AE2..451AF4 supplies SOUND→particle order. Positive
material ShotMgr branches and unproduced Mine fields remain required.

Earlierda59872 [blast binding](analysis/checkpoints/p2.2-human-containing-blast-20261112.json)
uses borrowed Human1C/final hit0, lifetime→005A1C90→B0 queue. Owner-map key guard
precedes source dereference. EC+14 word5 is Versions master, NOT rigid-body force.
Earlier18196e5 [entry/voice](analysis/checkpoints/p2.2-human-entry-input-voice-20261112.json)
uses SAME pending inputs/ONE selected FIFO. Final voice ABI-v3 has EIGHT initializer
args ending clientConfig,optionalPlaybackInterface; raw overrides bad C aliases.
Connect(audio,clientConfig,1), interface at18; publish0C before init, ignored HRESULT,
client-before-server close. Positive non7 reset still fails honestly. Attachment80
borrows SAME existing Car/StaticWeapon seats. Solver/clip/rest/Car/contact/spawn/map
checkpoints remain valid; no unchanged-kernel audit needed. Temporary audits stay
ignored until full selected Human checkbox completion.

## Next work

1. Continue positive CarD4 blast damage004DBA10, then other reached derived/surface
   receivers and full Human frame. Native Car vtable00810778 (ctor004D0B60) has
   D4=004DBA10,D8=004DBEE0. Blast006DDD40 skips D8 material forCar4 and callsD4
   directly. Index/reused original WORK `human-containing-car-blast-index.txt`,
   `car-callback-world-damage-required/004dba10.c`/JSON, pinned vtable
   `human-containing-car-blast-vtable.json`. Existing C has engine aliases; verify
   reached ABI/raw before wiring. No new Car exports yet. Preserve actual health,
   protection, frame hit, part/wheel events, shared owner and packet ordering.
   Armed Mine/normal creation/Versions now have checkpoints; do not reopen kernels.
2. Full status9/received0 entry remains unbound. Read
   `analysis/multiplayer-world/shared-world.md`.00730A40 needs actual peer6C/client334
   wrappers/worker lifecycle, then0072E890, then00731610(true). Reuse pinned WORK
   `human-entry-connection-originals`. Worker registration underB4 writes connection08
   =workerD4 before append; swap-last unregister does not delete. Client00723D20
   publishes334 thenport10/register; host007260A0 peer6C register AFTER418 unlock.
3. Preserve game publication/failure order: status9 restores retained game BEFORE
   menu shutdown and uses SAME00698510/006989B0. Received0 manager replacement
   precedes old-game retirement; failed new-game deletion differs from status9's
   explicit shutdown/delete. Non7 zone decisions/frame and full containing Human
   frame/failure/re-entry/ordered teardown remain required. Actual application
   loading/rendering/user desktop acceptance remain separate. No fake wrappers.
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
ExportFunctions refuses existing output directories: choose new suffix. Inspect
ERROR even on exit0; absent entry needs explicit rollback function creation.
Never overlap heavy compilation. `objdump -s` ignores bounds: bounded PE reads only.
