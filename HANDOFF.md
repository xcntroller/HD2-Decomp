# Session handoff — 2026-11-09

Continue FULL P2.2, first unchecked implementation checkbox in plan.md. Read
AGENTS.md. The user now explicitly authorizes smaller verified local checkpoint
commits while continuing that full checkbox. Latest user instruction: DO NOT STOP
after a dependency/checkpoint; continue until the full checkbox is done or a
concrete blocker requires user/external input. Commit all completed changes; no
push. No subagents/goals/original game/DLL/Proton/Windows-fixture execution.
Preserve original assets. Focus verification on new work, not repeated audits.

GameApplicationState still declines loading. No live world, first frame or new
mission desktop result. Reconciled ledger:34 complete client/1,469 partial
(1,464 client/five engine)/18,783 frozen functions.881 reconciled plus eight swap, four reselection and seventeen formation partials;
zero whole-body promotions. See analysis/function-ledger-p2.2-reconciliation.md.

## Saved checkpoint

The accumulated source was reviewed and split into15 local commits after
f9674b3, through f72ca56. The user undid the subsequent research-record commit;
its documentation is restored in the README/ledger reconciliation checkpoint.
[Verification record](analysis/checkpoints/p2.2-20261109.json) saves build/test
results and prior receipt digests. Strict app/startup i386 compile/link passes;
250 new headers compile together; existing native menu text test and64 relevant
Python tests pass. No Windows program or original DLL was executed. The earlier
user-run resolved-input capture remains the only relevant new desktop action.

[Human-state contracts](analysis/human-state-loading.md),
[equipment contracts](analysis/human-equipment-loading.md),
[Sight/sound contracts](analysis/sight-loading.md) and
[source progress](analysis/loading-modules.md) are canonical.

Latest source: actual Human containing lifetime, Players roster/current/selected/
frame buffers, process proximity owner008AED54, derived sound/look callbacks,
weapon priority/candidate/usability policy and slot-specific inventory lookup.
HumanLoadingInventoryCommands binds SAME Human.loading_inventory before B0 bind;
active/area AI now calls its actual source1/2→empty-hand3 offline loading command.
Player/remote queued commands and nonoffline outgoing packets explicitly reject.
HumanEquipmentTransfers uses typed230 peers4/0C, SAME holsters/state/DD0/E0 and
00792590/00792720/00792AB0 notifications. Shoulder model remains attached until
007913D0 event3 removes source kind2 and attaches hand kind3. Events0..3 only;
action-model producers remain separate.007B96C0 stow uses631C FLOAT×200→signed64
truncate/EAX6314 and native flags. Inventory/expiry/forced E0 stores verified.

Latest verified checkpoint: same-Human weapon swap007D9DE0 now composes
007DA5E0 preflight, retained displaced return,007D9B40 fallback and native
no-source/no-destination drop. Offline AI command005A2BB0/005A2E90 and hand
selector00538770 are source.00791EB0 fades BOTH actual animation lanes via
00774CC0, with350/1.4→truncate250. Focused actual RuntimeItem/storage/230 and
command fixture passes native ASan/UBSan/LSan and strict i386 compile; app
compile/link passes. Fixtures fingerprinted/removed. Eight eligible partials
added; absent00587E00 earns none. No whole-body or desktop promotions.
[Swap contracts](analysis/human-equipment-loading.md#same-human-weapon-exchange--2026-11-09),
[saved checks](analysis/checkpoints/p2.2-weapon-swap-20261109.json).

Latest source continuation: active00530D40→00530A40 now binds actual typed
AI128+30/attachment160/task224, actual CD0 sector/scene root and Items/hand
selection to offline inventory commands. Initial active bind/close passes.
005607B0 resets typed target and six scalar accumulator fields;005AC4C0 reads
SAME DD0/CD0 sector. Special-hand IDs now use actual Items. Native sanitizers,
strict i386 fixture and app compile/link pass. Four partials added, no complete
or desktop promotions. [Reselection contract](analysis/human-state-loading.md#active-weapon-reselection--2026-11-09),
[checks](analysis/checkpoints/p2.2-weapon-reselection-20261109.json).

Latest verified source: Human2C8/2CC formation mode3 ownership/member/leader/
recursive retirement now replaces required external cleanup in Human activation
and player bind. Actual mode3 table005965E0→005971E0, WORD buffers, delegated
engine SetDir/vector transforms and SAME MissionNavigation floor fallback/
projection are supplied. Runtime modes/orders/steering excluded. Native
ASan/UBSan/LSan (89 failures), actual host i386 and app compile/link pass.
Seventeen eligible partials added, no whole-body/desktop promotions.
[Formation contract](analysis/human-state-loading.md#formation-loading-owner--2026-11-09),
[checks](analysis/checkpoints/p2.2-formation-20261109.json). Fixtures hashed/removed.

## Current implementation — keep working after checkpoints

Continue detached-camera00463040 and Players module11 ownership/activation,
then containing world/VM and required loading consumers. First unchecked P2.2
implementation checkbox remains the full scope; do not mark it complete from
source-only dependencies. Camera list is module8C, separate from control60
collected-frame buffers.00463040 copies only the last actor pointer into the
removed8-byte slot; second DWORD and producer still need research.005A9760
camera amount setter is already supplied in human_camera_amount.h.

Formation evidence WORK/human-formation-owner/,human-formation-storage/,
human-formation-navigation/,human-formation-navigation-fallback/, saved
human-formation-*.asm and template references. No Ghidra process remains.
Confirmed: recursion can delete SAME leader owner; reread after each callback.
Engine delegates earn no body credit; mode3 table data receives none either.
Do not re-audit unchanged weapon/proximity/input/math findings without new cause.

Remaining full boundary: detached-camera/Players module, then006989B0/
session/world ownership/camera00469300; actual compiler/VM00427730/runtimePatch
ISD1 programs; remaining SOUND portal/subtitle/effects and actor/physics1/3/4/6;
missionA8 objectives/zone37 A4 identities; first rendered frame and user repeat/
failure acceptance. Runtime actors before physics; descriptors before buckets;
entries before Items. No successful missing services or alternate viewer.

## Retained verified boundaries

Offline7 is normal single-player. Human19C1 initially becomes active3, not player1.
State publication/handover/derived close and SAME input/action/perception pools
are verified. Native derived close does not automatically call base close.
Proximity is NOT graphical GEffects; fixed100 tick/zero-only expiry and original
nonzero→nonzero callback quirks are confirmed. Do not simplify/re-audit them.
Human190 populated tasks and AI D4 threat producers remain typed-producer gaps.
HUD source wrapper remains uncalled; command-before-offline7 and0049B7E0 loaded
flag4 are required. Registry32 holes are not active actors; Graph74/78 are not
Driver18/1C.004A7EB0 draws goal text, not objective state. Captured Tutorial146
ISD0 base scripts are not runtimePatch ISD1 programs. B810 bindings are not VM
programs. Preserve resource2/native34 dotted item keys and items.dat.

## Execution

WORK=analysis/work/mission-world-20261103. Ordinary shell uses default sandbox.
Ghidra config writes and ASan/LSan runs need require_escalated. One Ghidra process,
fresh export directory; check ERROR even on exit0. Section-map PE reads.
Sanitizer libraries now persist in ignored .tools/hd2-sanitizer-runtime/usr/lib64
(exact signed Fedora44 GCC16.2.1-3 packages; extract only, fingerprints in record).
Use -L/rpath there; native fixtures may be created temporarily under ignored WORK
so crashes do not erase them before recording. Remove after fingerprinting.
i386 compiler:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
