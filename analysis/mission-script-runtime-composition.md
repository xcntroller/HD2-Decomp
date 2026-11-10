# Mission script runtime and initial objectives — 2026-11-10

Source subsystem of the full first P2.2 checkbox. Application loading remains
 disabled; no loaded world, rendered world frame or desktop acceptance. This
continues [initial controls](mission-script-initial-controls.md) and adds a
containing runtime over the actual recovered receivers, without supplying the
remaining world/inline/presentation consumers.
[Verification receipt](checkpoints/p2.2-script-runtime-objectives-20261110.json).

## Objective status and process state

006B6510→0069BA60/0069BC40 uses SAME SessionActivationState missionA8.related
(native34 objective vector), Players roster, process CRT stream and HUD small
panel. Initial TUT_Objectives Block sets objectives1..7 to0.0069BEB0 stores
child.word10 and latches flag15 for status1; later statuses do not clear the
latch. Status1 awards1000 (flag14:1500) to positive-health Humans in reverse
roster order, once per latch. Invalid objective with a mission returns before
awards; absent mission still awards1000.005A9640 updates HumanE0, not profile
rank/stats, with DWORD wrapping and signed retail output.005A82C0 offline uses
profile user14, short name10 fallback, or actual actor name when no profile.
0069BFE0 and0069BFD0 are genuine RET bodies, not HUD/console output services.

Status3 invokes0069BED0: set session6F, result88 and clock8C, notify actual
0049DE30→004AD880 hint1CE9, THEN pending0 objective statuses become2. A failing
hint leaves the earlier result mutations but descriptors untouched. Status4
stores and immediately returns. Other statuses evaluate completion: no mission,
empty/all4 vector or any pending0 returns; otherwise result uses1CE8 only for
status1 without6F, else1CE9. Opcode88 preserves signed operands, authority and
ordinary advance. Nonoffline transport/score/HUD branches explicitly reject.

The prelude is required:006E9EA0 initializes128 expected/observed DWORDs,
separate WORD permutations and three auxiliary random arrays with the SAME
seeded CRT stream (896 draws). Stage13 indices are WORDs at8CAB4A/8CB24A,
**not bytes**; values are DWORDs in8CA730/8CAE30. On mismatch0069BA60 draws
rand and forces status3 only below FLOAT008159E4=3276.699951171875. Equality
consumes no draw. MissionIntegrityTables owns these scalar tables and rejects
use before initialization. StartupSession owns one shared instance and the
existing CRT seed endpoint, but does not initialize/use it in the application.
Actual loader/API/integrity observed-value publication remains required. No
production equalization or successful protection substitute exists. Native
fixtures explicitly prepare equality at their isolated process boundary.

The original-address/per-thread integrity VM is outside the accepted startup
checkpoint ([scope](startup-integration.md)); this recovery does not claim it.
Already-resolved real Win32 payloads remain authoritative over misleading
Ghidra IAT trampoline signatures. Source table initialization/comparison is
separate from resolving the remaining observed-value producer.

## ReturnEnd and EndScript

00416DB0 gates authority, stores104false/D4true, then actual frameEC Human/B0
kind5 invokes005AF5C0 working0 and005AF570 resume, with no health gate.
EndScript base00416E10 F9 stores D4 FIRST; positive-health Human invokes working0,
resume, then005AF450→0058F680. Dead Human skips callbacks. Actualstate5 enables
events if needed before state selection: property19C1 calls real select_script;
2 chooses8/4 from command14;3 chooses6;4/5 choose3. No state borrower survives
selection. Current-player type22 attachment0045DA40 and live Dog005D5870/
0041D790 remain explicit required rejections. F9 increments PC directly,
bypassing00416CF0 and leaving104 unchanged.

## Containing module runtime

MissionScriptLoadingRuntime borrows the SAME frames, mode, session, process
advance flag, clock, binding factory, views/objectives/lifecycle/Human controls,
player predicates, world and Effects. Composition owns runtime BEFORE module;
module finish occurs while dependencies live. Binding validates SAME runtime
and process-mode address. Unknown commands/predicates reject; only a known
base false result reaches Human/Dog's genuine default advance. Base receivers
run before derived00421C00. Human signal admission checks actual EC/B0kind5.

00416CF0 uses actual008692D0 OR game28, without a guessed loading exemption.
The BSS override defaults false; confirmed direct writer00424900 scopes it
around script reset, a separate unentered cold-loading path. Clock delegates
004167B0's QPF/QPC microseconds wrapping to existing FrameClockPlatform.
Module shutdown calls actual GEffects8C/00486B30 fade configure(0,false,nullptr).
Native loaded10 still latches on bad bindings; the containing world must inspect
failed_bindings before claiming required loading succeeded.

## Script camera look update

00466970's00467353..0046784F branch now consumes borrowed288. Position/shake
of main240 precedes lookup of the SAME scene active camera15C, which may be
external. Target world translation minus active-camera translation is normalized
with native (X²+Y²)+Z² and zero-direction Y fallback. Angle28C<=0 applies directly;
positive speed uses actual normalized camera forward, S_vector::AngleTo and
existing engine SetDir/Slerp/GetDir with signed elapsed*.001*speed/angle. Strict
factor>1 clamps and stores speed-1. Camera virtual0C receives direction/roll0,
then active camera updates before common main-camera update, even when SAME.
Clearing target returns ordinary breathing/direction behavior. Target288 remains
borrowed and distinct from retained Human-follow248. Debug/config8/attached
pose branches still reject; ordinary verified follow/collision is unchanged.

## Evidence and verification

Original client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored durable WORK=analysis/work/mission-world-20261103 retains exports:
script-objective-guard-producer/006e9ea0; script-objective-result-receivers/
0049de30,005a9640; script-objective-experience-name/005a82c0;
script-initial-suspend-objective-dependencies/0069ba60;
script-initial-camera-objective-required/0069bc40;
script-initial-camera-target-objective-state/0069beb0,0069bed0,0069bfd0;
script-flow-pc-required/00416db0; script-human-end-state-required/;
script-cold-item-end-required/005af450,005af5c0; camera-simulation-typed-abi/
00466970; script-camera-look-region.asm; script-advance-references.log.
objective-guard-seed1234.json records the896-draw scalar golden capture.

Focused native ASan/UBSan/LSan checks actual compiler/VM/Tutorial Block,
Players/Human experience, HUD small panel/TextState/result ordering, source
Runtime/Module/Contexts/Bindings cold tick/close/repeat/bad binding and actual
MissionCamera load/look/clear/repeat. Engine/frame/file/clock/CRT/presentation
boundaries are explicit doubles; lifecycle cleanup endpoints are doubled.
Strict i386 instantiates actual host Human/state/Scene2/HUD/Clock/Script/camera
receivers; unresolved world/inline/presentation endpoints are declarations.
Application compile/link and27 tracker checks pass. Temporary native/Windows
fixtures are hashed and removed; no original/Windows binary was executed.

Continue full checkbox with actual Human/inline/FPV/HUD/SOUND/world/session
ownership, required observed-value producer, ordered006CA780 loader and first
camera/update/render/failure/repeat. This source checkpoint does not enable it.
