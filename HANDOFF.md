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

[Cold Car/world callback binding](analysis/car-callback-world-binding.md),
[receipt](analysis/checkpoints/p2.2-car-callback-world-binding-20261110.json).
Concrete GameCarCallbacks/factory now borrow SAME Startup/mode/local/clock/CRT,
frame/EC/Scene2, actual Car body/solver/health/wheels and owned emitter pool.
Tyre IDs derive from CURRENT CarTyres; activation rejects other tyre/pool owners.
Checked PhysicsContactLink breaks construction cycle; WorldActors binds its OWN
scene's Car callbacks and unbinds before Physics retirement. Real force004F2A30,
impulse004F2B60 and offline004D3EF0 target SAME270/280/410/210/918/220/22C0.
Saved initial model160 position is independent of moving240. Heavy/destroyed
contact temporarily halves SAME mass, scales impulse1.4 and restores on failure.

Concrete corrections:004D4A70/6A8 word4 is PHYSICS handle, NOT Script.
00410630→module2 vtable80F640+4C→00410530. It migrates second→first list and
repairs moved handles BEFORE derived contact.6A8 callback is now contact6a8.
Human58 preserves SIXTH FRAME pointer before appending null seventh to5C;
old bool signature lost it. NonHuman5C00611070 genuineRET, Human nullB0 genuine
skip. GEffects+B4 is00485BA0 mesh fragments→00484F60, NOT opacity fade. Positive
mesh fragments/type5 tipping/queued pickup/Human hits/Versions changes/detached
Item12/non7 event/blast/effect producers explicitly reject; no fake success.

[P24-F05 squad authority](analysis/live-squad-session-ownership.md): user advisory
was read via git show research/future-dependencies at4332281, no merge/worktree
changes. SessionOwner now REQUIRES mutable Startup.pre_mission().squad() borrower;
GameApplicationState.live_squad returns SAME. Request squad is immutable handoff
metadata, never spawn authority. Real stack splits/identity/voice mutations survive
teardown/repeat/failure in persistent application records. No P2.4 UI/stock/mask/
voice-producer/reconciliation/save/result changes. F01/F02/F03/F06 remain later;
F04 working head1/uniform2 unchanged. No extra random fallback.

Focused native ASan/UBSan/LSan: real list migration/handle repair/failure, actual
force/impulse/rotated inertia/live mass/callback order/prefix/initial pose,6A8
routing/link lifetime and actual SessionOwner/stack split alias all PASS.
Strict i386 actual concrete Car factory/WorldScene/WorldActors/Physics bind/
shared resource guard and prior Human/VM/router/spawn graph PASS; app link and
27 tracker checks PASS. Five partials added. Completed audits hashed/removed;
active next ignored fixture WORK/checkpoint-containing-world-host.cpp.

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
by application. The strict fixture supplies actual Car factory now, but still has
DECLARED HGroup and HHudWorld and borrowed dependency aggregates. Next construct
actual containing graph; resolve cycles with checked SAME-owner borrowers, never
references to unconstructed objects or successful default endpoints. HUD map bind
needs actual scene_frame/objective/Scene2 registry views; its template was not yet
instantiated by prior graph. Players subgroup has no append producer; don't invent
successful tick/destroy services. Continue after this dependency checkpoint.

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
