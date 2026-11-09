# Session handoff — 2026-11-09

Continue FULL P2.2: first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Latest user instructions: do not stop after dependencies; continue
until the full checkbox is done or concretely blocked. Commit at larger verified
subsystem checkpoints and consolidate documentation/tally work there. No push,
subagents/goals, original game/DLL/Proton or Windows-fixture execution. Preserve
assets/user changes; focused verification, no repeated broad audits.

GameApplicationState still declines loading. No live world, first world frame
or new mission desktop acceptance. Ledger:34 complete/1,490 partial (1,485 client,
five engine)/18,783 frozen; no whole-body promotions. README/tally reconciliation
was committed as20ca6cc; later swap/reselection/formation checkpoints are committed.
See [ledger history](analysis/function-ledger-p2.2-reconciliation.md).

## Latest verified subsystem

Detached-camera actual kind3 frame/Human amount records and containing Players
module11/process loading are source. SAME Human/DD0/roster/selection/formation
owners replace cleanup declarations. Original cached-zero/pointer-only removal,
reverse passes, shared process reset preservation, formation scale3.75, loading
timers/groups, delayed speech dispatch, follower cleanup and module registration/
embedded-control/auxiliary lifecycle are retained. StartupSession owns actual
process0089AC30 and separate delayed-speech0089AC20 lifetimes.

[Camera and Players contracts](analysis/human-state-loading.md#detached-camera-human-references--2026-11-09),
[checks/fixture hashes](analysis/checkpoints/p2.2-detached-camera-20261109.json).
Native ASan/UBSan/LSan, strict actual-host i386 and app compile/link PASS. Temporary
fixtures hashed/removed. Twenty-one eligible partials (six camera/fifteen Players),
zero complete promotions.004650A0/004593E0/00460B30 absent inventory earn none.
Frame/DD0/presentation/speech boundary doubles are not desktop verification.

## Next implementation — keep working

Continue containing006989B0/session/world composition and actual00469300 first
camera simulation, using these SAME process/module/Human owners. Players current
source covers loading lifecycle; subgroup00461290/save/remote records and mode9
formation producers remain separate. Active250 populated orders explicitly reject
raw writes; no successful defaults. First-person/HUD presentations still require
real wiring. Camera motion/toggle/input bodies remain outside detached amount work.

Remaining full checkbox also includes actual compiler/VM00427730/runtimePatch
ISD1 programs; SOUND portals/subtitles/effects; required actor/physics1/3/4/6;
missionA8 objectives/zone37 A4 identities; first rendered frame and user failure/
repeat acceptance. Captured Tutorial146 ISD0 base scripts and B810 bindings are
not runtimePatch ISD1 programs. Runtime actors before physics; descriptors before
buckets; entries before Items. Do not add alternate viewer or placeholder services.

Canonical details: [world boundary](analysis/mission-world-loading.md),
[source index](analysis/loading-modules.md), [Human state](analysis/human-state-loading.md),
[equipment](analysis/human-equipment-loading.md), [Sight/sound](analysis/sight-loading.md),
[HUD](analysis/hud-loading.md). Confirmed weapon/input/proximity/formation/math
contracts need no new audit without contradictory evidence. Offline7 is ordinary
single player; initial Human19C1 becomes active3, not player1. DD0 teardown follows
B0 close. Proximity008AED54 is separate from graphical GEffects; native nonzero
callback quirks and zero-only expiry are deliberate.

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored, durable). Camera evidence in
WORK/detached-camera-{reference-producers,amounts-and-players,distance-players-reset}/;
Players in players-process-ownership/,players-process-consumers/,
players-auxiliary-records/,players-owned-record-producers-correct/,
players-loading-records-final/ and saved players-process-tick.asm. Ignore invalid
0045DDA0/00460170 overlapping probes; they are not evidence. Last Ghidra export
players-loading-records-final completed; poll session47623 to close if needed.

Ordinary shell default sandbox. Ghidra configuration writes and ASan/LSan runs
require escalation; one Ghidra process/read-only project/fresh export directory.
Always check ERROR despite exit0; section-map PE reads. Sanitizer libraries:
.tools/hd2-sanitizer-runtime/usr/lib64 (verified signed Fedora44GCC16 packages,
extract only). i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
