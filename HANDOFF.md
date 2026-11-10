# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy compiles/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,807 partial
(1,802 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Car cold activation](analysis/car-activation-loading.md),
[receipt](analysis/checkpoints/p2.2-car-activation-20261110.json): actual offline
004D16A0/004D25C0/004D2790/004D3110 composition and source type4 Scene2 dispatch.
SAME geometry/initializer/solver/BODYCOL/finalizer/navigation/DM/seats/camera/
process receivers; native repeat and53 activation/teardown failures pass
ASan/UBSan/LSan with nine captured parameter sets and explicit synthetic
four-wheel model/engine/math/session/callback doubles. Actual host/world dispatch
is compile-only. Part/glass/mass/audio/damage, nine-set live alternating wheel
cache and shared water/support/proximity checks pass. Strict i386, app link and
27 tracker tests pass.24 new partials, none complete. Temporary audits hashed/
removed; captures/exports/logs remain. No application readiness changed.

Important corrections/limits are in the contract; do not repeat verified research:
actor624/core594 is SAME lighting list; actor4A0/core410 is velocity, actor958/
core8C8 speed. Interior impact bands C38..C44/22AD..22B0, not punctureC34.
Actual actor overrides wheel534=98302 and BODYCOL53C=D8002, flags0; F02/2 are
pre-actor initializer defaults. DM ctor countdown-1 is reset to0 before cold
update. Water uses SECOND/max006FDA00 output/-FLOATMAX; reuse existing decoder.
Native descriptor addresses remain stable across reactivation using inline
optional initializer storage. Per-wheel20000 alternating caches are implemented;
front/rear plane-cache mode remains excluded. Positive running/occupied/gun-seat/
deformation/skid/emitter/non7/queued SOUND receivers remain explicit errors.

Car callback GameCarCallbackFrames now exposes SAME runtime state through
actor.loading; remaining pure module/Human/effect/script receivers need actual
containing owners. GameWorldActors.activate_car requires actual Consumers
car_loading_resources (tyres/materials/process sound+light lists/pool/seat-camera/
rand), actor_proximity(), current attachment/settings264/game kind. StartupSession
exposes SAME process seat camera/lists/settings; real proximity bridge now has
remove_actor. Do not supply successful placeholders. Car cameras/geometry retain
resources until their actual ordered retirement.

## Next — same entire checkbox

Finish actual containing Human/DD0/state/FPV/HUD/SOUND/VM/module/session ownership,
ordered006CA780 loader/postactivation, first camera/update/render/failure/repeat
and user desktop acceptance. WorldScene/WorldActors still uncalled by application.
HumanSceneActor.loading_lifetime is a borrowed endpoint, still needs actual bound
HumanLoadingActivation. Existing HumanInlineLoading, HumanControlLoading,
HumanEquipmentServices/Loading, HumanLoadingStateFactory/Owner, inventory receiver,
Sight/SOUND/proximity, Players and camera sources must share real dependencies.
Retire Human/B0/VM owners before world actors, before entries/models/tree/session.
Do not bring all P2.3 simulation/VM opcodes into P2.2.

Canonical contracts: [world](analysis/mission-world-loading.md),
[shared ownership](analysis/world-scene-ownership.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md),
[receivers](analysis/mission-script-receivers.md),
[sound](analysis/sound-world-loading.md).

Previous verified actor checkpoints: Car cold runtime7acfb09, StaticWeapon1009ef3,
Animation/SupplyBox2ef03b2, Fowl6d5ae97, Pike/Explosivef25acbd, Door89d3d2f,
shared worldc9c80fc. Use their analysis contracts; no repeated broad audit.
Mode7 is offline. Script77/79 are original unhandled base cases; unsupported
snapshot masks still reject. Scene2 first header statuses/order, live lightmap
scope and legacy check.bin close-only behavior are confirmed. Actual145/146
Tutorial programs compile;114 binding names have source; context/notifications/
derived runtime commands still need integration.

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored durable, not /tmp). Latest
exports listed in Car activation contract. Use car-runtime-typed-v4-required/
004ea780.c (correct three-arg SceneEC;004290A0 THISCALL/six stack args/RET24),
not older v2/v3 initializer. v2/0050de60.c remains correct for core. Original
client SHA8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Seat export job produced00466010 then errored on undefined004DA090: do not claim
whole-job success; existing4DA090 export lives in car-activation-required.

Default sandbox normal commands; git/Ghidra config/native sanitizers need
escalation. One read-only Ghidra process/fresh output; check ERROR even exit0.
Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Ghidra project:/home/control/Documents/Projects/HD2ResearchProject HD2Research.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64; do not reinstall.
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
