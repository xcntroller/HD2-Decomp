# Script module and visibility receivers — 2026-11-09

P2.2 remains open. This source checkpoint extends the
[compiler/context](mission-script-loading.md); it does not activate the host
world or enable loading. No mission desktop test has been performed.

`mission_script_frame_commands.h` supplies actual00416E10 opcodes1/5E/66/87:
ScriptAssign, FindFrame, FindChildFrame and SendSignal. FindFrame releases its
old destination before validating the string, tries driver+88 before scene+7C
(maskCFFFF), and retains the result. FindChildFrame clears its destination
before reading the parent; aliased destination/parent therefore finds nothing.
The child query uses maskFFFF. NULL SendSignal returns false without advancing;
a valid frame without a context advances.0041CFA0 reads the SAME frame EC word1;
source additionally checks that the actual module owns it. ScriptAssign retains
the native SpawnPoint→spawned Human fallback and ignores bind's false result.
Self-assignment explicitly rejects retirement of the executing context.

`mission_script_player_predicates.h` implements0041E3F0 cases0/2/5/14/16:
PlayerInRange, AllPlayersInRange, dead/live/lost-player counts. It uses the SAME
Players control/auxiliary rosters and signed healthEC>0.004622C0 counts ascending;
range checks traverse backwards. PlayerInRange uses wrapping IMUL radius²,
signed integer threshold and strict distance² comparison. AllPlayersInRange
requires at least one live member, uses inclusive distance/radius, and retains
the native FLOAT spill/tiny and near-one distance gates. Actual005A63B0 checks
mode/B0/faction19C/activation20C;005A63F0 suppression still requires its receiver.

`mission_script_variables.h` owns the module's256 current and previous DWORDs.
SaveGameValue8D and _LoadGameValue0C validate the actual integer pool/index.
00428C80 copies CURRENT→PREVIOUS; activation kind1 clears CURRENT only.

`mission_script_visibility.h` implements00416E10 SetOn5F through the real
session broadcast packet/FIFO, not direct presentation mutation. Invalid pool
indices return true without advancing. Valid lights enter00423980's borrowed,
reverse-deduplicated queue before update/send.00424590 consumes opcode8 before
the presentation gate;0042B650's5F payload is name+one-bit bool. Decoder lookup
is scene+7C BEFORE driver+88, reversing the compiler's lookup order. Lights use
actual+94(0) on enable and+AC(bool,0); models use the actual specialized owner.
Nonoffline transport and other presentation opcodes remain required.

`mission_script_frame_snapshots.h` implements00423670 capture-once/OR-mask
records retaining BOTH frame and parent, local position/rotation, visibility,
and light colour1A0/parameter16C. Supported masks20/30 restore visibility in the
module finish prepass.00423BF0 always restores visibility again, relinks a
retained parent with mode0, updates, restores light colour/parameter, updates,
then releases the frame.00423B40/TeleportNearFrameBack6D retire one matching
snapshot and swap/pop without the finish prepass. Transform/animation masks
remain explicit rejects until their complete restoration consumers exist.
Failures still retire all owned references before propagating the first error.

`mission_script_model_visibility.h` reconstructs0042B650's model branch using
the SAME Human inline98 DD0 shadow and230 equipment; it never constructs a
second Human control owner. Enable refreshes equipment/shadow and the current
player's FPV visibility; disable clears the actual Human shadow. Other models
use the existing GEffects shadow build/query. Actor25 visibility rebuilds or
clears its actual BODYCOLB list; actor34/36 uses004865E0's children-first reverse
visibility mask2088. The native frame-kind mask table and light+AC calls are
preserved. Human/FPV containing composition remains a required external owner.

`animation_actor_collisions.h` reconstructs005B3AC0/005B4DE0/005B4850/
005B48B0/005B4DC0/005B54C0: clear old list first, find consecutive BODYCOLB00...
(maskFFFF), stop at first missing name, own aligned descriptor/matrix/inverse
records, and register/remove with the SAME dynamic collision buckets. Defaults
are flags40/mask400000 and a borrowed actual actor/frame. Case-sensitive matid
parsing ORs the shifted material into flags. Kind6 uses dummy120 bounds;
other kinds use actual driver+AC aggregate bounds and world/local scale radius.
The unscaled centre transforms through the actual world matrix; bbox XYZ uses
local scale, W is preserved, and basis XYZ normalizes by local scale. Native
indeterminate centre from invalid aggregate bounds and zero/nonfinite scale
reject explicitly.005B4790 retires descriptors BEFORE the actor's model/frame.
This does not implement runtime animation event/attachment or physics simulation.

`mission_script_module_loading.h` registers kind4/name Script (original call
00428F60/string00838954), reverse-loads/binds Scripts.dta or mpscripts.dta,
latches native loaded10 even on compiler false, and exposes failed bindings to
the containing world. Actual004270D0 begins contexts but executes no instruction.
Tick invokes each context once in reverse; deferred-frame/timer producers remain
required.00427B80 clears lights, restores frames BEFORE reverse context Exit/
destruction, then requires actual GEffects+8C(0,false,nullptr). `effects_fade.h`
implements00486B30's duration/target/colour configuration, not its renderer/tick.
The native null-colour W is indeterminate; source marks W unspecified.
004270D0/00427B80/00428490 are absent frozen inventory and earn no new credit.

Evidence is retained under ignored durable
`analysis/work/mission-world-20261103`: script-presentation-writer-required/,
script-presentation-decoder-required/, script-frame-snapshot-required/,
script-module-finish-required/, script-model-visibility-required/,
script-model-effects-required/, script-animation-visibility-required/,
script-animation-collision-required/, script-animation-collision-constructor/,
script-animation-collision-update/, script-animation-collision-retire/,
script-frame-transform-required/, and the script-*.asm excerpts. Client SHA256
is8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
The frame-transform exports are research for the next dependency, not new credit.

Temporary native ASan/UBSan/LSan coverage exercises lookup precedence/aliasing,
retained frame/context cleanup, signal and binding removal, player range/health/
remote gates, actual local packet queue and decoder, visibility/light snapshot
restore, module failed-bind/finish/repeat, model/Human visibility orchestration,
and dummy/scaled aggregate BODYCOLB registration/move/removal. Engine/world/
Human inline/FPV boundaries use explicit fixture doubles. Strict actual-host
Windows i386 instantiation and application compile/link pass; declarations do
not establish containing Human/FPV/runtime receivers. No Windows fixture runs.
See the [receipt](checkpoints/p2.2-script-receivers-20261109.json).

Continue FULL P2.2 through actual remaining base/Human/Dog commands/predicates,
asynchronous/deferred/transform owners and runtime Files selection, then the
containing world and remaining SOUND/actor/physics/objective/frame consumers.
