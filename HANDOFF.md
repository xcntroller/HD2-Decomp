# Session handoff — 2026-11-09

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until the checkbox is done or
concretely blocked. Larger verified subsystem commits; consolidate docs/tally.
No push, subagents/goals, original game/DLL/Proton or Windows fixture execution.
Preserve assets/user changes; focused verification, no repeated broad audits.
User reopened VSCode after memory shortage: keep heavy compiles sequential.

GameApplicationState still declines loading. No live world, first world frame
or new desktop acceptance. Ledger:34 complete/1,685 partial (1,680 client/five
engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Door loading/first update](analysis/door-loading.md) now supplies ordinary
rotating Door6 activation/portal/descriptor lifetime, local presentation packet
producer/receiver, handle and Door motion, blocked-closing rollback/cursor and
actual dynamic-only edge probes. All25 captured Tutorial Door records rotate;
two start open and produce actual pending packets. Scene2 activation dispatch
now includes6; GameWorldActors exposes actual Door update/packet consumers.

Door query006F78F0/006F3250 is NOT the camera ray: dynamic-only, one-cell range
expansion both ends, native Z/X/bucket order, different broad phase and inside/
zero-length sphere handling. Known49/4A requires real receivers and rejects.
Sliding Door and ordinal>12 campaign protection guard remain unsupported. The
receiver00601F90 is absent from frozen inventory: implemented, no new credit.

[Receipt](analysis/checkpoints/p2.2-door-loading-20261109.json):eleven new
partials; native ASan/UBSan/LSan, strict actual-host i386 Door/World composition,
app compile/link and27 tracker tests pass. Four temporary artifacts hashed/
removed. Engine geometry/frames, script/localization/audio/lock endpoints are
boundary doubles; host/world remains compile-only. No desktop promotion.
All raw exports/ASM remain in WORK; do not repeat the confirmed Door research.

Previous committed checkpoint c9c80fc:
[shared scene/actor ownership](analysis/world-scene-ownership.md) composes actual
Scene2/documents/tables/environment/effects/tree/grid and Items/runtime/Physics
using ONE GameActorServices/frame-data/entry registry. Actual Car callback
factory receives those services before binding. Tree reaches its final stable
allocation before buckets borrow it. Runtime actors retire before Physics;
ordinary actors before entries/models; containing Human/state/script owners
must finish/retire first. Script/Players borrow process module registrations,
preserving registry identity through repeat/startup/shutdown. Required consumer
services remain real dependencies, never successful defaults.

[Physics0..5](analysis/item-transfer-physics-loading.md) includes1/3/4 and actual
retained5000ms fade-out. Type4 reads cached frame+3C FLOAT after bounds update;
legacy helper name says inverse but byte receiver is correct. Type6 still
rejects unresolved second registration; opposite fade-in/cancellation excluded.
File wrappers keep SAME live archive document scope for embedded light maps,
names/version and session tables. Missing check2 clears tables before probing
check.bin; legacy-present only opens/closes (no legacy decoder). These owner
compositions are uncalled by GameApplicationState.

## Next — same entire checkbox

Continue actual containing session/Human/DD0/FPV/HUD/SOUND/VM and required
Car/module/derived actor activation, then complete ordered loader/first-frame
update/render/failure/repeat/user acceptance. Existing GameWorldScene and
GameWorldActors are source composition only, uncalled by GameApplicationState.
Use them rather than create another frame-data/bucket/item service. Next actual
required derived families include Animation005B3BE0, SupplyBox005FAC80,
Pike005B9670, ExplosivePlace0060C3B0, FowlGenerator005BA670, StaticWeapon and
Car; original activation exports already exist under WORK/actor-activation.
Current GameSceneActors activation dispatch supports3/6/8/10/13/33 only. Human activation needs its
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
