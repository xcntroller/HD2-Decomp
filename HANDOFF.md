# P22-HUM H6 handoff — 2026-11-12

Active worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h6`, branch
`feature/p22-hum-h6`, created from completed H2 `85593e8`. H2 worktree/branch is
preserved. Check HEAD/status before acting; preserve unrelated changes. H4 stays
separate; H8 unassigned. Read AGENTS.md, section2 of plan.md,
[Human plan](analysis/p22-hum-plan.md), [H6 plan](analysis/p22-hum-h6-plan.md).

**H6 remains OPEN.** Latest user priority: finish P22-HUM and get in game; focus
on the offline path rather than expanding currently unneeded research. The user
retracted the accidental commit/no-commit messages and authorizes meaningful
checkpoint commits. No push/subagents/original EXE/DLL/Proton/Windows execution.
Heavy compilation and Ghidra sequential. No permanent tests or desktop claim.

## Latest verified controller checkpoint

[Contract](analysis/human-player-item-controller-source.md),
[receipt](analysis/checkpoints/p2.2-human-player-item-controller-20261112.json).
Held00542E20, basis00544D20/spread00545140/recoil005454D0, Player08/00542920
local effects and SAME input Record pending production, empty-weapon007D4E90/
005433B0, and positive firing coordinator source are implemented. Existing
005433B0/factory dispatch invokes these bodies. Positive item24/28 still requires
its real projectile receiver; no successful firing service was installed.

Native100 ASan/UBSan/LSan PASS on actual Player/Human/input manager/Record/
RuntimeItem/holder owners, including failure/re-entry and balanced retained
frames. Inline/Look, FPV, camera/engine observations, first-shot callback and
item24 projectile effects are explicit doubles. Full actual owner graph is not
claimed. Public state/input/catalogue data are authored; no private-field override.
Strict actual nested-host i386 and latest launcher/probe rebuild/link PASS.
Five other Runtime declarations always throw, compile-only. New temporary audit
sources/binary/object are fingerprinted/removed, logs and dedicated Windows
snapshots retained. Existing frozen inputs unchanged. Five new frozen partials,
two extensions, no complete promotions: **34 complete/3254 partial (3249client+5engine)**.

Important contracts: held deadlines are hand HOLDER18 via007D68B0, not item14
or DD0. Recoil publishes each B0+10 component before the next draw. Local command
mutations precede pending publication; missing Record fails after them. Original
007D45B0 proves process0098AFE0 initialization via004374D0; Startup now owns the
same shared item request across worlds, including reload/empty writers.

## Latest sound/action checkpoint and next work

[Sound/action contract](analysis/human-player-item-sound-actions-source.md),
[receipt](analysis/checkpoints/p2.2-human-player-item-sound-actions-20261112.json).
Actual RuntimeItem SOUND owners007D5C70, local/authority007D5A80 and following
007D6330 are implemented, plus melee00542B80, loaded-World repair query and
PlayerD8/0053D340 first-queue producer. Native100 sanitizers, strict actual nested
host i386 and launcher/probe rebuild PASS. Explicit engine/query/Look/Human doubles
are documented; full positive owner graph is not claimed. New temporary audits
removed after fingerprints; frozen inputs retained. Four new partials, no complete
promotions:34 complete/3254 partial (3249client+5engine). Native SOUND refill leak
is retained through source item retirement and then reclaimed, explicitly limited.

Continue shared actual item24/28 `007D4FD0/007D5210`, reached projectile
producer/retirement on SAME process ShotMgr. Existing C candidates in
frame-human-shot-and-control-workers, frame-human-live-inventory-and-aim,
frame-human-shot-projectile-workers and frame-human-shot-bullet-lifetime;
no broad new exports. `human-h6-projectile-base-index.txt` records boundary lookup.
Ordinary offline unmounted00450D30 initializes/publishes a bullet without its
conditional100ms frame. Attached/non7 Player does invoke it. Actual ShotMgr
RuntimeRecord is still only forward-declared; never install successful fire or
opaque pointer/pool substitutes. Real body004482A0 initializes shooter/pose/
direction/payload/damage,0044B030 scales velocity/configures visual;0044B910 is the
large reached asynchronous damage/collision consumer. Shell0044E5D0/visual and
flash00453D80 lifetimes also selected in mode2. ShotMgr full World scheduling is
H8, but producer ownership/retirement must be real. Kind9 grenade and type2
projectile remain selected dispatcher branches. Do not absorb active H4.

Existing static exports and original assembly are in WORK with `human-h6-`
prefixes. Successful new005441E0/005AC550/007D5C70 C exports have initially
unknown signatures. Full non7 history/replay/DirectPlay admission is conditional
D/OWN/H8/Phase3 work, not an offline H6 detour. Player08 pending production is
implemented; it does not imply transport/readiness. Preserve all required guards.

Remaining D work still matters: D4/D5 containing Car integration, D6–D9 OWN/
entry composition and D11 part5F9/wheel695/EXPL8/smoke14 native omissions. H4/H6
supply particular producers/consumers; their completion does not automatically
finish those D entries. H8 must account for required selected paths or exact
blockers. [Ownership table](analysis/p22-hum-h6-plan.md#remaining-d-ownership).

## Evidence and commands

WORK=`analysis/work/mission-world-20261103`; independent copy/reflink of H2's
ignored evidence is present. `.tools` links share installed compiler/runtime.
Original SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Use `python3 -B tools/research_index.py find ADDRESS...` BEFORE new exports.
Keep new output names; never overwrite frozen export directories. Ghidra is
`../H&D2 Decomp 4/ghidra/support/analyzeHeadless`, project
`/home/control/Documents/Projects/HD2ResearchProject HD2Research`, process
hd2.unpacked.exe -readOnly -noanalysis; JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`; rollback changes.
Check ERROR even on exit0. Bounded objdump-d/PE reads only; never original execution.

Native g++17/O0/warnings-as-errors/pthread/ASan/UBSan/frame pointers; runtime
`.tools/hd2-sanitizer-runtime/usr/lib64`. i386 compiler:
`.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.
H2 remains source-closed with its [contract](analysis/human-h2-source-closure.md);
full application/World/frame/render/user desktop acceptance remains P2.2.
