# Session handoff — 2026-11-09

Continue FULL P2.2: first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Do not stop after dependencies; continue until the full checkbox is
done or concretely blocked. Commit at larger verified subsystem checkpoints and
consolidate documentation/tally there. No push, subagents/goals, original game/
DLL/Proton or Windows-fixture execution. Preserve assets/user changes; focused
verification, no repeated broad audits or confirmed-finding reinvestigation.

GameApplicationState still declines loading. No live world, first world frame
or new mission desktop acceptance. Ledger:34 complete/1,529 partial (1,524 client,
five engine)/18,783 frozen; no whole-body promotions. README/tally are reconciled.
Camera/Players committed as7e4387f; latest source subsystem is described below.
[Ledger history](analysis/function-ledger-p2.2-reconciliation.md).

## Latest verified subsystem

[Offline activation and prepared spawning](analysis/session-activation-loading.md)
now recover006989B0 ordering, actual borrowed LIGHT filtering, scene world bounds
and SAME session/history/process quantizer bindings; prepared Human profile/
speech/storage/expiry, cold SAME-model clothing/animation/link/foreign attachment
transfer, and actual local broadcast spawn packet production. SAME live squad
008AE910 is distinct from HumanStore008AE90C. Native quirks: model key retains
dot; jitter rand(6)-4; clothing shortcut strstr(resource34,model134); packet
finish target0 leaves cursor pending.006739F0 scalar defaults earn no new body
credit.39 new eligible partials/four extended rows, zero complete promotions.

[Checks/body selections/hashes](analysis/checkpoints/p2.2-prepared-spawn-20261109.json):
ASan/UBSan/LSan, strict actual-host i386 composition and app compile/link PASS;
14 temporary fixtures/artifacts hashed/removed. Actual Human/RuntimeItem/storage,
input/history/global quantizers and captured catalogue used. Frame/cache/audio/
presentation/activation boundary doubles and declared containing endpoints are
not desktop verification. No new interactive behavior was enabled.

## Next implementation — keep working

Complete actual local flush0072C5D0→0072C230 presentation queue→0072C3B0/
0072C0B0→module13 decoder and006123A0. New sessions game1C=0 STILL serialize local
packets in offline7. Flush appends terminator19, enqueues, then resets cursor;
0072BF90 alone does not enqueue. Module dispatch0048DC50/70 uses actual module
index/vtable28/24, with null module genuine false. Actual camera command0/2
0046CB20 and first simulation00469300 follow. Command0(0,presence1,param4=0)
clears26D and calls0046C370 target selection; command2 is a native no-op in7.
Use SAME MissionCamera/Players/Human/inline98 owners, no fake successful endpoint.
Cold clothing rejects active DD0/B0/Sight/Players refresh; those wider branches
remain separate active gameplay work. Required real setup/direct/lock/event
receivers still need containing host world composition.

Remaining full checkbox: actual compiler/VM00427730/runtimePatch ISD1 programs;
SOUND portals/subtitles/effects; actor/physics1/3/4/6; missionA8 objectives/zone37
A4 identities; real first frame and user failure/repeat acceptance. Tutorial146
captured ISD0 base scripts and B810 bindings are not runtimePatch ISD1 programs.
Runtime actors before physics; descriptors before buckets; entries before Items.
No alternate viewer/placeholders. Other canonical notes: [world boundary](analysis/mission-world-loading.md),
[source index](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[equipment](analysis/human-equipment-loading.md), [Sight/sound](analysis/sight-loading.md),
[HUD](analysis/hud-loading.md). Continue implementation after verification.

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored, durable). Original offline
activation: session-activation-first-camera/,world-camera-required-helpers/;
prepared: offline-squad-spawn/,offline-squad-human-equipment/,
offline-squad-storage-and-input/,prepared-item-construction/;
clothing: spawn-clothing-model-change/,clothing-attached-frames/ plus saved
clothing-model-full.asm,clothing-contexts.asm,clothing-foreign-children.asm;
packet: offline-spawn-packet/,offline-spawn-packet-queue/,
session-packet-world-owner/,session-buffer-construction/,
session-packet-actor-generation/,session-quantize-vector.asm.
Latest valid export offline-local-packet-dispatch/ completed (session61753);
contains0072C230/170/C0B0/CA10 and0046CB20. Invalid0073D140/006A22B0 and
camera-spawn-reset/00465C70 probes are NOT body evidence; ignore export errors.
Read0046C370 from camera-secondary-owner/; virtual40 is0046C370, not00465C70.

Ordinary shell default sandbox. Ghidra configuration writes and ASan/LSan runs
require escalation; one Ghidra process/read-only project/fresh export directory.
Check ERROR despite exit0; section-map PE reads. Sanitizer libraries:
.tools/hd2-sanitizer-runtime/usr/lib64 (verified signed Fedora44GCC16 packages,
extract only). i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Source-only and desktop delivery remain distinct. General ABI cleanup and byte
matching are selective follow-up, not a new exhaustive gate.
