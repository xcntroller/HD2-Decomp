# Session handoff — 2026-11-09

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification, no repeated broad audits. Keep heavy
compiles/Ghidra sequential after repeated memory shortages/PC crashes.

GameApplicationState still declines loading. No live world/first rendered world
frame/new desktop acceptance. Ledger:34 complete/1,729 partial (1,724 client/
five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[StaticWeapon](analysis/static-weapon-loading.md) and
[receipt](analysis/checkpoints/p2.2-static-weapon-20261109.json): actual offline
mode7 control/item/barrel/marker/pivot/shell/collider/navigation, cold initial
update and retirement. Captured Tutorial Browning uses w_1919Crouch/item34,
actual runtime catalogue subtype3. Source SAME Items/entry/frame-data/navigation/
buckets. Native ASan/UBSan/LSan with84 frame failures and item/event/navigation/
factory/mode failure, both activation passes/repeat; strict actual-host i386/app
compile/link and27 tracker checks pass. Nine new partials; absent inventory and
cold callee null gates receive none. Four temporary artifacts hashed/removed.
No world/desktop promotion. Firing, occupied control, non7 input/network,
positive movement sounds and destroyed pose restore remain explicit receivers.
Do not repeat this verified research. Continue Car and containing owners.

Previous2ef03b2 [Animation/SupplyBox](analysis/animation-supply-loading.md): five/
seven captured records, actual helpers/clips/events/material/colliders, SAME
vertex registry/definitions and effects fades, cold first update/retirement.
Native sanitizers41/150 frame failures plus dependency/repeat cases, strict
host/app/tracker checks pass.20 partials.004737C0 exposed/corrected00473960:
padded definition normals4..6 → packed mesh3..5; mesh6/7 untouched. Actual
WorldActors REQUIRES module9's MissionVertexInstances reference; StartupSession
exposes SAME process definitions. Retire actors before registry/definitions/
frames. Initial Supply flags120=1 select crate-open6→0, SOUND group1/entry10.
Active Animation tracks/interaction/wings and Supply falling/respawn/pickup/
crossfade/positive fade-in remain explicit later receivers.

Previous6d5ae97 [Fowl](analysis/fowl-loading.md): captured count30 creates25
actual generated birds; mode1/2 hearing/proximity/packets/flight/retirement
verified,554 frame failures plus ground/listener. Mode0 still needs its real
borrowed pool. Previousf25acbd [Pike/Explosive](analysis/pike-explosive-loading.md):
stationary one/three unarmed records verified; motion/detonation excluded.
Door89d3d2f [contract](analysis/door-loading.md):25 captured rotating Doors/two
initial packets, dynamic-only edge query; sliding/protection49/4A excluded.

[Shared world](analysis/world-scene-ownership.md),c9c80fc: real scene/documents/
tables/environment/effects/tree/grid/Items/runtime/Physics around ONE actor/
frame-data/entry registry. Car callback factory receives these services before
binding. Tree stable allocation before bucket borrowers. Runtime before Physics;
ordinary actors before entries/models; containing Human/state/VM retire first.
Script/Players borrow actual process registries across repeat/startup/shutdown.
[Physics0..5](analysis/item-transfer-physics-loading.md) includes1/3/4/shared
5000ms fade-out;6 rejects unresolved unsafe second registration. Scene documents
retain SAME live archive scope for lightmaps. Missing check2 clears tables before
check.bin probe; legacy-present only opens/closes, no invented decoder.

## Next — same entire checkbox

Finish required Car activation and actual containing session/Human/DD0/FPV/HUD/
SOUND/VM/module owners, ordered006CA780 loader/postactivation, first camera/
update/render/failure/repeat and user acceptance. Existing GameWorldScene and
GameWorldActors are source compositions, still uncalled by application. Reuse
SAME services. GameSceneActors dispatch3/6/8/9/10/13/21/22/25/28/29/33; type3
still needs actual bound HumanLoadingActivation. Car activation exports already
under WORK/actor-activation. No new broad audit before implementation.

Do not bring every P2.3 simulation/VM opcode into P2.2. Full compiler/context/
visibility/rotations source verified; actual context factory/notifications and
derived command receivers remain.77/79 are unhandled original base cases;
position/animation snapshot masks reject,2/20/30 supported. Self ScriptAssign
rejects executing-context retirement. SetOn resolves scene before driver.

Actual mode7 is offline single-player. MissionA8 loop006989B0 skipsmode7. HUD
map kinds2/3/4 bind zone/goals, ordinary kind1 does not. Deferred global script
frame queue's observed producer is save restore004257B0/0042607B, not cold load.
Runtime actors35/11 source; placed flag1D/kind10 ground/effect rejects (all24
captured placed records flag1D0).145/146 BASE ISD0 Tutorial programs compile;
114 binding names have source. No runtime Patch/original execution claim.

Canonical: [world](analysis/mission-world-loading.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md),
[receivers](analysis/mission-script-receivers.md),
[sound](analysis/sound-world-loading.md).

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored durable, not /tmp).
StaticWeapon exports static-weapon-loading-required/ and
static-weapon-control-required/, actor-activation/006036d0, collider ASM, captured
actor/object fields and base model/member hashes/strings; no repeat exports.
GunShells is category1/model; payload+1C maps serializedbyte24. No repeat compiler/
portal/subtitle/Door/Pike/Explosive/Fowl/Animation/SupplyBox audit needed.
Git crash metadata repair preserved under WORK/git-crash-backup-20261109;
exact staged blobs restored, fsck passed, commits succeeded. No push.

Default sandbox normal commands; git/Ghidra config/native sanitizers need
escalation. One read-only Ghidra process/fresh output; check ERROR even exit0.
Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64 (verified; do not reinstall).
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
