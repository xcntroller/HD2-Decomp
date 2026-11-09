# Session handoff — 2026-11-09

Continue FULL P2.2, first unchecked implementation checkbox in plan.md. Read
AGENTS.md. The user now explicitly authorizes smaller verified local checkpoint
commits while continuing that full checkbox. Commit all completed changes; no
push. No subagents/goals/original game/DLL/Proton/Windows-fixture execution.
Preserve original assets. Focus verification on new work, not repeated audits.

GameApplicationState still declines loading. No live world, first frame or new
mission desktop result. Reconciled ledger:34 complete client/1,440 partial
(1,435 client/five engine)/18,783 frozen functions.881 new client source partials;
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

Human weapon transfer source007D8B40 supports SAME-Human/no-source weapons in
slots1/2/3/6, dry run, cooldown precedence, displaced entries and retained weight
accounting. Focused ASan/UBSan passes; its containing swap is NOT implemented.
Prior actual-equipment native/i386 fixture logs survived, but their temporary
files were lost before hashing in the PC crash. Do not invent removal hashes.
New checkpoint fixtures were fingerprinted/removed in the saved record.

## Next implementation

Finish actual active00530D40→00530A40→00538770 reselection and007D9DE0 swap.
Successful evidence under WORK: human-loading-weapon-selection/,
human-loading-inventory-commands/,human-inventory-command-receivers/,
human-inventory-immediate-transfer/,human-inventory-equipment-callbacks/,
human-inventory-equipment-retirement/,human-equipment-weapon-stow/,
human-inventory-weapon-swap/,human-inventory-swap-dependencies/. All jobs ended;
no Ghidra process remains. Ignore older failed action/reset export directories.

Important confirmed source:
-005A2D50 slot lookup ECX is resolved SOURCE actor, from receiver assembly.
- AI+74=00587BF0/+78=00587E00: immediate receiver; no packet in offline7.
- Player/remote commands queue on84; do not make them immediate.
-007D9DE0 calls007DA5E0 preflight,007D8B40 first notify0, then displaced-item
 transfer override1/retain1;007D9B40 fallback, else actual world drop. Never drop
 an entry silently or substitute the original no-source loading helper.
-007DA5E0 uses displaced item's real permitted_slots, storage capacity/volume/
 flags, current actor limit, stack occupancy and source-slot replacement deltas.
-007D9B40 fallback uses NO source: pack5, head0, hand3/back2 where eligible,
 then holsters1/pack6. Preserve the original unusual kind3/back check grouping.
-00791EB0 holster refresh condition DD0+1E8==20hex then00774CC0; ftol operand
 still needs assembly. Containing swap callbacks/FPV and fallback remain open.
-005AC4C0 returns DD0+CE4, i.e. existing CD0.sector(). Compare SAME actual
 scene root sector. Expose existing typed sector through DD0/inline; no new lookup.
- Target128+30 and active224 still need typed producers. Cold target constructor
 genuinely writes NULL. Do not cast raw scalar DWORDs to host actor pointers.
- Holder+48=007D6890 unsigned(game28<field18); +4C stores expiry. Stack-array
 +48/+4C genuine no-op mapping still needs saved byte confirmation.

After equipment: formation005A91A0/005A9120 typed2C8/2CC; detached-camera00463040
list/current/FPV/HUD mappings; Players module activation. Then full006989B0/
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
