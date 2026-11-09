# Session handoff — 2026-11-09

Continue FULL P2.2, first unchecked implementation checkbox in plan.md. Read
AGENTS.md. The user now explicitly authorizes smaller verified local checkpoint
commits while continuing that full checkbox. Commit all completed changes; no
push. No subagents/goals/original game/DLL/Proton/Windows-fixture execution.
Preserve original assets. Focus verification on new work, not repeated audits.

GameApplicationState still declines loading. No live world, first frame or new
mission desktop result. Reconciled ledger:34 complete client/1,452 partial
(1,447 client/five engine)/18,783 frozen functions.881 reconciled plus eight swap and four reselection client source partials;
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

## Next implementation

Implement formation005A91A0/005A9120 and typed Human2C8 owner/2CC borrower, then
detached-camera00463040/Players module consumers. Existing formation entry exports:
WORK/human-containing-final-dependencies/005a91a0.c and00463040.c;
human-containing-activation-dependencies/005a9120.c. New dependency export:
WORK/human-formation-owner/ (check Ghidra log for completion/errors before use).
Formation owner/storage/template-reference exports completed successfully; no
Ghidra process remains. See the next-formation research section in the Human
contract. Required template data is BSS populated by a large static initializer,
not on-disk defaults.00597C50 needs assembly to resolve bad decompiler vector
results;0059B7B0 uses actual navigation projection. Do not implement placeholders.

Confirmed new behavioral details (do not repeat unchanged audits):
-00530D40/00530A40 gate signed DWORD at64hex>0 and command60!=-1. Not int64.
-Actual assembly replaces decompiler distance ordering: priority0052FB26 uses
 (Z²+X²)+Y² in extended x87 THROUGH comparison; usability00530C24 uses
 (Z²+Y²)+X² then stores FLOAT at00530C4D. Keep threshold distinction.
-Target128+30 is typed and005607B0 is supplied. Runtime steering/task224 writers
 remain separate, as does actual Car5E0 service. No fake default-false target.
-Containing Human deactivation detaches160, clears formation2CC, then retires
 formation2C8 before the remaining owners. Player bind calls005A91A0(0) too.
-Formation005A9120 iterates six slots in reverse, rereading owner after callback;
 clearing followers may destroy the owner recursively. Preserve reentrancy.

After formation: detached-camera00463040 list/current/FPV/HUD mappings; Players
module activation. Then full006989B0/
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
