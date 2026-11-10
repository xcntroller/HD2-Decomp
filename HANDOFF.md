# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy builds/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,867 partial
(1,862 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[HUD Human/player presentation](analysis/hud-player-presentation-loading.md),
[receipt](analysis/checkpoints/p2.2-hud-player-presentation-20261110.json).
Actual soldier bind/name/rank/photo/caption/AI textures and inventory identity,
real compass OBJ_ markers and add/remove/hide ownership; source Players control
→concrete presentation→HUD duplicate/add/remove and collected-frame cleanup.
Focused native sanitizers, strict actual-host HUD/FPV/Players/shared-presentation/
player-close composition, app link and27 tracker tests pass. FPV/engine/frame/
item/localization/clock boundaries doubled in native; remaining DD0/equipment/
containing composition/SOUND/cursor/world/inventory-refresh interfaces declarations.
15 new partials/two extended, no completes.00538590/B0 absent inventory: no credit.
Four temporary artifacts hashed/removed. No application/desktop readiness claim.

005A81C0 offline user14 else LAST04 or actual modelE8, not005A82C0 short10.
005A8380 uses profile.field24. Active3 HUD first/second virtual13C/140 target
00538590/B0: pending1CC/1D4 unless-1, else current1C8/1D0. Texture caches store
before frame calls, equality suppresses retry.005A78F0 finds DEFINITION ID, unlike
005A6FF0 runtime identity. Rank table first IDs27A6..AD already confirmed in
analysis/human-store.md; raw BSS0 is not an initialized rank table.
0049B870 hides then reverse-scans roots; any visible root skips marker removal.
Last hides clear4 then retire newest matching marker amount, without erase.
0049B900 marks each Human before reverse hide, then flags&~6.004936C0 creates
real OBJ_, local pose/radius/update/A8alpha0, amount1; difficulty4 blocks append.
Native duplicate marker behavior preserved. HUD Actor alias now decays actual
array/vector element reference before removing pointer; no second identity store.

Previousd6b81e9 [FPV/player owners](analysis/first-person-loading-composition.md):
actual same-overlay model/items/animation SOUND/playback/view/reset, actual
current-Player/WorldActors/Scene2/inline/B0/sector/retirement lookups, mode005469E0
and typed Look04/scalar48 actions. Application+0C/008AE908 byte0 preference is
separate from profile and FPV58. Source model/FPV composition/reset compile-only.
Previous5cf27a0 [script runtime/objectives](analysis/mission-script-runtime-composition.md):
VM/module, objective88, ReturnEnd/EndScript and camera look.006E9EA0 tables use896
CRT draws; stage13/41 WORD indices select DWORDs. Actual observed publication
required; never force equality. Startup tables stay uninitialized. Original-address
integrity VM excluded from accepted startup scope.008692D0 normallyfalse, actual
advance=override OR game28; reset00424900 is its confirmed scoped writer.
Module loaded10 can latch on bad binding; inspect failed_bindings in real loader.
Previousad54f4c [initial controls](analysis/mission-script-initial-controls.md):
contextEC=events[4], Guard queues, Human controls/suspend, Sight/SOUND and views.
Previous1b703b8 [Human ownership](analysis/human-world-composition.md) composes
DD0/E0/230/B0/inventory/activation;99c8688 recovers cold Car activation.

## Next — same entire checkbox

Actual0049FBF0 current inventory refresh and remaining reached HUD updates,
containing Human/inline/FPV/HUD/SOUND/VM/world/session ownership, required
observed-value producer, ordered006CA780 loader/postactivation, first camera/
update/render and failure/repeat, then user desktop acceptance. WorldScene/
WorldActors remain uncalled by application. Consumers.make_human_loading_owner,
presentation and VM consumers must use real containing dependencies. Retire
Human/B0/VM before world actors, before entries/models/tree/session. Do not
expand all P2.3 simulation/opcodes into P2.2.

Static census:145/146 captured programs compile;114/114 initial binding names
compile. First synchronous Block reaches five Human controls, face texture,
three camera commands, tactical and objective status (now source supplied).
NPC1..4 CreateItem scripts are later assignments, not initial114. Actual overlays/
resolver authoritative. Captures script-start-census.json, script-human-command-
labels.json/base jump map and human-suspend-vtable-targets.json remain in WORK.
Human float switch labels decode bits against COMMANDS only, not predicates.

Next concrete presentation dependency0049FBF0 has no source receiver yet;
HumanLoadingPresentation requires explicit Dependencies.inventory_changed().
Car004D9280 and other nonoffline virtual64 remain deliberate rejections; no
successful callback default exists. Source bridge permits completed cyclic
owner types but does not construct the actual dependency aggregate or world.
Detailed new HUD contracts/captures are linked above; no further broad audit of
unchanged source needed. All Ghidra jobs completed cleanly.

Canonical contracts: [world](analysis/mission-world-loading.md),
[shared ownership](analysis/world-scene-ownership.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md),
[receivers](analysis/mission-script-receivers.md),
[sound](analysis/sound-world-loading.md).

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored durable, not /tmp).
Client SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Default sandbox normal reads/edits; git/Ghidra/native sanitizers need escalation.
One read-only Ghidra process/fresh output; check ERROR even exit0.
Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Ghidra project:/home/control/Documents/Projects/HD2ResearchProject HD2Research.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64; do not reinstall.
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
Tracker:python3 tools/function_progress.py --render / --check;
python3 tools/test_function_progress.py (not unittest package mode).
