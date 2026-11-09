# Session handoff — 2026-11-09

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until the checkbox is done or
concretely blocked. Larger verified subsystem commits; consolidate docs/tally.
No push, subagents/goals, original game/DLL/Proton or Windows fixture execution.
Preserve assets/user changes; focused verification, no repeated broad audits.
User reopened VSCode after memory shortage: keep heavy compiles sequential.

GameApplicationState still declines loading. No live world, first world frame
or new desktop acceptance. Ledger:34 complete/1,674 partial (1,669 client/five
engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Shared scene/actor ownership](analysis/world-scene-ownership.md) now composes
actual Scene2/documents/tables/environment/effects/tree/grid and Items/runtime/
Physics using ONE GameActorServices/frame-data/entry registry. Required Car
factory receives those actual services, then binds callbacks before parsing.
Collision tree is allocated in its final stable location before buckets borrow
its dynamic grid. Runtime actors retire before borrowed Physics descriptors;
ordinary actors before entries/model caches. Containing Human/state/script
owners must finish/retire before this owner closes. Production consumers remain
required; no default successful Car/item/sound/presentation service is added.

File wrappers006CBF30/006D45C0/006D4880/006D4280 now retain the existing live
archive document scope for embedded light maps, shared name map/version and
session navigation datasets. Missing/short Scene2 returns3/1; other full root
returns0 as original. Optional actor/sound absence is ignored. Missing check2
clears tables before check.bin probe; legacy-present only opens/closes, no
legacy decoder. Original assembly confirms this branch.

[Physics continuation](analysis/item-transfer-physics-loading.md#physics134-and-shared-fade-continuation--2026-11-09):
types0..5 supported, type6 still rejects. Type1 scales geometry radius for its
descriptor while retaining selected170; type3 uses native principal-axis/tie/
precision rules; type4 has no collision registration and uses shared retained
5000ms fade-out. Out-only GEffects200 queue/update/frame+A8 opacity recovered;
opposite fade-in/cancellation excluded. Do not invent a safe type6 second
registration: saved pointer overwrite conflicts with native descriptor230 cleanup.

Script/Players can borrow actual StartupSession process module registrations,
leaving registry identity intact through repeated world lifetimes/startup/shutdown.
[Receipt](analysis/checkpoints/p2.2-world-owners-20261109.json):nine new partials,
three extended; focused native ASan/UBSan/LSan, strict actual-host i386 scene/
Items/runtime/Physics composition, app compile/link and27 tracker tests pass.
Twelve temporary source/bin/objects hashed/removed. Native archive/scene/parser/
engine endpoints are doubles; actual host owner is compile-only. No world or
new desktop delivery. Earlier committed presentation checkpoint:e1d6e28.

## Next — same entire checkbox

Continue actual containing session/Human/DD0/FPV/HUD/SOUND/VM and required
Car/module/derived actor activation, then complete ordered loader/first-frame
update/render/failure/repeat/user acceptance. Existing GameWorldScene and
GameWorldActors are source composition only, uncalled by GameApplicationState.
Use them rather than create another frame-data/bucket/item service. Next actual
required derived families include Door00600CC0, Animation005B3BE0, SupplyBox,
Pike, ExplosivePlace, FowlGenerator, StaticWeapon and Car; original activation
exports already exist under WORK/actor-activation. Current GameSceneActors
activation dispatch still supports3/8/10/13/33 only. Human activation needs its
actual bound HumanLoadingActivation lifetime before type3 dispatch.

Do not bring all P2.3 simulation/VM opcodes forward merely for P2.2. Research
required first-frame receivers. Full VM compiler/context/visibility/rotations
source already verified; original switch is two-stage (script-base-vm-jump-map).
Actual base/Human/Dog context factory/notifications and derived commands remain.
FRM_Move77/GetPosition79 are unhandled base cases, not successful stubs.
Position/animation snapshot masks reject;2/20/30 supported. Self ScriptAssign
rejects executing-context retirement. SetOn decoder resolves scene before driver.

MissionA8 loop in006989B0 is skipped in mode7. HUD map kinds2/3/4 bind zone/goals,
ordinary kind1 does not: recover actual reached consumers before treating zone37
or A8 as initial Tutorial requirements. Deferred global script frame queue's
observed producer is save restore004257B0/0042607B, not cold mission loading.
GameRuntimeActors supports35/11; flag1D/kind10 placed ground/effect still rejects
(all24 captured placed records flag1D0). Unsupported required input cannot pass.

Canonical: [world](analysis/mission-world-loading.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md), [receivers](analysis/mission-script-receivers.md),
[sound/subtitles](analysis/sound-world-loading.md).

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored durable, not /tmp).
No pending process. Do not redo confirmed compiler/portal/subtitle research.
145/146 BASE ISD0 Tutorial programs compile;114 base binding names all have
source. This is not runtime Patch selection or original execution evidence.

Default sandbox for normal commands; git mutations/Ghidra config/native
sanitizers need escalation. One Ghidra process, read-only project, fresh output;
check ERROR even on exit0. Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64 (verified; do not reinstall).
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
