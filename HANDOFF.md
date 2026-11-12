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
external master merges. Latest preceding checkpoint6995623; older checkpoint
hashes may differ after external rebases.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra **sequentially**. No push. Source-only checkpoints
may be committed; interactive acceptance requires the user. Application mission
loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Entry input/voice receipt](analysis/checkpoints/p2.2-human-entry-input-voice-20261112.json)
adds18 partials, extends2, no promotions:34 complete/2667 partial (2662 client+5
engine) of18783. Actual00731610 uses SAME process14 current records, resets first/
second pending lists, selects ONE FIFO BEFORE430, then node/count before payload
release. No flush or other-FIFO clear.0073E5A0/E600/E660/E6E0 actual pending words/
identity/erase policy are in source. Forced deletion of a shared alias still makes
later access fail; no deferred refcount cleanup. No-decrement/nonforced positive
count would spin natively and reports nonprogress. Source allocation RAII reclaims
unconsumed payloads after native destructor node cleanup, where original leaks.
Actual packet-history20/B0/Car message producers remain required, not fabricated.

00724110 actual5044/descriptor88/profile38/optional voice/manager replacement prefix
is Startup-bound, BEFORE old-game retirement.00724370 publishes actual0C before
window/init; ignored HRESULT retained. Actual COM backend/factory GUIDs and same
settings/transmit04 view. Raw caller resolves EIGHT00741E20 args: window,server,
client,connection,mode,compressionGUID,clientConfig,optionalPlaybackInterface.
**Do not trust older v1/v2 stack aliases.** Final `human-entry-voice-abi-v3` plus raw
`human-entry-voice-{create,helpers}.asm` pin this. Connect(audio,clientConfig,1),
optional interface is audio18. Native0x24/0x40 sizes are HEX, not decimal.
Client Disconnect1/Release/null before server Stop0/Release/null; process0C stays
published through deletion. Repeated creation native leaks remain represented by
allocation-only bookkeeping until source final fallback. Cold/non7 reset guard
still reports unsupported positive networking; no successful teardown placeholder.

Human delayed/deactivation attachment80 now borrows SAME existing Car004D9A40 /
StaticWeapon00604630 seats before160/164 clear. Kernels unchanged; added actual
World binding compile-verified, not full occupied World/frame execution. Both seat
bodies are absent frozen: no new credit. ASan/UBSan/LSan pending-input/queue and
voice native checks pass, mode1/2/3/7 x100 and failure prefixes. COM/window/clock/
locks/replacement endpoints are explicit doubles. Strict actual complete World/
Human/entry/voice i386 plus final sequential launcher/probe compile/link pass
`human-entry-input-voice-{host,application}-i386.log`. No Windows output ran.
Temporary audits stay ignored until full selected checkbox completes.

Earlier [authority spawn/map](analysis/checkpoints/p2.2-human-authority-spawn-map-20261112.json),
[presentation spawn](analysis/checkpoints/p2.2-human-presentation-spawn-20261112.json),
[Car/contact](analysis/checkpoints/p2.2-human-car-contact-20261112.json),
[process input](analysis/checkpoints/p2.2-human-process-input-20261112.json),
[application messages](analysis/checkpoints/p2.2-human-application-packets-20261111.json)
and peer lifecycle remain valid; do not repeat unchanged solver/clip/contact audits.
Detailed contracts/exclusions live in the active Human analysis.

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
3. Actual runtime00479700 kind2 Human-reference9C owner is still missing; forward
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
