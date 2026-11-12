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
two extensions, no complete promotions: **34 complete/3266 partial (3261client+5engine)**.

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
promotions:34 complete/3266 partial (3261client+5engine). Native SOUND refill leak
is retained through source item retirement and then reclaimed, explicitly limited.

## Latest bullet production checkpoint and next work

[Weapon/bullet contract](analysis/human-item-bullet-production-source.md),
[receipt](analysis/checkpoints/p2.2-human-item-bullet-production-20261112.json).
Actual007D4FD0 dispatch,00451100 marker selection, offline00450D30,004482A0/
0044B030 record initialization, SAME ShotMgr pool/active114 ownership/retirement
and B0 muzzle light/retained flash lifetimes are implemented. Player fire now
uses actual World item receiver. Native100 sanitizers, strict real host i386 and
latest launcher/probe rebuild PASS. Explicit engine/item/effect/frame doubles,
non7 and selected projectile guards are documented; no combat/World/desktop claim.
New audit sources/native/object removed after fingerprints. Twelve new partials,
no complete promotions:34 complete/3266 partial (3261client+5engine).

Continue remaining offline H6 **producers/retirement**: shell0044E5D0/
00454190/004546C0 using SAME pool64/transient134; capsule00451670/00448CE0;
grenade00450400/0044CFF0 with existing rigid owner; immediate melee00451230
through existing H2 damage. Do not expand full async combat/World assembly or
non7 replay as an H6 detour:0044B910/0044B400/00454360 are H8, with explicit
required guards including attached initial100. No successful simulation stubs.
Exact-frontier C exports shell/capsule/falloff now exist in
`human-h6-shot-record-dependencies` (all five true, no ERROR), indexed before
export; unknown ABI still needs actual caller/raw confirmation. Existing
0044E5D0 C has stack aliases; inspect bounded raw where needed. Projectile
getter007D46B0 returns marker74, NOT auxiliary clone78. Native6C tracer uses
signed IDIV. Actual neighboring light threshold is16; old-time flash hides>50.
Keep active H4 separate and preserve module17/Scene/borrower retirement order.

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
