# P22-HUM handoff — 2026-11-11

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll** remains OPEN.
Read `AGENTS.md`, section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md)
and [original contracts/current gaps](analysis/human-ragdoll-source.md).
Continue dependencies until the full selected checkbox completes or genuinely blocks.

## Active checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
explicitly requested returning here. Inspect HEAD/status before acting. Transfer
base50f9193 and22 pending Human files were verified; MAIN ignored WORK
`human-return-main-20261111` records before/after hashes. Earlier LIFE/UI/PACK
worktrees and `/tmp/hd2-p22-hum-continue` are separate, inactive for this task.
Preserve their work and newer master merges. Latest preceding checkpoint5981afa; inspect current HEAD.

No original EXE/DLL/Proton or Windows fixture execution. Compile/link only.
Heavy compilation and Ghidra must run **sequentially**. No push. Source-only
checkpoints may be committed; interactive acceptance requires the user.
Application mission loading remains disabled; no mission desktop verification.

## Latest verified source checkpoint

[Reaction/authority retirement receipt](analysis/checkpoints/p2.2-human-reaction-authority-retirement-20261111.json)
adds one client partial0074D510/extends four; totals34 complete/2593 partial of18783,
no complete promotions. [Peer/console](analysis/checkpoints/p2.2-human-peer-console-20261111.json)
and [objective/session](analysis/checkpoints/p2.2-human-death-objective-session-20261111.json)
remain the preceding dependency checkpoints. Contracts are consolidated in the
source note; do not re-audit unchanged solver/particles.

World now admits actual non7 authority reaction17/damage and drop10; remaining
missing PACK effects reject at their actual receivers.6 forwards AFTER action
only non7, with Scene2/B0 recheck. Nonreserved Remote authority is genuine empty
00583220; reserved Remote consumes authority DISCARD. Input0 stores C4/1C4 BEFORE
queue insertion. Motion4 uses priority3(clamped2), ordinary0, lazily opening groups
and finishing motion before ordinary; authority FIRST may alias both to2BC.
Native tests preserve that actual append quirk, not an invented split.

0074D510 selected retirement now shares one source body with actual DD0, with all
original owner refs and phase order. Pool lookup remains late. Native Look hit→
selection→retirement and packet→damage→actual Player B0→packet→presentation pass
ordered failure/re-entry, callback B0-before-Human retirement and100 lifetimes.
These are source/native boundary tests, NOT a full World/application run.

0072F2F0 now preserves absent record00 return, scalar17(reason,peer) publication
BEFORE localized text, then actual006C4B40 application routing. It does NOT itself
remove the peer/Human.0069F1E0 owns one actual lazy Win32 console, published before
CreateWindow re-entry; damage/rank/nonrender messages use SAME output owner.
Console text copies before48 lock, prepends/caps100, unlocks then invalidates.
Submitted input owns a FIFO. Actual window/font/paint/scroll/status4-close source
exists, with callback exceptions rethrown at the actual control phase. Source
RAII also reclaims HWND/font and pending strings beyond original cleanup.
No window/desktop behavior has been tested. Console parser/frame and application
kind12/210 remain required. Win32/GDI bodies absent frozen receive zero credit.
00744FC0 is the NEXT transport class, not console cleanup; do not reuse it here.

Objective0069BA60 uses score→status/HUD→scalar10 queue.0069BC40 completion/result
scan is offline only.00724EA0 is signed(side+1)%2, not local team. All B064 states
read70. Application queue uses DISTINCT application2C lock, not418/430/peerB4;
false appends in BOTH modes, true drains. Each node flushes1000 first. Non7
transport still throws before envelope writing and retains the pending node.
Kind12 fields are explicit;006B8700 only has a one-row producer, not full normal
selected-map admission/publication. Ordinary non7 peer3C producer also remains.

PACK writer discrepancy remains corrected: PUSH20AB changesESP, so FIMUL20B2
reads the mask15 at[S+08], NOT zero. Keep `(speed-1)*.25*15`.005B1F10 retains x87
until final FLOAT store;005A8320 fallback is full_name0C. Existing actual clip
007B3C80, ragdoll007A7470, rest007B37C0→DD000787110→ShotMgr A0 and DD0+AA0
0074D510 selection remain preserved.184=5/188=NULL is normal reset;00761690 is
saved DD0 restore via00784310/005AB7A0, not a missing authored loader.

## Immediate next work

1. Continue normal non7 selected metadata/peer3C/lifecycle. **Inline MP mission
   is kind13**, native006B5620 at006B56AF; objectives are kind12.006B8700 resource
   goes to GameData38 (mission-relative24), name to80. Vtable00815AA8:
   24=006B83B0 finalize,28=006B5890 clear,3C=006B5960 append; use index first.
   Reused map/rules C remains in research worktree ignored
   `analysis/work/future-dependencies/p28-*`; consult before new exports.
   `human-objective-reused-originals` in MAIN already has006B8700/0072A0B0.
2. Ordinary00612A60 ECX peer slot differs from EDX Scene identity. Late3C/A8
   write, remote-AI auxiliary publication under418, and actual teardown must all
   be recovered together; do not publish a dangling borrower or invent a map.
   Existing MAIN C `offline-squad-storage-and-input/00612a60.c` and raw
   `prepared-human-model.asm` retain evidence. Existing non7 storage/spawn
   preconditions still reject; do not bypass them to manufacture a producer.
3. Close the full containing Human frame/World/failure/re-entry/retirement
   boundary. New focused audits use actual shared HumanPacketCommand. Old ignored
   `human-state-damage-native.cpp` predates PACK and is not current evidence.
   No permanent tests; remove temporary audits only at full checkbox completion.
   Other PACK item/action/typed-control/speech effects retain required failures.

## Verification and commands

WORK=`analysis/work/mission-world-20261103`. Latest ASan/UBSan/LSan PASS:
`human-authority-damage-native.log` (actual Player B0/input registry/codec/damage,
Base::bind queue boundary only; access checks disabled for private-input OOM
inspection), `human-reaction-retirement-native.log` (actual Look/selection/body,
component/engine/geometry doubles). Each includes100 entries/lifetimes.
Previous peer/console/formatter and objective/score/damage evidence stays linked.
Strict actual World/DD0 authority/presentation i386:
`human-reaction-containing-host-i386.log` PASS. Launcher AND startup-probe:
`human-reaction-containing-application-i386.log` PASS, compile/link only.
Outputs NOT executed. Tracker render/check and diff check required at checkpoint.

Client=`analysis/work/hd2.unpacked.exe`, SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
`python3 -B tools/research_index.py find ADDRESS...` before new exports;
`neighbors ADDRESS` has no `--count`. Candidates are not equivalence proof.
Latest read-only rollback exports `human-console-owner-abi` and
`human-console-window-abi`; logs complete without ERROR.0069F1E0's correct ABI
is no-argument/pointer-return, not the old thiscall(void*,text) decompiler shape.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
Inspect ERROR even on exit0; never overlap with heavy compilation.
