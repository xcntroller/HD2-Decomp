# Session handoff — 2026-11-10

Continue FULL P2.2 first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until done or concretely blocked.
Larger verified subsystem commits; consolidate docs/tally there. No push,
subagents/goals, original game/DLL/Proton or Windows fixture execution. Preserve
assets/user changes; focused verification only. Heavy compiles/Ghidra sequential
because of memory shortages/crashes. No permanent tests.

GameApplicationState still declines loading. No loaded world, first rendered
world frame or new desktop acceptance. Ledger34 complete/1,811 partial
(1,806 client/five engine)/18,783 frozen. Zero complete promotions.

## Latest verified source checkpoint

[Human/world composition](analysis/human-world-composition.md),
[receipt](analysis/checkpoints/p2.2-human-world-composition-20261110.json): source
HumanInlineLoadingFactory builds actual DD0/E0/230; HumanLoadingComposition owns
SAME factory/inline/B0 factory+owner/inventory/activation. WorldActors owns typed
Human lifetimes, type3 dispatch and retirement before Scene2 identity deletion.
Shared SOUND routes each Human through its own factory; B0 close failure now
releases both retained states before inline retirement. Native focused ownership/
script-selection/sound identity and actual actor/context/compiler/EC/binding
checks pass ASan/UBSan/LSan, including bad/missing script, retained frame/prepare
failures, retry and nested binds. Frame/B0/inline/file/lookup boundaries doubled.
Strict actual host composition + process SOUND/proximity + script bindings is
compile-only; app and27 tracker checks pass. Four new partials, none complete.
Temporary audits hashed/removed; exports/logs retained. No application readiness.

00427730 now requires Runtime.prepare_script_binding and captured-target
completion. Actual base/Human/Dog context family0/1/2 stores real actor10C and
zero108. Eligible existing Humans get005A3DF0 before compile; captured state5
flag later rereads current B0 and005907C0(context/null). Cold B0-null genuinely
skips both. Pending context EC publication is resolvable during state bind,
before vector append. Failure clears EC/ref BEFORE notification. Actual
00421C00/00421970 derived command receivers still required. Family getter
entries0041DF10/00421BF0/00421960 are absent inventory: no credit. Dog534 positive
receiver remains explicit error (cold field is0).

Previous99c8688 [Car activation](analysis/car-activation-loading.md) completes
cold offline source type4 composition, SAME owners, nine parameter sets and53
activation/teardown failures; actual host/world dispatch compile-only. Important
contracts: actor624/core594 is SAME lighting; actor4A0/core410 velocity, actor958/
core8C8 speed. Interior impact bands C38..C44/22AD..22B0. Actor masks wheel98302 /
BODYCOLD8002 supersede F02/2 defaults. DM countdown resets0 before cold update.
Water uses SECOND/max006FDA00 output/-FLOATMAX; no duplicate decoder credit.
Inline optional initializer keeps descriptor addresses stable.20000 per-wheel
cache implemented; front/rear plane mode, positive running/occupied/gun-seat/
deformation/skid/emitter/non7/queued SOUND receivers still reject.

## Next — same entire checkbox

Finish actual containing Human/DD0/state/FPV/HUD/SOUND/VM/module/session ownership,
ordered006CA780 loader/postactivation, first camera/update/render/failure/repeat
and user desktop acceptance. WorldScene/WorldActors still uncalled by application.
HumanSceneActor.loading_lifetime now binds the source composition through
WorldActors. Actual Consumers.make_human_loading_owner still must construct it
with real containing dependencies; no successful substitute. human_loading_owner
<Composition>(human) supplies checked per-Human control/state factory lookup.
HumanSoundStateRouter<Frames,Human,StateFactory,Lookup> supplies actual shared
SOUND routing without borrowing another Human's inline. VM/context access,
FPV/HUD/Players presentation and session consumers still need concrete bindings.
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

Previous verified actor checkpoints: Car activation99c8688, Car cold runtime7acfb09, StaticWeapon1009ef3,
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
Human notification exports:human-script-binding-notifications/005a3df0.c and
005907c0.c (clean read-only job). Existing actor context constructors are in
human-state-script-dependencies/. Next export script-actor-command-receivers/
00421c00.c and00421970.c; check its log/session completion before using.
Seat export job produced00466010 then errored on undefined004DA090: no whole-job
success; existing4DA090 export lives in car-activation-required.

Default sandbox normal commands; git/Ghidra config/native sanitizers need
escalation. One read-only Ghidra process/fresh output; check ERROR even exit0.
Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Ghidra project:/home/control/Documents/Projects/HD2ResearchProject HD2Research.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64; do not reinstall.
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
