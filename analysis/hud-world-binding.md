# HUD world binding — source checkpoint, 2026-11-10

The actual GameHudLoading/HudPanels/HudScene bind now instantiates with the real
WorldScene, Scene2, session descriptor, navigation tables and Startup process
owners. Application loading still declines. P2.2 remains open; no engine/world
execution or desktop acceptance is claimed.

`HudWorldLoading` uses the existing `HudActorIdentityView`: actual Scene2 identity
slots0089B3B0/B4, reserved32, holes and current count. It creates no two-entry
substitute, copied roster or actor owner. HUD's6B8F8/FC selection pair is separate.
The view returns each actual SceneActor's **ActorBase**. The common world consumer
now passes that same base pointer to0049EF70 retirement, rather than its outer
SceneActor wrapper. This makes selection, command reference and map/timer domains
agree without changing the Scene2/Human/Items registries.

Map goals borrow session.mission->related/nativeA8->34, the same descriptors used
by MissionObjectiveStatus.004A7520/004A7680 capture missionA8 once per pass and
reread its vector count/entry; HudMissionObjectiveView and the map bind now do
that. A null mission genuinely yields no goals. No second objective vector or
status authority is introduced. Positive type37 groupA4 remains an explicit
required receiver; its scene constructor is outside the supported offline path.

Two concrete ownership corrections remove the former opaque external Borrowers:

- 0049B280 at0049B343 forms **HUD+6B320** and passes that SAME inline pointer
 context to map004A4AE0, inventory004A0EE0 and command004B4980. Source now passes
 `&auxiliary_.pointer()` after binding it, never a session/HUD/grid substitute.
 That pointer owner outlives the panel borrowers through GameHudLoading teardown.
- 004B4980 at004B4A99 reads **game+54**, stores it at command+5A7B4. This is the
 SAME two navigation datasets from00523AB0, not collision tree/grid:00698510 at
 006986F1 calls00523AB0 and00698701 stores game54. HudWorld borrows the actual
 SessionOwner.tables(), and the command argument/member is named navigation.

The strict containing graph now uses GameWorldScene/GameWorldActors with the
actual **GameOwnerBackend** template parameter, matching SessionOwner's actual
MissionTables<GameOwnerBackend>. The old TableStorage-only fixture could not
consume that parent allocation. No reinterpretation or extra dataset is used.
GameHudLoading bind now needs mission/mode/editor arguments only; every pointer
context comes from the actual owning graph. The remaining Players subgroup
contract is a pure virtual required interface, with no allocated subgroup or
successful tick/destroy implementation. Its loading module has no append producer.

Original client SHA256 remains
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Evidence in WORK=analysis/work/mission-world-20261103: existing
hud-loading-artwork-bindings/0049b280.c,
hud-loading-panels-lifetime/004b4980.c,
hud-map-command-dependencies/004a7520.c and004a7680.c, and
analysis/work/multiplayer-bitstream-20261101/00698510.c. Short instruction anchors
are hud-inline-pointer-context.asm, hud-command-navigation-owner.asm and
session-navigation-owner.asm. This is targeted discrepancy resolution; no broad
unchanged HUD audit was repeated.

[Receipt](checkpoints/p2.2-hud-world-binding-20261110.json) records native sanitizers
on actual identity/objective view lifetime, current counts/holes, captured mission
with live vector mutation and actual WorldActorLoadingConsumers Base retirement.
Registry/frame inputs and HUD sink are explicit doubles. Strict i386 compiles the
real GameHudLoading bind/close and prior world/Car/Human/VM/module/spawn graph;
application link passes. Windows/original/DLL/Proton code was not executed.
No new original function credit; existing partial coverage is corrected.
Continue actual dependency aggregate construction, ordered loader/guard/first
frame/failure/repeat under the same full checkbox.
