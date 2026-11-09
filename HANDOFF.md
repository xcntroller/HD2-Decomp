# Session handoff — 2026-11-09

Continue FULL P2.2: first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Do not stop at dependencies/checkpoints; continue until the full
checkbox is done or concretely blocked. Larger verified subsystem commits only;
consolidate docs/tally there. No push, subagents/goals, original game/DLL/Proton
or Windows-fixture execution. Preserve assets/user changes; focused verification,
no repeated broad audits or confirmed-finding reinvestigation.

GameApplicationState still declines loading. No live world, rendered first world
frame or new mission desktop acceptance. Ledger:34 complete/1,557 partial (1,552
client/five engine)/18,783 frozen. README/tally reconciled; zero complete
promotions. Prior prepared-spawn checkpoint23db715; latest source follows below.

## Latest verified subsystem

[Local spawn dispatch and ordinary Human camera](analysis/camera-local-dispatch-loading.md):
pending local broadcast flush0072C5D0→owned presentation FIFO→module13→SpawnPoint
006125A0/006123A0→SAME Players/Human. Offline camera commands0/2 precede roster
append. Actual source CollisionRay/tree/descriptors supplement sphere sweep;
ordinary00469300/00468130/00466970 source now covers input/follow/pose, required
Human getters/exclusions, breathing/shake/fade, sector/recoil/zoom timers.
Static camera singleton0089AEA0 proves initial BSS50/58/27C zero. Both breathing
rates0080FEE0/FEF0 are DOUBLEs; native FST retains extended comparison value.

[Checks/body selections/hashes](analysis/checkpoints/p2.2-camera-local-dispatch-20261109.json):
ASan/UBSan/LSan, strict actual-host i386 composition and app compile/link PASS;
27 tracker tests PASS.28 new partials/seven extended; absent frozen00475630 and
delegated engine calls get no new credit. Ten temporary fixtures/artifacts hashed
and removed. Engine transforms, UI/inline/world boundary doubles and required
containing declarations are not runtime/desktop verification. Source pump/camera
are not connected to a successful loaded host world. Wider attached/debug/look
camera and ray primitive families explicitly reject; see contract.

## Next implementation — keep working

Continue actual compiler/context/VM00427730→0042C850, bindings init and containing
world composition. New research script-compiler-required/ has0042CF90 reset,
0042C580 #include expansion and0042C0B0 parser dispatch. These are researched,
NOT implemented/credited. Read0042CC00 lexer and0042DE80 command lookup next;
resolve real command/operand owners and context lifetime before compiler/VM work.
Actual runtime loose/Patch precedence remains authoritative: captured Tutorial146
ISD0 base source scripts and B810 bindings are NOT runtimePatch ISD1 captures.
No successful script context/VM/empty init substitute.

Remaining full checkbox: containing host world/session/Human/FPV/HUD composition;
SOUND portals/subtitles/effects; actor/physics1/3/4/6; missionA8 objectives/zone37
A4 identities; genuine first frame, ordered failures/repeat and user acceptance.
Runtime actors before physics; descriptors before buckets; entries before Items.
No alternate viewer/placeholders. Canonical notes: [activation/spawn](analysis/session-activation-loading.md),
[world boundary](analysis/mission-world-loading.md), [source index](analysis/loading-modules.md),
[Human](analysis/human-state-loading.md), [equipment](analysis/human-equipment-loading.md),
[Sight/sound](analysis/sight-loading.md), [HUD](analysis/hud-loading.md).

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored, durable). Camera originals:
camera-simulation-typed-abi/,camera-required-ray-fallback/,camera-ray-narrow-required/,
camera-ray-shapes-required/,camera-human-required-getters/,camera-module-allocation/;
saved camera-follow-pose.asm,camera-opacity-call.asm,camera-ray-traversal.asm and
first-camera-simulation.asm. Local packet originals: offline-local-packet-dispatch/,
scene2-spawn-presentation-decoder/,spawn-presentation-decoder.asm. Prepared
source evidence remains linked from its contract/checkpoint. Earlier invalid
0073D140/006A22B0/00465C70/overlapping00475610 probes are NOT body evidence.

Ordinary shell default sandbox. Ghidra config writes and ASan/LSan execution need
escalation; one Ghidra process/read-only project/fresh directory; check ERROR even
if exit0. Latest script-compiler-required/ export completed (session9832).
Java25 at/home/control/.sdkman/candidates/java/25.0.0.0-oracle. Native sanitizer
libraries:.tools/hd2-sanitizer-runtime/usr/lib64 (already verified; do not reinstall).
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
General ABI cleanup/byte matching are selective follow-up, not an exhaustive gate.
