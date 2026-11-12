# P22-HUM handoff — 2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [active original contracts/gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the FULL selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Check HEAD/status before acting. Transfer
was byte-verified in ignored WORK `human-return-main-20261111`. Other LIFE/UI/PACK/
research worktrees and `/tmp/hd2-p22-hum-continue` are separate; preserve them and
external master merges. Latest preceding checkpoint18196e5; older checkpoint
hashes may differ after external rebases.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra **sequentially**. No push. Source-only checkpoints
may be committed; interactive acceptance requires the user. Application mission
loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Containing blast receipt](analysis/checkpoints/p2.2-human-containing-blast-20261112.json)
extends4, adds0: still34 complete/2667 partial. Human D8=66 and actual D4 now reach
SAME lifetime/005A1C90/Player B0 queue. Record/Request1C is a typed borrowed Human
pointer, not DWORD/serialized identity; final hit-kind0. Actual World compares live
owner keys before dereferencing target/source. Native mode1/2/3/7 sourced/NULL800
lifetimes and material/authority/failure/node-retirement checks PASS; geometry/ray/
Versions and remaining damage endpoints are doubles. Actual World/i386 and latest
launcher/probe compile/link PASS. No application or desktop execution.

Concrete correction: frameEC+14 word5 is a Versions master, not Physics force;
0043C6A0/0043B000 part transitions still require real bodies. Historical
`blast_physics_impulse` endpoint name remains; its required diagnostic is corrected.
Other actor D8/D4 and full containing frame remain required.

Earlier entry checkpoint:

[Entry input/voice](analysis/checkpoints/p2.2-human-entry-input-voice-20261112.json)
18196e5 added18 partials.00731610 resets SAME pending records and clears ONLY
selected FIFO under430; forced stale aliases/nonprogress fail honestly.00724110
prefix and actual0C voice owner share existing settings/transmit interface04.
Final voice ABI is `human-entry-voice-abi-v3`: EIGHT initializer args end with
clientConfig,optionalPlaybackInterface; Connect(audio,clientConfig,1), interface
is audio18. Creation publishes before init, ignores HRESULT; client closes before
server, global0C clears after deletion. Required non7 reset still fails; source
fallback cleanup is explicitly separate. Attachment80 now borrows existing
Car/StaticWeapon seat receivers. Detailed safety differences/contracts live in
[Human source](analysis/human-ragdoll-source.md); no unchanged solver/clip/contact
audit is needed. Temporary audits stay ignored until the full checkbox completes.
Earlier authority/presentation spawn, Car/contact, process input, application
messages and peer lifecycle checkpoints remain valid.

## Immediate next work

1. Full control entry/status9 and received0. Read
   `analysis/multiplayer-world/shared-world.md`.00730A40 still requires actual
   peer6C/client334 connection producers/worker lifetime, then0072E890, then new
   00731610(true). WORK `human-entry-connection-originals` reuses read-only pinned
   00745D20/00746B50/00746D20. Constructor owns three queues and20 lock; worker
   register underB4 writes connection08=workerD4 BEFORE append; unregister swap-last
   does NOT delete. Actual00723D20 client publishes334 thenport10/register; host
   007260A0 creates peer6C then registers AFTER418 unlock. No fake wrappers.
2. Received0 full configuration/map/player/game publication/load/sync/request9
   remains unbound; prefix now exists. Status9 restores retained game BEFORE menu
   shutdown and initializes SAME00698510/006989B0. Received0 manager replacement
   precedes old game retirement and its failed new game deletion differs from
   status9's explicit shutdown/delete. Preserve these distinctions.
3. Actual runtime00479700 kind2 Human-reference9C owner is still missing.
   Original Mine factory005FDE30 allocates0x120, ctor005FDEC0/vtable00812F08;
   scene name Actor_mine. Timer94 reaches2000 then005FEBB0 (NO stack args) arms
   type42/mask40000 sphere at modelY+.2, radius.4 for item71/.15 otherwise;
   deactivation unregisters99 before common. WORK human-runtime-kind2-* pins
   lifecycle/tick/vtable, read-only originals and ABI-v2. Some genuine leaves
   absent frozen, zero credit. Do not use older FEbb0 forced one-arg prototype.
   Exact005FDFB0 index lookup was late, after its export; do not claim otherwise.
   The positive9C producer005FE010 and00478B90 still require full Mine owner.
   The forward
   ownership scan already preserves native increment-after-swap behavior. Non7
   zone decision/frame receivers and full containing Human reaction/frame,
   partial failure/re-entry/ordered teardown remain. Existing actual attachment80
   routing is now bound; positive occupied update/actions still need their real
   receivers. Application loading/rendering/user desktop acceptance are separate.
   Commit larger verified checkpoints and continue the FULL selected checkbox.

## Commands and evidence

WORK=`analysis/work/mission-world-20261103`. Client original
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
ExportFunctions refuses an existing output directory; choose a new variant suffix.
Inspect ERROR even on exit0; never overlap with heavy compilation. Full `objdump
-s` ignores address bounds: use bounded PE-section reads for data instead.
