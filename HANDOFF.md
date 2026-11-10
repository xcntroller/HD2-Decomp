# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
agents/goals, original EXE/DLL/Proton or Windows fixture execution. Preserve
assets/user changes. Heavy builds/Ghidra SEQUENTIAL (memory crashes), focused
checks, no permanent tests. Interactive changes need user desktop acceptance;
source-only work needs no invented desktop test.

Application still DECLINES loading; no loaded world/first frame/desktop result.
Ledger34 complete/1,949 partial(1,944 client/five engine)/18,783 frozen.
Zero complete promotions. Both P2.2 checkboxes remain OPEN.

## Latest source checkpoint

[HUD world/parent binding](analysis/hud-world-binding.md),
[receipt](analysis/checkpoints/p2.2-hud-world-binding-20261110.json).
Actual GameHudLoading/HudPanels/HudScene bind/close now composes with SAME Scene2
identity view, session.mission.related objective view and parent navigation tables.
HUD needs full reserved32/holes/current Scene2 count, not its selection pair.
Common world retirement uses SAME ActorBase pointer, not outer SceneActor.
Map captures missionA8 once per pass, reads its vector count/entries live.
0049B280 passes SAME inlineHUD6B320 pointer to map/inventory/command; source now
uses auxiliary.pointer(), removing external invented opaque Borrowers.
004B4980 command5A7B4 reads game54, which00698510 gets from00523AB0: NAVIGATION
TABLES, not collision tree/grid. Source world facade borrows actual owner.tables().
Strict graph now uses GameOwnerBackend template parameter matching SessionOwner's
actual MissionTables<GameOwnerBackend>, not unrelated TableStorage instantiation.
Players subgroup is pure required interface; no loading append producer or fake
successful tick/destroy. Type37 zone/groupA4 still requires its actual owner.
Native focused views/Base retirement, strict REAL complete HUD bind and prior
world/Car/Human/VM/router/spawn graph, app link and27 tracker checks PASS.
No new credit. Completed audits hashed/removed; active ignored next fixture
WORK/checkpoint-containing-session-host.cpp. Actual containing dependency
aggregate, ordered loader and first frame remain UNCALLED by application.

Prior7f55042 [cold Car binding](analysis/car-callback-world-binding.md) supplies
actual factory/body/solver/Physics contact link/emitter pool. Tyres derive current
IDs; activation rejects different tyre/pool identities.6A8 word4 is Physics2
00410630→00410530, NOT Script. Same270/280/410/210/918/220/22C0 force/impulse
receivers, initial model160 versus moving240, temporary heavy-contact mass restore.
Human58 preserves sixth FRAME before appending null seventh. GEffects+B4 is
00485BA0 mesh fragments, NOT opacity fade. Positive Human/tipping/Versions/
Item12/non7/blast/effect paths explicitly reject. Five partials added there.

[P24-F05 squad authority](analysis/live-squad-session-ownership.md): advisory read
via git show research/future-dependencies at4332281; no merge/worktree changes.
SessionOwner REQUIRES mutable Startup.pre_mission().squad() borrower; GameApp
live_squad returns SAME. Request squad is immutable metadata. Real splits/identity/
voice mutations survive teardown/repeat/failure in application records. No P2.4
UI/stock/mask/voice-producer/reconciliation/save/result changes; F01/F02/F03/F06
remain later, F04 head1/uniform2 unchanged. No extra random fallback.

Prior dcbef57 [world consumers](analysis/world-actor-loading-consumers.md),
21b5c7c [module dispatch](analysis/module-loading-dispatch.md)/
[18 vtables](analysis/module-lifecycle-map.json),6021978
[GEffects](analysis/geffects-module-loading.md),4af20a3
[Versions](analysis/version-module-loading.md),c97d95b
[lifetimes](analysis/module-loading-lifetimes.md),52c5689
[session consumers](analysis/session-loading-consumers.md)/[ShotMgr](analysis/shot-manager-loading.md).
Other Human/VM/FPV/HUD/Players contracts in RESEARCH.md/loading-modules.md.

## Next — SAME full checkbox

Actual containing world/session instances and reached required positive receivers,
ordered006CA780 loader, first camera/update/render, failure/repeat then user
Python desktop acceptance. All recovered loading world/VM/router remain UNCALLED
by application. The strict fixture supplies actual Car factory and complete HUD bind now, with
real parent tables/identity/objective/pointer views; dependency aggregates are
still borrowed. Next construct actual containing graph, resolving cycles with
checked SAME-owner borrowers, never references to unconstructed objects or
successful defaults. WorldActorLoadingConsumers/HumanWorldLoadingServices/
HumanLoadingPresentation/HumanLoadingOwnerFactory currently store Dependencies&;
containing cycles require checked late dependency binding before activation.
Preserve actual instance identities and original teardown order. Continue after
this dependency checkpoint.

Ordering: +14/+1C/+18 fresh ASCENDING;0048DB90/+20 fresh REVERSE.00698F80 does
presentation finish→dubbing close→authority finish→HUD close. Reserved Humans can
die before Script4 Exit; ActorMissionScriptContext checks retained-frame EC0 before
dereference. Do NOT move VM finish ahead of native module order as a substitute.
Final aggregate retracts remaining Human/VM/FPV/camera/audio borrowers before scene.
Positive Exit requiring retired B0 remains required. Same EC/roster/grid/cache/
pools/process CRT/mode/config/registrations; Human Frames=GameActorServices.
Script bank persists; standalone kind1/campaign bank selection after contexts begin.
Fresh B0 clears74/78; previous-null Area8 skipsB4. Positive stance/area/Dog534/
nonoffline friendship and mode0 Fowl/grass/positive particle producers excluded.

Integrity tables still UNINITIALIZED; private observed publication unresolved.
006E9EA0 initializes128 entries with896 SAME CRT draws, WORD indices/DWORD values.
API payload order00698AAE ticks,00698C09 visible,00698C5D capture supplied. Never
put raw API values in observed slots or force equality. Original-address/thread
VM outside accepted startup scope; meaningful actual observations still required.
Live overlay resolver authoritative: BASE ISD0 captures never prove ISD1/Patch/
language precedence/absence. Initial114 bindings/Block/faces/camera/tactical/
objectives supplied; later NPC CreateItem not initial prerequisite.

## Execution

WORK=analysis/work/mission-world-20261103 (ignored durable, not/tmp).
Client SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Native libs .tools/hd2-sanitizer-runtime/usr/lib64; do not reinstall.
i386 .tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Ghidra JAVA_HOME=/home/control/.sdkman/candidates/java/25.0.0.0-oracle;
project /home/control/Documents/Projects/HD2ResearchProject HD2Research.
Read-only/noanalysis/fresh output. Check ERROR even exit0. Check frozen entries
FIRST; absent confirmed entries use static instructions/transient definition.
Default reads/edits; git/Ghidra/native run escalation. Tracker python3
tools/function_progress.py --render/--check; python3 tools/test_function_progress.py.
