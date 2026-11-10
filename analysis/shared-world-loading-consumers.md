# Shared world loading consumers — source checkpoint, 2026-11-10

The full first P2.2 implementation checkbox remains open. Application loading
still declines; no loaded world, first rendered world frame or desktop result.
This continues [Human ownership](human-world-composition.md),
[FPV](first-person-loading-composition.md), [HUD/player presentation](hud-player-presentation-loading.md)
and [world sound](sound-world-loading.md). The containing aggregate is still
required; these source bridges do not construct it or publish readiness.

## Inventory, Switch and SOUND

0049FBF0 reads the SAME inventory root's E0 bit0. Hidden returns immediately;
visible tail-calls004A49F0→004A26D0. `HudPanelsLoading.inventory_changed` now
implements this actual hidden branch and `HudSceneLoading`/Human presentation
route through it. Missing root and visible interactive inventory reject. Normal
004A0EE0 bind hides this root. The visible receiver updates drag, actor weights,
body slots and holster/pack sides; it is not a missing successful cold refresh.

`SceneActorLoadingRuntime` supplies the actual common runtime interface used by
Scene2 Human/Switch. Context lookup must be the SAME module's checked EC owner;
006136F0→0041D960 requests Use PC with authority bit1. The pushed state argument
is unused/RET4, not a variable assignment or immediate Block. Switch sound
uses00433FA0 flags1/group1/entryD8/world position, retaining other request defaults.
Human destruction routes actual006DFE50 proximity and the required nonoffline
player record consumer. No replacement context/proximity/player array exists.

`WorldSoundLoading` supplies the actual actor/inline/B0/camera endpoints of the
existing00433FA0/00435B00 producer. It calls SAME StartupSession.emit_game_sound,
which uses process SOUND/catalogue/gate/random/cache. Sector005AC4C0 comes from
WorldActors' checked composition's actual DD0 animation owner; actual null sector
remains null. Hearing requires a Human source, matching the original unconditional
source+98 helper. Wrong family/null/stale lookup rejects.005A6420 reads current
B0+50 bit18hex (40000), not an actor flag or a second state. Camera virtual7C
is00465070/byte270, distinct from26D/26E/2B4; virtual8C is004650C0/sector24C.
The existing sector getter's incorrect00465AD0 address comment is corrected;
00465AD0 is a different280 writer. Both actual tiny camera getter entries are
absent from the frozen inventory and receive no credit.

## Common Human services and identity

`HumanWorldLoadingServices` binds common DD0/230/B0/loading callbacks to SAME
startup pools/profile/configuration, session28/2C clocks, WorldActors composition,
Players control, FPV, shared face/presentation/animation and game music/SOUND.
Look requires actual E0. Equipment refresh verifies the supplied shadow belongs
to that Human's DD0 and uses its actual230; no shadow/visibility substitute.
Face camouflage uses actual presentation terrain0098A6B0, including its cold
value before first shared acquire. Player/view eligibility uses005A0890, with
0069A4B0 reading actual FPV58 OR59 independently. Current selection/removal,
Scene2/runtime activity and inventory capacity use existing recovered owners.
This common subset does not yet supply every derived AI/Sight/speech/area endpoint
needed to construct and activate the full Human factory.

Concrete composition exposed an unnecessary external requirement:
007855C0 initializes DD0+DC4 byte0;0075B280 reads that SAME DD0 byte through
HumanA0. `HumanControlLoading` now owns/reads it directly for physical descriptor
activation, instead of requesting a guessed world-level Human collision flag.
No loading producer changes it; later runtime setters remain separate.

Startup exposes the SAME control process00982104 identity, retaining its exact
32-bit storage with signed Human ID interpretation. Cold reset still writes0;
there is no per-world copy or changed interactive producer.006B66B0 checks actual
GameConfiguration04 nonzero then224==3, independently of process mode.006B6700
uses remote26C FIRST, else offline7 profile34, else3. The profile getter is lazy;
remote selection never accesses it. Source preference and actual Startup delegate
share one policy, not parallel mirrored implementations.

0045E920 scans actual control38 selection records in reverse by identity only.
Zero references, dead/enabled state and expiry timer do not exclude a record;
0045E960 is a different positive-reference query. `HumanSelectionRecords.knows`
and the world service preserve that distinction.005A6380 returns false for
signed health<1/nullB0/property19C1, otherwise B0+50 bit26hex.0049B980 uses any
nonzero HUD flags and actual compass append/retire, without soldier-panel changes.
Normal selection record expiry and zero-ref knowledge are exercised with these
actual source bodies.

## Evidence, verification and tally

[Receipt](checkpoints/p2.2-shared-world-consumers-20261110.json). Original client
SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored durable WORK=analysis/work/mission-world-20261103 retains:
current-player-inventory-refresh-required/, current-inventory-refresh-callees/,
current-player-inventory-refresh-gate.asm; spatial-world-camera-targets.json,
spatial-world-camera.asm, spatial-world-sector-identity.asm; human-world-config.asm,
human-world-view-offset.asm, human-world-known-enemy.asm. Existing Switch/context
contracts in actor-loading-ownership.md were reused, not re-audited.

Read-only camera export reported no Ghidra function at00465070; static vtable
bytes and disassembly establish both getters instead. No original binary ran.
Human feedback exports human-shared-player-feedback-required/ and
human-feedback-consumers-required/ completed; positive visibility and speech
consumers remain next research, not implementation or credited callee bodies.

Focused native ASan/UBSan/LSan checks actual Switch/property/context Use event
and failure prefix, Human teardown dispatch, actual Human/B0 world SOUND lookup
and hearing, shared Human bindings/config/music, selection record expiry and
HUD selected-marker gates. Frame/engine/containing world/DD0/Players/camera/SOUND
boundaries are explicit doubles. Strict actual-host i386 checks real Startup,
SceneActor runtime, Script module EC lookup, process proximity, actual spatial
producer, WorldActors map, inline/DD0/230 visibility, FPV getters, Players
selection/removal and game music. Containing aggregate, animation callbacks,
nonoffline records, player presentation endpoints and VM/model owners remain
required declarations. It is compile-only composition, not complete Human or
loaded-world execution. App links;27 function tracker tests pass.

Six eligible partials are added:0049FBF0,005A6420,006B6700,0069A4B0,0045E920,
005A6380. Existing0049B980 now points to its actual marker receiver body. Two
prior reconciliation rows005A8740/005A86F0 are retracted: they referenced abstract
feedback calls in HumanSelectionRecords, with neither original visibility body
implemented. This targeted correction affects the current dependency path, not
a new broad audit. Revised total34 complete/1,871 partial (1,866 client/five
engine)/18,783 frozen. Zero complete promotions; delegated/absent bodies gain none.

Continue actual containing Human/FPV/HUD/SOUND/VM/session ownership, reached
selection/AI speech receivers, required integrity observed-value publication,
ordered006CA780 loader/postactivation, camera/update/render and failure/repeat,
then user desktop acceptance. Keep later interactive inventory/occupied attachment/
nonoffline services explicit; do not expand all P2.3 simulation into cold loading.
